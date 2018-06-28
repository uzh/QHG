#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include <map>
#include <set>

#include "colors.h"
#include "BinomialDist.h"
#include "LayerBuf.h"
#include "LayerBuf.cpp"
#include "LBController.h"
#include "GeneUtils.h"
#include "AncestorNode.h"
#include "AncGraphBase.h"
#include "AGOracle.h"

#include "GraphEvolverBase.h"
#include "GraphEvolverF.h"

#define EPS 1e-6

#define DEF_SAVE_BUFFER_SIZE 32768

double t0;
#define START_TIMER t0=omp_get_wtime()
#define STOP_TIMER(tC) (tC)+=omp_get_wtime()-t0

#define START_TIMER2
#define STOP_TIMER2(tC)

//----------------------------------------------------------------------------
// create
//
GraphEvolverF *GraphEvolverF::create(AncGraphBase *pAG, const char *pAGFile, int iOracleBlockSize, int iGenomeSize, int iNumCrossovers, double dMutationRate) {
    GraphEvolverF *pGE = new GraphEvolverF(pAG, iNumCrossovers, dMutationRate);
    int iResult = pGE->init(iGenomeSize, pAGFile, iOracleBlockSize);
    if (iResult != 0) {
        delete pGE;
        pGE= NULL;
    }
    return pGE;
}


//----------------------------------------------------------------------------
// create
//
GraphEvolverF *GraphEvolverF::create(AncGraphBase *pAG, int iGenomeSize, int iNumCrossovers, double dMutationRate) {
    GraphEvolverF *pGE = new GraphEvolverF(pAG, iNumCrossovers, dMutationRate);
    int iResult = pGE->init(iGenomeSize, NULL, 0);
    if (iResult != 0) {
        delete pGE;
        pGE = NULL;
    }
    return pGE;
}


//----------------------------------------------------------------------------
// constructor
//
GraphEvolverF::GraphEvolverF(AncGraphBase *pAG, int iNumCrossovers, double dMutationRate) 
    : GraphEvolverBase(iNumCrossovers, dMutationRate),
      m_pAGO(NULL),
      m_bParLoad(false) {
    m_pAG = pAG;

    //@@V3&V4
    int iNodeLayerSize = 100000;
    m_pNodeController = new LBController;
    m_aNodes.init(iNodeLayerSize);
    m_pNodeController->init(iNodeLayerSize);
    m_pNodeController->addBuffer(static_cast<LBBase *>(&m_aNodes));
    
    //V4
    m_iNumThreads = omp_get_max_threads();
}


//----------------------------------------------------------------------------
// init
//
int GraphEvolverF::init(int iGenomeSize, const char *pAGFile, int iOracleBlockSize) {
    int iResult = -1;

    iResult = GraphEvolverBase::init(iGenomeSize);
    
    if ((iResult == 0) && (pAGFile != NULL)) {
        printf("creating oracle for [%s]\n", pAGFile);
        m_pAGO = AGOracle::createInstance(pAGFile, iOracleBlockSize);
        if (m_pAGO != NULL) {
            iResult = 0;
        } else {
            printf("%sCouldn't create AGOracle from [%s]\n", RED,pAGFile);
            iResult = -1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// destructor
//
GraphEvolverF::~GraphEvolverF() {

    if (m_pAGO != NULL) {
        delete m_pAGO;
    }

    //@@V3    
    delete m_pNodeController;

}


//----------------------------------------------------------------------------
// calcGenome
//  recursive genome calculation
//  slow anf might run into trouble for big ancestor graphs
//
const ulong *GraphEvolverF::calcGenome(idtype iID) {
    const ulong *pGenome = NULL;
    
    if (iID > 0) {

        pGenome = getGenome(iID);
        if (pGenome == NULL) {
            AncestorNode *pFN = m_pAG->findAncestorNode(iID);
            if (pFN != NULL) {
                const ulong *pGenomeM = calcGenome(pFN->getMom());
                const ulong *pGenomeD = calcGenome(pFN->getDad());
                
                if ((pGenomeM != NULL) && (pGenomeD != NULL)) {
                    pGenome = createMix(pGenomeM, pGenomeD);
                    m_mGenomes[iID] = pGenome;
                } else {
                    printf("%s%ld has bad parents\n", RED, iID);
                    printf("mom %ld: %p\n", pFN->getMom(), pGenomeM);
                    printf("dad %ld: %p\n", pFN->getDad(), pGenomeD);
                }
            } else {
                printf("non node for %ld\n", iID);
            }
        } else {
            //            printf("%d already has a genome\n", iID);
        }
    } else {
        //        printf("non-positive ID: %d\n", iID);
    }
    return pGenome;
}


//----------------------------------------------------------------------------
// calcGenomes
//   
int GraphEvolverF::calcGenomes(idset &sSelected) {
    int iResult = 0;
    printf("calcGenomes with: Genomesize %d, num blocks %d, num bits %d\n", m_iGenomeSize, m_iNumBlocks, m_iNumBits);
    idset_cit it;
    for (it = sSelected.begin(); (iResult == 0) && (it != sSelected.end()); ++it) {
        if (calcGenome(*it) == NULL) {
	    printf("%sno genome for id %ld\n", RED, *it);
            iResult = -1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// listIntSet
//   show the contents of an int set
//
void listIDSet2(const char *pText, const idset &s) {
    printf("%s : ", pText);
    idset_cit it;
    for (it = s.begin(); it != s.end(); ++it) {
        printf(" %ld", *it);
    }
    printf("\n");
}


//----------------------------------------------------------------------------
// spreadMotherGenomes
//   tries to create genomes for all children of ids in sCurGen.
//   ids with newly created genomes are put in sNextList,
//   ids which are not needed anymore (all teir children have genomes) are
//   put in sDumpList.
//   
//   for a given id in sCurGen, each remaining child. If it  has a genome,
//   nothing has to be done, and we remove it from the first parent's set of 
//   children. Otherwise, we look for the other parent's genome. 
//   If this exists we create the childs genome and remove it from the first
//   parent's child list. Otherwise, the first parent remains in sCurGen and
//   will have another go in the next round
//
int GraphEvolverF::spreadMotherGenomes(const idset &sCurGen, idset &sNextList, idset &sDumpList) {
    int iResult = 0;

    idset_cit itM;
    if (m_pAGO != NULL) {
        // we must make sure all nodes in sCurGen and their children are loaded
        idset sLoad;
        if (iResult == 0) {
            // loop through ids in curgen and check if their children have nodes
            for (itM = sCurGen.begin(); (iResult == 0) && (itM != sCurGen.end()); ++itM) {
                AncestorNode *pANMom = m_pAG->findAncestorNode(*itM);
                if (pANMom != NULL) {
                    idset_cit itC;
                    for (itC = pANMom->m_sChildren.begin(); (iResult == 0) && (itC != pANMom->m_sChildren.end()); ++itC) {
                        AncestorNode *pANChild = m_pAG->findAncestorNode(*itC);
                        if (pANChild == NULL) {
                            sLoad.insert(*itC);
                        }
                    }
                } else {
                    printf("%s[spreadMotherGenomes] No node for id %ld\n", RED, *itM);
                    printf("Should not happen, because all in curgen have been loaded\n");
                    iResult = -1;
                }
            }
        }
        
        // load the nodes for the children 
        iResult = m_pAGO->loadNodes(m_pAG, sLoad);
    }
    
    if (iResult == 0) {
        
        // loop through all ids in curgen
        for (itM = sCurGen.begin(); (iResult == 0) && (itM != sCurGen.end()); ++itM) {
        
            // i call the ancestor node Mom, but it could also be a male
            AncestorNode *pANMom = m_pAG->findAncestorNode(*itM);
            if (pANMom != NULL) {

                idgenome::const_iterator itGM = m_mGenomes.find(*itM);
                if (itGM != m_mGenomes.end()) {
                
                    // loop through children of Mom
                    //@@ if (bVerbose) printf("Mom %d has %zd children\n", *itM,  pANMom->m_sChildren.size());
                    idset_cit itC;
                    // loop through all children ofMom
                    // NOTE: we do not increment the iterator in the loop, because we may remove a child from the set of children 
                    for (itC = pANMom->m_sChildren.begin(); (iResult == 0) && (itC != pANMom->m_sChildren.end()); ) {
                        // for the case of child deletion, we need an iterator to the next element
                        idset_cit itCNext = itC;  
                        itCNext++;
                    
                        bool bCompletedChild = false;

                        // find genome of child
                        idgenome::const_iterator itGC = m_mGenomes.find(*itC);
                        if (itGC == m_mGenomes.end()) {
                        
                            // get AncerstorNode of child to find other parent
                            AncestorNode *pANChild = m_pAG->findAncestorNode(*itC);
                            if (pANChild != NULL) {
                            
                                // find other parent 
                                idgenome::const_iterator itGD;
                                if (pANChild->getMom() == *itM) {
                                    itGD = m_mGenomes.find(pANChild->getDad());
                                } else {
                                    itGD = m_mGenomes.find(pANChild->getMom());
                                }
                                if ((itGD != m_mGenomes.end()) && (itGD->first != *itM)) {
                                    //                                    if (itGD->second == NULL) { printf("NULL genome for %d\n", itGD->first);}
                                    const ulong *pGenome = createMix(itGM->second, itGD->second);
                                    m_mGenomes[*itC] = pGenome;
                                    sNextList.insert(*itC);
                                    bCompletedChild = true;
                                } else {
                                    // no partner genome: we can't build genome for child yet.
                                    // parent will stay in CurGen list
                                }
                            } else {
                                iResult = -1;
                                // No ancestor node found for child
                                printf("%sNo child node: This should not happen\n", RED);
                            }
                        } else {
                            // child already has genome
                            // This case occurs if the other parent has initiated the genome-building for this child:
                            // It was removed from the other parent's child list, but not from this parent's one
                            bCompletedChild = true;
                        }
                        // if child is complete, remove it from current node's child list
                        if (bCompletedChild) { 
                            pANMom->m_sChildren.erase(*itC);
                        }   
                        // move to the correct next position
                        itC = itCNext;
                    
                    }
                } else {
                    iResult = -1;
                    printf("%sNo genome for id %ld\n", RED, *itM);
                }
                    
                // no more children in list: all children have genome,
                // this element is not needed anymore
                if (pANMom->m_sChildren.empty()) { 
                    sDumpList.insert(*itM); 
                }
            } else {
                iResult = -1;
                printf("%sThis should not happen\n", RED);
                printf("%sNo ancestor node for id %ld\n", RED, *itM);
            }
        }
    } else {
        iResult = -1;
        printf("%sAGOracle failed to load the nodes\n", RED);
    }
    return iResult;
}



//----------------------------------------------------------------------------
// updateLists
//   remove ids marked for dump from current generation set and
//   insert ids from next generation set to current generation set
//
int GraphEvolverF::updateLists(idset &sCurGen, const idset &sNextList, const idset &sDumpList) {
    int iResult = 0;
    
    // remove completed ones
    idset_cit it;
    for (it = sDumpList.begin(); it != sDumpList.end(); ++it) {
        sCurGen.erase(*it);
    }

    // add new ones
    sCurGen.insert(sNextList.begin(), sNextList.end());

    return iResult;
}


//----------------------------------------------------------------------------
// dumpCompleted
//   delete the genome arrays for the specified ids.
//   we can't erase the entry in m_mGenomes, because it 
//   serves as a marker that this node already has got a genome
//   (possibly already saved)
//
int GraphEvolverF::dumpCompleted(const idset &sDumpList) {
    int iResult = 0;

    // delete them remove them from the genome list
    idset_it it;
    for (it = sDumpList.begin(); (iResult == 0) && (it != sDumpList.end()); ++it) {

        idgenome::iterator itGC = m_mGenomes.find(*it);
        if (itGC != m_mGenomes.end()) {
            delete[]  itGC->second;
            itGC->second = NULL;
            m_mGenomes.erase(itGC);
           
        } else {
            iResult = -1;
        }

        // remove unused AncestorNodes
        ancnodelist &mIndex = m_pAG->getModifiableMap();
        ancnodelist::iterator itAG = mIndex.find(*it);
        if (itAG != mIndex.end()) {
            delete itAG->second;
            mIndex.erase(itAG);
        }
        
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// buildGenome
//
int GraphEvolverF::buildGenome(uint iSaveBufferSize, const idset &sRoots, const idset &sTargets) {
    int iResult = 0;

    idset sCurGen;
    idset sNextList;
    idset sBigDump;
    ulong iTotalDumped = 0;


    if (iSaveBufferSize == 0) {
        iSaveBufferSize = DEF_SAVE_BUFFER_SIZE;
    }

    printf("Starting to build genome: roots %zd (total %ld)\n", sRoots.size(), (m_pAGO != NULL) ? m_pAGO->getNumNodes() : (int)m_pAG->getMap().size());
    sCurGen.insert(sRoots.begin(), sRoots.end());

    if (m_pAGO != NULL) {
        // we need the ancestor nodes of the initial set of ids to start genome building
        idset sLoad;
        for (idset_cit itM = sCurGen.begin(); (iResult == 0) && (itM != sCurGen.end()); ++itM) {
            AncestorNode *pANMom = m_pAG->findAncestorNode(*itM);
            if (pANMom == NULL) {
                sLoad.insert(*itM);
            }
        }
        // load the nodes 
        printf("need to load %zd nodes in curgen: ", sLoad.size());
        for(idset_cit itLoad = sLoad.begin(); itLoad != sLoad.end(); ++itLoad) {
            printf(" %ld", *itLoad);
        }
        printf("\n");
        
        iResult = m_pAGO->loadNodes(m_pAG, sLoad);
    }

    // we keep a copy of the target set which we will modify:
    // each time an id of this set has a genome, it is removed from the set
    idset sMyTargets(sTargets.begin(), sTargets.end());

    // iNoChange is a counter: if there is no change in the sets after repeated loop iterations,
    // something is wrong and we have to abort
    int iNoChange = 0;
    // loop as long as there a targets and ids in sCurGen
    while ((iResult == 0)  && (sTargets.size() > 0) && (sCurGen.size() > 0) && (iNoChange < 5)) {
        sNextList.clear();

        // create genomes for a many children of the ids in sCurGen as possible
        idset sDumpList;
        iResult = spreadMotherGenomes(sCurGen, sNextList, sDumpList);
        if ((sNextList.empty()) && (sDumpList.empty())) {
            iNoChange++;
        } else {
            if (iResult == 0) {
                // update the lists
                iResult = updateLists(sCurGen, sNextList, sDumpList);
            }
            if (iResult == 0) {
                // if the dumpable is a target remove it from the target list,
                // otherwise add it to the big dump list.
                for (idset_cit it= sDumpList.begin(); it != sDumpList.end(); ++it) {
                    if (sMyTargets.find(*it) == sMyTargets.end()) {
                        sBigDump.insert(*it);
                    } else {
                        sMyTargets.erase(*it);
                    }
                }

                // we don't dump in every round, only if the size of the
                // dump list exceeds a ertain limit 
                if (sBigDump.size() > iSaveBufferSize) {
                    iTotalDumped += sBigDump.size();
                    printf("\r%16zd processed -> %16ld total  (curgen %16zd, ag %16zd)   ",  
                           sBigDump.size(), iTotalDumped, sCurGen.size(), m_pAG->getMap().size());fflush(stdout);
                    iResult = dumpCompleted(sBigDump);
                    sBigDump.clear();
                }
            }
        }
    }
    
    // dump the remaining ids in the dump list 
    if ((iResult == 0) && (sBigDump.size() > 0)) {
        iTotalDumped += sBigDump.size();
        printf("\r%16zd dumped -> %16ld total   ",  sBigDump.size(), iTotalDumped);fflush(stdout);
        iResult = dumpCompleted(sBigDump);
        printf("\n");
    }
    
    if (iResult == 0) {
        if (sMyTargets.empty()) {
            printf("successfully created genomes for all targets\n");
        } else {
            iResult = -1;
            printf("%zd remaining targets: ", sMyTargets.size());
            for (idset_cit it= sMyTargets.begin(); it != sMyTargets.end(); ++it) {
                printf(" %ld", *it);
            }
            printf("\n");
        }
    }

    fflush(stdout);

    /*
    // "correctness" check (debug): only targets should remain in the genomes list
    // may take very long...
    printf("Unsaved Genomes: \n");
    std::map<int, ulong*>::iterator it;
    for (it = m_mGenomes.begin(); it != m_mGenomes.end(); ++it) {
        if (it->second != NULL) {
            if (sTargets.find(it->first) == sTargets.end()) {
                printf(" %d", it->first);
            }
        }
    }
    printf("\n");
    */
    return iResult;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//-------EXPERIMENTAL VERSION-------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// This version (buildGenome2, spreadMotherGenomes2, updateLists2)
// differs from the original that only mothers are considered in the current
// generation.
//  
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// separateGenders
//
void GraphEvolverF::separateGenders(idset &sIn, idset &sOutM) {
    for (idset_it it=sIn.begin(); it != sIn.end(); ) {
        AncestorNode *pAN = m_pAG->findAncestorNode(*it);
        if (pAN->m_iGender != 0) {
            // we will delete the item, so remember who will be next
            idset_it itNext = it;
            itNext++;

            sOutM.insert(*it);
            sIn.erase(*it);

            // do not increase iterator, but use the "previous next"!
            it = itNext;
        } else {
            ++it;
        }
    }
}


//----------------------------------------------------------------------------
// removeChildless
//
void GraphEvolverF::removeChildless(idset &sTargets, idset &sIn) {
    for (idset_it it=sIn.begin(); it != sIn.end(); ) {
        AncestorNode *pAN = m_pAG->findAncestorNode(*it);
        if (pAN->m_sChildren.empty()) {
            printf("%ld is childless; genome:%p\n", *it, m_mGenomes.find(*it)->second);
            // we will delete the item, so remember who will be next
            idset_it itNext = it;
            itNext++;
            sIn.erase(*it);
            sTargets.erase(*it);
            // do not increase iterator, but use the "previous next"!
            it = itNext;
        } else {
            ++it;
        }
    }
}



    double tAncFind    = 0;
    double tAncFindMom   = 0;
    double tAncFindChild = 0;
    double tAncFindDad   = 0;
    double tGenomeFind = 0;
    double tMix        = 0;
    double tInsert     = 0;
    double tErase      = 0;

//----------------------------------------------------------------------------
// spreadMotherGenomesSeq
//   tries to create genomes for all children of ids in sCurGen.
//   ids with newly created genomes are put in sNextList,
//   ids which are not needed anymore (all teir children have genomes) are
//   put in sDumpList.
//   
//   for a given id in sCurGen, each remaining child. If it  has a genome,
//   nothing has to be done, and we remove it from the first parent's set of 
//   children. Otherwise, we look for the other parent's genome. 
//   If this exists we create the childs genome and remove it from the first
//   parent's child list. Otherwise, the first parent remains in sCurGen and
//   will have another go in the next round
//
int GraphEvolverF::spreadMotherGenomesSeq(const idset &sCurGenM, 
                                          idset &sDumpListM, idset &sDumpListD, 
                                          idset &sNextListM) {
    int iResult = 0;
    double t0;

    ancnodelist mPotentialChildlessDads;
    idset_it itM;

    // loop through all Moms (sCurGenM)
    for (itM = sCurGenM.begin(); (iResult == 0) && (itM != sCurGenM.end()); ++itM) {
        START_TIMER;
        AncestorNode *pANMom = m_pAG->findAncestorNode(*itM);
        STOP_TIMER(tAncFindMom);
        if (pANMom != NULL) {

            // loop through children of Mom
            idset_cit itC;
            // NOTE: we do not increment the iterator in the for-statement, because we may remove a child from the set of children 
            for (itC = pANMom->m_sChildren.begin(); (iResult == 0) && (itC != pANMom->m_sChildren.end()); ) {
                // for the case of child deletion, we need an iterator to the next element
                idset_cit itCNext = itC;  
                itCNext++;
                    
  
                // get AncestorNode of child (so we can find the Dad)
                START_TIMER;
                AncestorNode *pANChild = m_pAG->findAncestorNode(*itC);
                STOP_TIMER(tAncFindChild);
                if (pANChild != NULL) {
                   
                            
                    // find Dad's genome (may not be created yet) 
                    idtype iDadID = pANChild->getDad();
                    START_TIMER;
                    idgenome::const_iterator itGD = m_mGenomes.find(iDadID);
                    STOP_TIMER(tGenomeFind);
                    if (itGD != m_mGenomes.end()) {

                        // get Mom's genome (should have one)
                        START_TIMER;
                        idgenome::const_iterator itGM = m_mGenomes.find(*itM);
                        STOP_TIMER(tGenomeFind);
                        if (itGM != m_mGenomes.end()) {
                                    
                               
                            // crossing-over and mutate to get child's genome
                            START_TIMER;
                            ulong *pGenome = createMix(itGM->second, itGD->second);
                            STOP_TIMER(tMix);

                            // save child's genome 
                            START_TIMER;
                            m_mGenomes[*itC] = pGenome;
                            // if the child has no children, we can dump itC 
                            if (pANChild->m_sChildren.empty()) {
                                if (pANChild->m_iGender == 0) {
                                    sDumpListM.insert(*itC); // sDumpListM[iThread]
                                } else {
                                    sDumpListD.insert(*itC); // sDumpListD[iThread]
                                }
                            } else {
                                // otherwise, if female, it will be Mom in the next round
                                if (pANChild->m_iGender == 0) {
                                    //                                        printf("putting child  %d of mom %d\n", *itC, *itM);
                                    sNextListM.insert(*itC);      // sNextListM[iThread]
                                }
                            }
                            STOP_TIMER(tInsert);
                            // child is complete, remove it from its parents' child lists
                            START_TIMER;
                            AncestorNode *pANDad =  m_pAG->findAncestorNode(iDadID);
                            STOP_TIMER(tAncFindDad);
                            START_TIMER;
                            if (pANDad != NULL) {

                                // if the AncestorGraph is built correctly,
                                // *itC is in pANDad->m_sChildren
                                pANDad->m_sChildren.erase(*itC); // atomic
                                        
                                //                                    sPotentialChildlessDads.insert(iDadID); // sPotentialD[iThread]
                                // save DadID an pANDad to check for childlessness later
                                mPotentialChildlessDads[iDadID] = pANDad;
                            } else {
                                printf("%sGenome but no dad!\n", RED);
                                iResult = -1;
                            }
                            // remove it from the mom
                            pANMom->m_sChildren.erase(*itC);
                            STOP_TIMER(tErase);

                        } else {
                            iResult = -1;
                            printf("%sNo genome for id %ld\n", RED, *itM);
                        }


                    } else {
                        // no partner genome: we can't build genome for child yet.
                        // parent will stay in CurGen list
                    }
                } else {
                    iResult = -1;
                    // No ancestor node found for child
                    printf("%sNo child node for c %ld of m %ld: This should not happen\n", RED, *itC, *itM);
                }
                // move to the correct next position
                itC = itCNext;
                    
            }
                    
            // no more children in list: all children have genome,
            // this element is not needed anymore
            START_TIMER;
            if (pANMom->m_sChildren.empty()) { 
                //@@                printf("Adding mom %d to dumplist\n", *itM);
                sDumpListM.insert(*itM); 
            }
            
            STOP_TIMER(tInsert);
        } else {
            iResult = -1;
            printf("%sThis should not happen\n", RED);
            printf("%sNo ancestor node for id %ld\n", RED, *itM);
        }
    }

  
    START_TIMER;
    //@@   printf("Checking for childlessness in sPotentialD\n");
    for (ancnodelist::const_iterator itD = mPotentialChildlessDads.begin(); (iResult == 0) && (itD != mPotentialChildlessDads.end()); ++itD) {
        if (itD->second->m_sChildren.empty()) { 
            sDumpListD.insert(itD->first); 
        }
    }
    STOP_TIMER(tInsert);
    /*
    printf("To be deleted: M %zd, D %zd\n", sDumpListM.size(), sDumpListD.size());
    printf("To be added:   M %zd\n", sNextListM.size());
    */
    return iResult;
}

double tAncFindMomU   = 0;
double tAncFindChildU = 0;
double tEraseU        = 0;
double tLBufU         = 0;
double tLoadU         = 0;
double tInsertU       = 0;
//----------------------------------------------------------------------------
// updateListsSeq
//   remove ids marked for dump from current generation set and
//   insert ids from next generation set to current generation set
//
int GraphEvolverF::updateListsSeq(idset &sCurGenM, 
                                const idset &sDumpListM, 
                                const idset &sNextListM) {
    int iResult = 0;
    
    START_TIMER;
    // remove completed ones
    idset_cit it;
    for (it = sDumpListM.begin(); it != sDumpListM.end(); ++it) {
        sCurGenM.erase(*it);
    }
    STOP_TIMER(tEraseU);

    if (iResult == 0) {
        idset sLoad;
        for (it = sNextListM.begin(); (iResult == 0) && (it != sNextListM.end()); ++it) {
            START_TIMER;
            AncestorNode *pANM = m_pAG->findAncestorNode(*it);
            STOP_TIMER(tAncFindMomU);
            if (pANM != NULL) {
                for (idset_cit itC = pANM->m_sChildren.begin(); (iResult == 0) && (itC != pANM->m_sChildren.end()); ++itC) {
                    START_TIMER;
                    AncestorNode *pANChild = m_pAG->findAncestorNode(*itC);
                    STOP_TIMER(tAncFindChildU);
                    if (pANChild == NULL) {
                        sLoad.insert(*itC);
                    }
                }
            } else {
                printf("%s[updateListsSeq] no node for id %ld\n", RED, *it);
                printf("Should not happen, because all in curgen have been loaded\n");
                iResult = -1;
            }
            
        }
        if (iResult == 0) {
            // load the nodes for the children 
            START_TIMER;
            iResult = m_pAGO->loadNodes(m_pAG, sLoad);
            STOP_TIMER(tLoadU);
        }
    } 

    
    if (iResult == 0) {
        // add new ones
        START_TIMER;
        sCurGenM.insert(sNextListM.begin(), sNextListM.end());
        STOP_TIMER(tInsertU);
    }
    return iResult;
} 


//----------------------------------------------------------------------------
// buildGenomeSeq
//
int GraphEvolverF::buildGenomeSeq(uint iSaveBufferSize, const idset &sRoots, const idset &sTargets) {
    int iResult = 0;

    idset sCurGenM;
    idset sCurGenD;
    idset sBigDump;
    ulong iTotalDumped = 0;

    double t0      = 0;
    double tInit   = 0;
    double tSpread = 0;
    double tUpdate = 0;
    double tFilter = 0;
    double tDump   = 0;

    int iSpreadCount = 0;

    if (iSaveBufferSize == 0) {
        iSaveBufferSize = DEF_SAVE_BUFFER_SIZE;
    }

    START_TIMER;
    idset sMyTargets(sTargets.begin(), sTargets.end());
        
    printf("Starting to build genome (new): roots %zd (total %ld); targets %zd\n", sRoots.size(), (m_pAGO != NULL) ? m_pAGO->getNumNodes() : (int)m_pAG->getMap().size(), sTargets.size());
    sCurGenM.insert(sRoots.begin(), sRoots.end());

    if (m_pAGO != NULL) {
        // we need the ancestor nodes of the initial set of ids to start genome building
        idset sLoad;
        for (idset_cit itM = sCurGenM.begin(); (iResult == 0) && (itM != sCurGenM.end()); ++itM) {
            AncestorNode *pANMom = m_pAG->findAncestorNode(*itM);
            if (pANMom == NULL) {
                sLoad.insert(*itM);
            }
        }
        // load the nodes 
        printf("need to load %zd nodes in curgenM: ", sLoad.size());
        for(idset_cit itLoad = sLoad.begin(); itLoad != sLoad.end(); ++itLoad) {
            printf(" %ld", *itLoad);
        }
        printf("\n");
        

        iResult = m_pAGO->loadNodes(m_pAG, sLoad);
    }
    STOP_TIMER(tInit);

    if (iResult == 0) {
     //@@   printf("before genderkill %zd targets\n", sMyTargets.size());
        // now we can separate genders
        separateGenders(sCurGenM, sCurGenD);
        removeChildless(sMyTargets, sCurGenM);
        removeChildless(sMyTargets, sCurGenD); // sCurGenD not needed anymore from here on
     //@@   printf("after removeChildless %zd targets\n", sMyTargets.size());
        
        // now load their children's nodes
        idset sLoad;
        // loop through ids in curgen and check if their children have nodes
        for (idset_cit itM = sCurGenM.begin(); (iResult == 0) && (itM != sCurGenM.end()); ++itM) {
            AncestorNode *pANMom = m_pAG->findAncestorNode(*itM);
            if (pANMom != NULL) {
                for (idset_cit itC = pANMom->m_sChildren.begin(); (iResult == 0) && (itC != pANMom->m_sChildren.end()); ++itC) {
                    AncestorNode *pANChild = m_pAG->findAncestorNode(*itC);
                    if (pANChild == NULL) {
                        sLoad.insert(*itC);
                    }
                }
            } else {
                printf("%s[buildGenomeSeq] no node for id %ld\n", RED, *itM);
                printf("Should not happen, because all in curgen have been loaded\n");
                iResult = -1;
            }
        }
        printf("need to load %zd child nodes in curgenM: ", sLoad.size());
        for(idset_cit itLoad = sLoad.begin(); itLoad != sLoad.end(); ++itLoad) {
            printf(" %ld", *itLoad);
        }
        printf("\n");

        // load the nodes for the children 
        iResult = m_pAGO->loadNodes(m_pAG, sLoad);
    }

    if (iResult == 0) {
        // we keep a copy of the target set which we will modify:
        // each time an id of this set has a genome, it is removed from the set
        printf("after genderkill %zd targets, sMyTargets: %zd\n", sTargets.size(), sMyTargets.size());
        // iNoChange is a counter: if there is no change in the sets after repeated loop iterations,
        // something is wrong and we have to abort
        int iNoChange = 0;
        printf("Before loop: iResult=%d, sCurGenM.size:%zd, sTargets.size()=%zd, iNoChange=%d\n",
               iResult, sCurGenM.size(), sTargets.size(), iNoChange);
        // loop as long as there a targets and ids in sCurGen
        while ((iResult == 0)  && (sTargets.size() > 0) && (sCurGenM.size() > 0) && (iNoChange < 5)) {
            
            idset sNextListM;


            // create genomes for a many children of the ids in sCurGen as possible
            idset sDumpListM;
            idset sDumpListD;
            
            START_TIMER;
            iResult = spreadMotherGenomesSeq(sCurGenM, sDumpListM, sDumpListD, sNextListM);
            iSpreadCount++;
            STOP_TIMER(tSpread);

            if (sNextListM.empty() && sDumpListM.empty() && sDumpListD.empty()) {
                iNoChange++;
            } else {
                if (iResult == 0) {
                    // update the lists
                    START_TIMER;
                    iResult = updateListsSeq(sCurGenM, sDumpListM, sNextListM);
                    STOP_TIMER(tUpdate);
                }
                if (iResult == 0) {
                    // if the dumpable is a target remove it from the target list,
                    // otherwise add it to the big dump list.
                    
                    START_TIMER;
                    // go through mom dump list
                    for (idset_cit it= sDumpListM.begin(); it != sDumpListM.end(); ++it) {
                        if (sMyTargets.find(*it) == sMyTargets.end()) {
                            sBigDump.insert(*it);
                        } else {
                         //@@   printf("not dumping M %d\n", *it);
                            sMyTargets.erase(*it);
                        }
                    }
                    // go through dad dump list?
                    for (idset_cit it= sDumpListD.begin(); it != sDumpListD.end(); ++it) {
                        if (sMyTargets.find(*it) == sMyTargets.end()) {
                            sBigDump.insert(*it);
                        } else {
                         //@@   printf("not dumping D %d\n", *it);
                            sMyTargets.erase(*it);
                        }
                    }
                    STOP_TIMER(tFilter);

                    // we don't dump in every round, only if the size of the
                    // dump list exceeds a ertain limit 
                    START_TIMER;
                    if (sBigDump.size() > iSaveBufferSize) {
                        iTotalDumped += sBigDump.size();
                        printf("\r%16zd processed -> %16ld total  (curgenM %9zd, loaded AncestorN0des %9zd)   ",  
                               sBigDump.size(), iTotalDumped, sCurGenM.size(), m_pAG->getMap().size());fflush(stdout);
                       iResult = dumpCompleted(sBigDump);
                        sBigDump.clear();
                    }
                    STOP_TIMER(tDump);
                }
            }
        }
    
        START_TIMER;
        // dump the remaining ids in the dump list 
        if ((iResult == 0) && (sBigDump.size() > 0)) {
            iTotalDumped += sBigDump.size();
            printf("\r%16zd dumped -> %16ld total   ",  sBigDump.size(), iTotalDumped);fflush(stdout);
            iResult = dumpCompleted(sBigDump);
            printf("\n");
        }
        STOP_TIMER(tDump);
    
        if (iResult == 0) {
            if (sMyTargets.empty()) {
                printf("successfully created genomes for all targets\n");
            } else {
                iResult = -1;
                printf("%zd remaining targets: ", sMyTargets.size());
                for (idset_cit it= sMyTargets.begin(); it != sMyTargets.end(); ++it) {
                    printf(" %ld", *it);
                }
                printf("\n");
            }
        }
 
        fflush(stdout);
    }
    printf("##########################\n");
    printf("Times for GraphEvolver\n");
    printf("Init:   %4.3f\n", tInit);
    printf("Spread: %4.3f\n", tSpread);
    printf("Update: %4.3f\n", tUpdate);
    printf("Filter: %4.3f\n", tFilter);
    printf("Dump:   %4.3f\n", tDump);
    printf("Total   %4.3f\n", omp_get_wtime() - t0);
    printf("##########################\n");
    printf("Times in spreadMotherGenomes2() (%d calls)\n", iSpreadCount);
    printf("AncFindMom:   %4.3f\n", tAncFindMom);
    printf("AncFindChild: %4.3f\n", tAncFindChild);
    printf("AncFindDad:   %4.3f\n", tAncFindDad);
    printf("AncFindTotal: %4.3f\n", tAncFindMom+tAncFindChild+tAncFindDad);
    printf("GenomeFind:   %4.3f\n", tGenomeFind);
    printf("Mix:          %4.3f\n", tMix);
    printf("Insert:       %4.3f\n", tInsert);
    printf("Erase:        %4.3f\n", tErase);
    printf("##########################\n");
    printf("Times in updateLists2() (%d calls)\n", iSpreadCount);
    printf("AncFindMom:   %4.3f\n", tAncFindMomU);
    printf("AncFindChild: %4.3f\n", tAncFindChildU);
    printf("AncFindTotal: %4.3f\n", tAncFindMomU+tAncFindChildU);
    printf("Load          %4.3f\n", tLoadU);
    printf("Insert:       %4.3f\n", tInsertU);
    printf("Erase:        %4.3f\n", tEraseU);
    printf("##########################\n");
    
    return iResult;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////  PARLAYERBUFEXPERIMENTAL  //////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// This version uses LayerBuf and NodeController to actually parallelize the
// main loop in spreadMotherGenomes4.
//-----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// prepareNodesPar
//  Load the AncestorNodes for the specified roots and put them in 
//  the LayerBuf. Then load the AncestorNodes for all children of the roots
//
int GraphEvolverF::prepareNodesPar(const idset &sRoots) {
    int iResult = 0;

    m_pNodeController->clear();


    START_TIMER;
        

    if (m_pAGO != NULL) {
        // we need the ancestor nodes of the initial set of ids to start genome building
        idset sLoad0;
        for (idset_cit itM = sRoots.begin(); (iResult == 0) && (itM != sRoots.end()); ++itM) {
            AncestorNode *pAN = m_pAG->findAncestorNode(*itM);
            if (pAN == NULL) {
                sLoad0.insert(*itM);
            } else {
                if (pAN->m_sChildren.empty()) {
                    // do not remove it - this will happen later in spreadMotherGenome4
                } else {
                    if (pAN->m_iGender == 0) {            
                        uint l = m_pNodeController->getFreeIndex();
                        m_aNodes[l] = pAN;
                    }
                }
            }
        }
        // load the nodes 
        printf("need to load %zd nodes in curgenM: ", sLoad0.size());
        for(idset_cit itLoad = sLoad0.begin(); itLoad != sLoad0.end(); ++itLoad) {
            printf(" %ld", *itLoad);
        }
        printf("\n");
        

        iResult = m_pAGO->loadNodes(m_pAG, sLoad0);
    

        if (iResult == 0) {
            // copy the IDs to the layeredBuf
            for (idset_cit itM = sRoots.begin(); (iResult == 0) && (itM != sRoots.end()); ++itM) {
                //        printf("Checking id %d\n", *itM);
                AncestorNode *pAN = m_pAG->findAncestorNode(*itM);
                if ((pAN != NULL) && (pAN->m_iGender == 0)) {
                    uint l = m_pNodeController->getFreeIndex();
                    m_aNodes[l] = pAN;
                } else {
                    if (pAN == NULL) {
                        iResult = -1;
                        printf("%sNo Node found for id %ld\n", RED, *itM);
                    } else {
                        //                printf("Ignoring male node %d\n", *itM);
                    }
                }
            }
            printf("After init: %d elements in LayerBuf (res %d)\n", m_pNodeController->getNumUsed(), iResult);

        }

        if (iResult == 0) {
        
            // now load their children's nodes
            idset sLoad1;
            
            int iFirstNode = m_pNodeController->getFirstIndex(ACTIVE);
            int iLastNode = m_pNodeController->getLastIndex(ACTIVE);
            if ((iFirstNode != NIL) && (iLastNode != NIL)) {

                for (int iNode = iFirstNode; iNode <= iLastNode; iNode++) {
                    
                    AncestorNode *pANMom = m_aNodes[iNode];
                    if (pANMom != NULL) {
                        for (idset_cit itC = pANMom->m_sChildren.begin(); (iResult == 0) && (itC != pANMom->m_sChildren.end()); ++itC) {
                            AncestorNode *pANChild = m_pAG->findAncestorNode(*itC);
                            if (pANChild == NULL) {
                                sLoad1.insert(*itC);
                            }
                        }
                    } else {
                        printf("%s[prepareNodesPar] No node for index %d\n", RED, iNode);
                        printf("%sShould not happen, because all in curgen have been loaded\n", RED);
                        iResult = -1;
                    }
                }
            } else {
                printf("EMpty list in init...\n");
            }
            printf("need to load %zd child nodes in curgenM4: ", sLoad1.size());
            for(idset_cit itLoad = sLoad1.begin(); itLoad != sLoad1.end(); ++itLoad) {
                printf(" %ld", *itLoad);
            }
            printf("\n");
            
            // load the nodes for the children 
            iResult = m_pAGO->loadNodes(m_pAG, sLoad1);
        }
    }

    return iResult;
}

//----------------------------------------------------------------------------
// buildGenome4
//
int GraphEvolverF::buildGenomePar(uint iSaveBufferSize, const idset &sRoots, const idset &sTargets) {
    int iResult = 0;

    idset sCurGenD;
    idset sBigDump;
    uint iTotalDumped = 0;

    double tInit   = 0;
    double tSpread = 0;
    double tUpdate = 0;
    double tFilter = 0;
    double tDump   = 0;

    int iSpreadCount = 0;

    if (iSaveBufferSize == 0) {
        iSaveBufferSize = DEF_SAVE_BUFFER_SIZE;
    }

    // make a working copy of the targets (ids of completed targets will be removed)
    idset sMyTargets(sTargets.begin(), sTargets.end());

    printf("Starting to build genome4 (new) with %d threads: roots %zd (total %ld); targets %zd\n", m_iNumThreads, sRoots.size(), (m_pAGO != NULL) ? m_pAGO->getNumNodes() : (int)m_pAG->getMap().size(), sTargets.size());
    printf("Parallel loading is%s set\n", m_bParLoad?"":" not");
    // load all required nodes (for roots and their children)
    iResult = prepareNodesPar(sRoots);

    if (iResult == 0) {
        // we keep a copy of the target set which we will modify:
        // each time an id of this set has a genome, it is removed from the set
        printf("after genderkill %zd targets, sMyTargets: %zd\n", sTargets.size(), sMyTargets.size());
        // allocate arrays for parallelization
        idset *asNextListM = new idset[m_iNumThreads];
        idset *asDumpListM = new idset[m_iNumThreads];
        idset *asDumpListD = new idset[m_iNumThreads];
        m_asChildlessDadIDs = new idset[m_iNumThreads];
        m_asChildlessMomListIndexes = new idset[m_iNumThreads];
        m_amNewGenomes = new idgenome[m_iNumThreads];

        // iNoChange is a counter: if there is no change in the sets after repeated loop iterations,
        // something is wrong and we have to abort
        int iNoChange = 0;
        printf("Before loop: iResult=%d, aNodes:%u, sTargets.size()=%zd, iNoChange=%d\n",
               iResult, m_pNodeController->getNumUsed(), sTargets.size(), iNoChange);
        // loop as long as there a targets and ids in aNodes
        while ((iResult == 0)  && (sTargets.size() > 0) && (m_pNodeController->getNumUsed() > 0) && (iNoChange < 5)) {

            // clear sets            
            for (int i =0; i < m_iNumThreads; ++i) {
                asNextListM[i].clear();
                asDumpListM[i].clear();
                asDumpListD[i].clear();
                m_asChildlessDadIDs[i].clear();
                m_asChildlessMomListIndexes[i].clear();
                m_amNewGenomes[i].clear();
            }
                       
            // create genomes for a many children of the ids in asNodes as possible
            START_TIMER;
            iResult = spreadMotherGenomesPar(asDumpListM, asDumpListD, asNextListM);
            iSpreadCount++;
            STOP_TIMER(tSpread);

            bool bAllEmpty = true;
            for (int i =0; bAllEmpty && (i < m_iNumThreads); ++i) {
                bAllEmpty = bAllEmpty && (asNextListM[i].empty()  && asDumpListM[i].empty() && asDumpListD[i].empty()) ;
            }

            if (bAllEmpty) {
                iNoChange++;
            } else {
                if (iResult == 0) {
                    // update the lists
                    START_TIMER;
                    iResult = updateListsPar(asNextListM);
                    STOP_TIMER(tUpdate);
                }
                if (iResult == 0) {
                    // if the dumpable ID is a target, remove it from the target list,
                    // otherwise add it to the big dump list.
                    for (int i = 0; i < m_iNumThreads; ++i) {
                        START_TIMER;
                        for (idset_cit it= asDumpListM[i].begin(); it != asDumpListM[i].end(); ++it) {
                            if (sMyTargets.find(*it) == sMyTargets.end()) {
                                sBigDump.insert(*it);
                            } else {
                                sMyTargets.erase(*it);
                            }
                        }
                        for (idset_cit it= asDumpListD[i].begin(); it != asDumpListD[i].end(); ++it) {
                            if (sMyTargets.find(*it) == sMyTargets.end()) {
                                sBigDump.insert(*it);
                            } else {
                                sMyTargets.erase(*it);
                            }
                        }
                        STOP_TIMER(tDump);
                    }
                    // we don't dump in every round, only if the size of the
                    // dump list exceeds a ertain limit 
                    START_TIMER;
                    if (sBigDump.size() > iSaveBufferSize) {
                        iTotalDumped += (uint)sBigDump.size();
                        printf("\r%16zd processed -> %16d total  (curgenM %9u, loaded AncestorN0des %9zd)   ",  
                               sBigDump.size(), iTotalDumped, m_pNodeController->getNumUsed(), m_pAG->getMap().size());fflush(stdout);
                       iResult = dumpCompleted(sBigDump);
                        sBigDump.clear();
                    }
                    STOP_TIMER(tDump);
                }
            }
        }
    
        START_TIMER;
        // dump the remaining ids in the dump list 
        if ((iResult == 0) && (sBigDump.size() > 0)) {
            iTotalDumped += (uint)sBigDump.size();
            printf("\r%16zd processed -> %16d total   ",  sBigDump.size(), iTotalDumped);fflush(stdout);
            iResult = dumpCompleted(sBigDump);
            printf("\n");
        }
        STOP_TIMER(tDump);
 
        delete[] asNextListM;
        delete[] asDumpListD;
        delete[] asDumpListM;
        delete[] m_asChildlessDadIDs;
        delete[] m_asChildlessMomListIndexes;
        delete[] m_amNewGenomes;

        if (iResult == 0) {
            if (sMyTargets.empty()) {
                printf("successfully created genomes for all targets\n");
            } else {
                iResult = -1;
                printf("%zd remaining targets: ", sMyTargets.size());
                for (idset_cit it= sMyTargets.begin(); it != sMyTargets.end(); ++it) {
                    printf(" %ld", *it);
                }
                printf("\n");
            }
        }
 
        fflush(stdout);
    }
    printf("##########################\n");
    printf("Times for buildGenomes4\n");
    printf("Init:   %4.3f\n", tInit);
    printf("Spread: %4.3f\n", tSpread);
    printf("Update: %4.3f\n", tUpdate);
    printf("Filter: %4.3f\n", tFilter);
    printf("Dump:   %4.3f\n", tDump);
    printf("Total   %4.3f\n", omp_get_wtime() - t0);
    printf("##########################\n");
    printf("Times in spreadMotherGenomes4() (%d calls)\n", iSpreadCount);
    printf("AncFindMom:   %4.3f\n", tAncFindMom);
    printf("AncFindChild: %4.3f\n", tAncFindChild);
    printf("AncFindDad:   %4.3f\n", tAncFindDad);
    printf("AncFindTotal: %4.3f\n", tAncFindMom+tAncFindChild+tAncFindDad);
    printf("GenomeFind:   %4.3f\n", tGenomeFind);
    printf("Mix:          %4.3f\n", tMix);
    printf("Insert:       %4.3f\n", tInsert);
    printf("Erase:        %4.3f\n", tErase);
    printf("##########################\n");
    printf("Times in updateLists4() (%d calls)\n", iSpreadCount);
    printf("AncFindMom:   %4.3f\n", tAncFindMomU);
    printf("AncFindChild: %4.3f\n", tAncFindChildU);
    printf("AncFindTotal: %4.3f\n", tAncFindMomU+tAncFindChildU);
    printf("LayerBuf:     %4.3f\n", tLBufU);
    printf("Load:         %4.3f\n", tLoadU);
    printf("Insert:       %4.3f\n", tInsertU);
    printf("Erase:        %4.3f\n", tEraseU);
    printf("##########################\n");
    
    return iResult;
}


//----------------------------------------------------------------------------
// updateListsPar
//   remove ids marked for dump from current generation set and
//   insert ids from next generation set to current generation set
//
int GraphEvolverF::updateListsPar(const idset *asNextListM) {
    int iResult = 0;
    
    if (iResult == 0) {
        idset sLoad;
        // place all new nodes into the LayerBuf
        // these nodes do exist, otherwise they would not be in the nextLists
        for (int i =0; i < m_iNumThreads; ++i) {
            for (idset_cit it = asNextListM[i].begin(); (iResult == 0) && (it != asNextListM[i].end()); ++it) {
                START_TIMER;
                AncestorNode *pANM = m_pAG->findAncestorNode(*it);
                STOP_TIMER(tAncFindMomU);
                if (pANM != NULL) {
                    // add the node to the array
                    START_TIMER;
                    uint l = m_pNodeController->getFreeIndex();
                    m_aNodes[l] = pANM;
                    STOP_TIMER(tLBufU);

                    // collect load all their babies' ids
                    for (idset_cit itC = pANM->m_sChildren.begin(); (iResult == 0) && (itC != pANM->m_sChildren.end()); ++itC) {
                        START_TIMER;
                        AncestorNode *pANChild = m_pAG->findAncestorNode(*itC);
                        STOP_TIMER(tAncFindChildU);
                        if (pANChild == NULL) {
                            START_TIMER;
                            sLoad.insert(*itC);
                            STOP_TIMER(tInsertU);
                        }
                    }
                } else {
                    printf("%s[updateListsPar] No node for id %ld\n", RED, *it);
                    printf("%sShould not happen, because all in curgen have been loaded\n", RED);
                    iResult = -1;
                }
                
            }
        }
        if (iResult == 0) {
            // load the nodes for the children 
            START_TIMER;
            if (m_bParLoad) {
                iResult = m_pAGO->loadNodesPar(m_pAG, sLoad);
            } else {
                iResult = m_pAGO->loadNodes(m_pAG, sLoad);
            }
            STOP_TIMER(tLoadU);
        }
    } 

    
    return iResult;
} 


//----------------------------------------------------------------------------
// spreadMotherGenomesPar
//   m_aNodes contains the current generation of females.
//   For every female f in m_aNodes we check the number of its children.
//   If f has no children it is completed and can be dumped.
//   Otherwise, we loop through f's children.
//   If such a c's father already has a node and a genome, 
//   we can create a genome for c, and place it on the list for the next round. 
//   The child c is then erased from both its parents' children lists.
//
//   If any of the parents ends up with an empty child list, it can be dumped.
//
//   If the father has no node and no genome yet it has not been created so far,
//   we put f onthe list for the next round. 
//
//      
//
int GraphEvolverF::spreadMotherGenomesPar(idset *asDumpListM, idset *asDumpListD, idset *asNextListM) {
    int iResult = 0;

    // loop through all Moms (sCurGenM)
    int iFirstNode = m_pNodeController->getFirstIndex(ACTIVE);
    int iLastNode = m_pNodeController->getLastIndex(ACTIVE);
    if ((iFirstNode != NIL) && (iLastNode != NIL)) {
        // ATTENTION: the loop below goes through all entries in the LayerBuf, dead or alive
        // unused spaces are set to NULL

#pragma omp parallel for schedule(dynamic)
        for (int iNode = iFirstNode; iNode <= iLastNode; iNode++) {

            int iThread = omp_get_thread_num();
            AncestorNode *pANMom = m_aNodes[iNode];
            if (pANMom != NULL) {
                
                // get the mom genome now because it might be used several times
                if (!pANMom->m_sChildren.empty()) {
                    // get Mom's genome (should have one)
                    START_TIMER2;
                    idgenome::const_iterator itGM = m_mGenomes.find(pANMom->m_iID);
                    STOP_TIMER2(tGenomeFind);
                    if (itGM != m_mGenomes.end()) {

                        // loop through children of Mom
                        idset_cit itC;
                        // NOTE: we do not increment the iterator in the for-statement, because we may remove a child from the set of children 
                        for (itC = pANMom->m_sChildren.begin(); (iResult == 0) && (itC != pANMom->m_sChildren.end()); ) {
                            // for the case of child deletion, we need an iterator to the next element
                            idset_cit itCNext = itC;  
                            itCNext++;
                            /*@@@@@
                            //#pragma omp critical
                            {
                                printf("TMC: %d %ld %ld\n", iThread, pANMom->m_iID, *itC);fflush(stdout);
                            }
                            @@@@*/
                            // get AncestorNode of child (so we can find the Dad)
                            START_TIMER2;
                            AncestorNode *pANChild = m_pAG->findAncestorNode(*itC);
                            STOP_TIMER2(tAncFindChild);
                            if (pANChild != NULL) {
                                
                                // find Dad's genome (may not be created yet) 
                                idtype iDadID = pANChild->getDad();
                                START_TIMER2;
                                idgenome::const_iterator itGD = m_mGenomes.find(iDadID);
                                STOP_TIMER2(tGenomeFind);
                                if (itGD != m_mGenomes.end()) {
                                
                                    // crossing-over and mutate to get child's genome
                                    START_TIMER2;
                                    ulong *pGenome = createMix(itGM->second, itGD->second);
                                    STOP_TIMER2(tMix);
                                    
                                    // save child's genome 
                                    START_TIMER2;
                                    // attention! parallel writing!
                                    m_amNewGenomes[iThread][*itC] = pGenome;
                                
                                    // if the child has no children, we can dump itC 
                                    // Due to the construction of the AncvestorGraph
                                    // this is only possible "naturally" for target nodes.
                                    if (pANChild->m_sChildren.empty()) {
                                        if (pANChild->m_iGender == 0) {
                                            asDumpListM[iThread].insert(*itC); // sDumpListM[iThread]
                                        } else {
                                            asDumpListD[iThread].insert(*itC); // sDumpListD[iThread]
                                        }
                                    } else {
                                        // otherwise, if female, it will be Mom in the next round
                                        if (pANChild->m_iGender == 0) {
                                            asNextListM[iThread].insert(*itC);      // sNextListM[iThread]
                                        }
                                    }
                                    STOP_TIMER2(tInsert);
                                    // child is complete, remove it from its parents' child lists
                                    START_TIMER2;
                                    AncestorNode *pANDad =  m_pAG->findAncestorNode(iDadID);
                                    STOP_TIMER2(tAncFindDad);
                                    START_TIMER2;
                                    if (pANDad != NULL) {
                                        
                                        // if the AncestorGraph is built correctly,
                                        // *itC is in pANDad->m_sChildren
                                        // CRITICAL: it is conceivable that two children by 
                                        // two different mothers are erased at the same time.
                                        // however, this should be rare ...
#pragma omp critical
                                        {
                                            pANDad->m_sChildren.erase(*itC);
                                        }
                                        if (pANDad->m_sChildren.empty()) {
                                            m_asChildlessDadIDs[iThread].insert(iDadID); // sPotentialD[iThread]
                                        }
                                    } else {
                                        printf("Genome but no dad!\n");
                                        iResult = -1;
                                    }
                                    // remove it from the mom
                                    pANMom->m_sChildren.erase(*itC);
                                    STOP_TIMER2(tErase);
                                    
                                } else {
                                    // no partner genome: we can't build genome for child yet.
                                    // parent will stay in CurGen list
                                }
                            } else {
                                iResult = -1;
                                // No ancestor node found for child
                                printf("%sNo child node for c %ld of m %ld: This should not happen\n", RED, *itC, pANMom->m_iID);
                            }
                            // move to the correct next position
                            itC = itCNext;
                            
                        } // child loop

                    } else {
                        iResult = -1;
                        printf("%sNo genome for id %ld: This should not happen\n", RED, pANMom->m_iID);
                    }
                }                
                // no more children in list: all children have genome,
                // this element is not needed anymore
                START_TIMER2;
                
                if (pANMom->m_sChildren.empty()) { 
                    asDumpListM[iThread].insert(pANMom->m_iID); 
                    m_asChildlessMomListIndexes[iThread].insert(iNode);
                }
                
                STOP_TIMER2(tInsert);
            } else {
                // this is ok: erased nodes are set to NULL
                // nothing to do
            }
        } // mom loop
        
        START_TIMER2;
        //@@   printf("Checking for childlessness in sPotentialD\n");
        for (int i = 0; i < m_iNumThreads; i++) {
            // add all childess dads to the dump list
            for (idset_cit itD = m_asChildlessDadIDs[i].begin(); (iResult == 0) && (itD != m_asChildlessDadIDs[i].end()); ++itD) {
                asDumpListD[i].insert(*itD); 
            }

            // the childless moms' ids are already in the dump list,
            // but they have to be removed from the LayerBuf
            for (idset_cit itM = m_asChildlessMomListIndexes[i].begin(); (iResult == 0) && (itM != m_asChildlessMomListIndexes[i].end()); ++itM) { 
                // clear the entry
                m_aNodes[(uint)*itM] = NULL;
                // remove it from used list
                m_pNodeController->deleteElement((uint)*itM);

            }

        }

        // collect all new genomes
        for (int i = 0; i < m_iNumThreads; i++) {
            m_mGenomes.insert(m_amNewGenomes[i].begin(), m_amNewGenomes[i].end());
        }

        STOP_TIMER2(tInsert);
        /*
        listIntSet2("dumpM: ", sDumpListM);
        listIntSet2("dumpD: ", sDumpListD);
        listIntSet2("NextM: ", sNextListM);
        */
        /*
        printf("To be deleted: M %zd, D %zd\n", sDumpListM.size(), sDumpListD.size());
        printf("To be added:   M %zd\n", sNextListM.size());
        */
    } else {
        printf("Empty List?\n");
    }
    return iResult;
}
