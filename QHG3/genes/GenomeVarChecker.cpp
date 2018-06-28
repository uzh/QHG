#include <stdio.h>
#include <string.h>

#include <vector>
#include <algorithm>
#include <hdf5.h>
#include <omp.h>

#include "types.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "QDFUtils.h"
#include "BinGeneFile.h"
#include "GenomeVarChecker.h"

#define DEF_BITS_PER_NUC 2
#define BUFSIZE_GENOMES 100000


//----------------------------------------------------------------------------
// createInstance
//  use for QDF files
//    pQDFPopFile :     Population QDF file
//    pSpeciesName :    Name of species for which to extract genomes
//                      (if NULL, first species will be used)
//    pAttrGenomeSize : name of genome size attribute
//    pDataSetGenome :  name of genome dataset
//
GenomeVarChecker *GenomeVarChecker::createInstance(const char *pQDFPopFile, 
                                                   const char *pSpeciesName, 
                                                   const char *pAttrGenomeSize, 
                                                   const char *pAttrBitsPerNuc, 
                                                   const char *pDataSetGenome,
                                                   int iBufSizeGenomes) {

    GenomeVarChecker *pGVC = new GenomeVarChecker(iBufSizeGenomes);
    int iResult = pGVC->init_qdf(pQDFPopFile, pSpeciesName, pAttrGenomeSize, pAttrBitsPerNuc, pDataSetGenome);
    if (iResult != 0) {
        delete pGVC;
        pGVC = NULL;
    }
    return pGVC;
}
    
                              
//----------------------------------------------------------------------------
// createInstance
//  use for bin gene files
//    pBinGeneFile :     bin Gene file (as e.g. created by QDFSampler)
//
GenomeVarChecker *GenomeVarChecker::createInstance(const char *pBinGeneFile,
                                                   int iBufSizeGenomes) {

    GenomeVarChecker *pGVC = new GenomeVarChecker(iBufSizeGenomes);
    int iResult = pGVC->init_bin(pBinGeneFile);
    if (iResult != 0) {
        delete pGVC;
        pGVC = NULL;
    }
    return pGVC;
}             


//----------------------------------------------------------------------------
// constructor
//
GenomeVarChecker::GenomeVarChecker(int iBufSizeGenomes)
    : m_aNucCountTemp(NULL),
      m_aNucCount(NULL),
      m_acCounts2(NULL),
      m_aNonZeroCountTemp(NULL),
      m_aNonZeroCount(NULL),
      m_hFile(H5P_DEFAULT),
      m_hPopulation(H5P_DEFAULT),
      m_hSpecies(H5P_DEFAULT),
      m_iGenomeSize(0),
      m_iNumBlocks(0),
      m_iNumGenomes(0),
      m_iBitsPerNuc(0),
      m_iNumNucs(0),
      m_iBufSizeGenomes((iBufSizeGenomes>0) ? iBufSizeGenomes : BUFSIZE_GENOMES),
      m_pDataSetGenome(NULL) {

}


//----------------------------------------------------------------------------
// destructor
//
GenomeVarChecker::~GenomeVarChecker() {
    qdf_closeFile(m_hFile);
    qdf_closeGroup(m_hPopulation);
    qdf_closeGroup(m_hSpecies);

    if (m_pDataSetGenome != NULL) {
        delete[] m_pDataSetGenome;
    }

    if (m_aNucCountTemp != NULL) {
        for (int t = 0; t < omp_get_max_threads() ; t++) {
            for (uint i = 0; i < m_iNumNucs; i++) {
                delete[] m_aNucCountTemp[t][i];
            }
            delete[] m_aNucCountTemp[t];
        }
        delete[] m_aNucCountTemp;
    }
    if (m_aNucCount != NULL) {
        for (uint i = 0; i < m_iNumNucs; i++) {
            delete[] m_aNucCount[i];
        }
        delete[] m_aNucCount;
    }

    if (m_acCounts2 != NULL) {
        delete[] m_acCounts2;
    }

    if (m_aNonZeroCountTemp != NULL) {
        for (int t = 0; t < omp_get_max_threads() ; t++) {
            delete[] m_aNonZeroCountTemp[t];
        }
        delete[] m_aNonZeroCountTemp;
    }
    if (m_aNonZeroCount != NULL) {
        delete[] m_aNonZeroCount;
    }
}


//----------------------------------------------------------------------------
// prepareArrays
//   m_iGenomeSize *must* be set!
//
void GenomeVarChecker::prepareArrays() {

    m_aNucCountTemp = new int**[omp_get_max_threads()];
    for (int t = 0; t < omp_get_max_threads(); t++) {
        m_aNucCountTemp[t] = new int*[m_iNumNucs];
        for (uint i = 0; i < m_iNumNucs; i++) {
            m_aNucCountTemp[t][i] = new int[2*m_iGenomeSize];
            memset(m_aNucCountTemp[t][i], 0, 2*m_iGenomeSize*sizeof(int));
        }
    }
    
    m_aNucCount = new int*[m_iNumNucs];
    for (uint i = 0; i < m_iNumNucs; i++) {
        m_aNucCount[i] = new int[2*m_iGenomeSize];
        memset(m_aNucCount[i], 0, 2*m_iGenomeSize*sizeof(int));
    }

    m_aNonZeroCountTemp = new int*[omp_get_max_threads()];
    for (int t = 0; t < omp_get_max_threads(); t++) {
        m_aNonZeroCountTemp[t] = new int[2*m_iGenomeSize];
        memset(m_aNonZeroCountTemp[t], 0, 2*m_iGenomeSize*sizeof(int));
        
    }
    m_aNonZeroCount = new int[2*m_iGenomeSize];
    memset(m_aNonZeroCount, 0, 2*m_iGenomeSize*sizeof(int));
}


//----------------------------------------------------------------------------
// init_qdf
//
int GenomeVarChecker::init_qdf(const char *pQDFPopFile, 
                               const char *pSpeciesName, 
                               const char *pAttrGenomeSize,
                               const char *pAttrBitsPerNuc,
                               const char *pDataSetGenome) {

    int iResult = -1;

    if (pAttrGenomeSize != NULL) {

        char *pPopName = NULL;
        if (pSpeciesName == NULL) {
            pPopName = qdf_getFirstPopulation(pQDFPopFile);
        } else {
            pPopName = qdf_checkForPop(pQDFPopFile, pSpeciesName);
        }
        if (pPopName != NULL) {
            m_pDataSetGenome = new char[strlen(pDataSetGenome)+1];
            strcpy(m_pDataSetGenome, pDataSetGenome);
                
            printf("%s: using population [%s]\n", pQDFPopFile, pPopName);
                
            // open qdf and get species
            // we know file, pop group and species group exist
            m_hFile        = qdf_openFile(pQDFPopFile);
            m_hPopulation  = qdf_openGroup(m_hFile, POPGROUP_NAME);
            m_hSpecies     = qdf_openGroup(m_hPopulation, pPopName);
            iResult = qdf_extractAttribute(m_hSpecies, pAttrGenomeSize, 1, &m_iGenomeSize);
            if (iResult == 0) {
                
                iResult = qdf_extractAttribute(m_hSpecies, pAttrBitsPerNuc, 1, &m_iBitsPerNuc);
                if (iResult != 0) {
                    // no such entry -> default
                    m_iBitsPerNuc = DEF_BITS_PER_NUC;
                    iResult = 0;
                }
                m_iNumNucs = 1 << m_iBitsPerNuc;
                if (m_iBitsPerNuc == 1) {
                    m_iNumBlocks = BitGeneUtils::numNucs2Blocks(m_iGenomeSize);
                } else {
                    m_iNumBlocks = GeneUtils::numNucs2Blocks(m_iGenomeSize);
                }
                
                // m_iGenomeSize is known, so we can prepare the arrays
                prepareArrays();

                iResult = processGenomesPar(pDataSetGenome);
            } else {
                fprintf(stderr, "Couldn't extract Attribute [%s] for genome size\n", pAttrGenomeSize);
            }
            
 
            delete[] pPopName;
            
        } else {
            if (pSpeciesName == NULL) {
                fprintf(stderr, "Couldn't find any population in [%s]\n", pQDFPopFile);
            } else {
                fprintf(stderr, "Couldn't get population [%s] from [%s]\n", pSpeciesName, pQDFPopFile);
            }
        }
    } else {
        fprintf(stderr, "The name for the GenomeSize attribute must not be NULL\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// init_bin
//
int GenomeVarChecker::init_bin(const char *pBinGeneFile) {


    int iResult = -1;

    m_iNumGenomes = -1;
    BinGeneFile *pBG = BinGeneFile::createInstance(pBinGeneFile);
    if (pBG != NULL) {
        m_iNumGenomes = pBG->read();
        if (m_iNumGenomes <= 0) {
            delete pBG;
            pBG = NULL;
        } else {
            printf("NumGenomes: %d\n", m_iNumGenomes);

            m_iGenomeSize = pBG->getGenomeSize();
            m_iBitsPerNuc = pBG->getBitsPerNuc();
            m_iNumNucs = 1 << m_iBitsPerNuc;
            if (m_iBitsPerNuc == 1) {
                m_iNumBlocks  = BitGeneUtils::numNucs2Blocks(m_iGenomeSize);
            } else {
                m_iNumBlocks  = GeneUtils::numNucs2Blocks(m_iGenomeSize);
            }

            const id_genomes &mIDGen = pBG->getIDGen();
            
            // m_iGenomeSize is known, so we can prepare the arrays
            prepareArrays();

            // read buffer
            ulong iReadBufSize = m_iBufSizeGenomes*2*m_iNumBlocks;
            ulong *aBuf = new ulong[iReadBufSize];
            memset(aBuf, 0, iReadBufSize*sizeof(ulong));

            int iC = 0;
            int iTot = 0;
            id_genomes::const_iterator it;
            for (it = mIDGen.begin(); it != mIDGen.end(); ++it) {
                memcpy(aBuf+iC*2*m_iNumBlocks, it->second, 2*m_iNumBlocks*sizeof(ulong));
                iC++;
                if (iC == m_iBufSizeGenomes) {
#pragma omp parallel for
                    for (int i = 0; i < 2*iC; i++) {
                        iResult = explicitCounts(aBuf+i*m_iNumBlocks);
                    }
                    iTot +=iC;
                    iC    = 0;
                }
            }
            if (iC > 0) {
                printf("Rest:\n");
#pragma omp parallel for
                for (int i = 0; i < 2*iC; i++) {
                    iResult = explicitCounts(aBuf+i*m_iNumBlocks);
                }
                iTot +=iC;
            }
         
            printf("processed %d half genes (expected: %zd)\n", 2*iTot, 2*mIDGen.size());
            if (iResult == 0) {
                printf("post-processing\n");
                iResult = accumulateThreadData();
                iResult = makeCounts();
                iResult = calcOderedFreqs();
            }
            
        }

    }
    return iResult;
}


//----------------------------------------------------------------------------
// processGenomesPar
//
int GenomeVarChecker::processGenomesPar(const char *pDataSetGenome) {
    int iResult = 0;
    double dT0 = omp_get_wtime();

    // read buffer
    ulong iReadBufSize = m_iBufSizeGenomes*2*m_iNumBlocks;
    ulong *aBuf = new ulong[iReadBufSize];
   
    // open the DataSet and data space
    //    if (m_bVerbose) printf("Opening data set [%s] in species\n", pDataSetGenome); fflush(stdout);
    hid_t hDataSet = H5Dopen2(m_hSpecies, pDataSetGenome, H5P_DEFAULT);
    hid_t hDataSpace = H5Dget_space(hDataSet);

    // get total number of elements in dataset
    hsize_t dims;
    herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
    m_iNumGenomes = dims/(2*m_iNumBlocks);
    printf("NumGenomes: %d\n", m_iNumGenomes);
    // initialize some counters and indexes
    hsize_t iCount;
    hsize_t iOffset = 0;

    int iGlobalOffset = 0;

    // loop until all elements have been read
    while ((iResult == 0) && (dims > 0)) {
        // can we get a full load of the buffer?
        if (dims > iReadBufSize) {
            iCount = iReadBufSize;
        } else {
            iCount = dims;
        }
        
        // read a slab
        hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
        status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                     &iOffset, NULL, &iCount, NULL);
        status = H5Dread(hDataSet, H5T_NATIVE_LONG, hMemSpace,
                         hDataSpace, H5P_DEFAULT, aBuf);
        
        if (status >= 0) {
            
            // distribute the data 
            int iNumThisPass = iCount/m_iNumBlocks;
#pragma omp parallel for
            for (int i = 0; i < iNumThisPass; i++) {
                iResult = explicitCounts(aBuf+i*m_iNumBlocks);
            }
            
            iGlobalOffset += iNumThisPass;
            dims          -= iCount;
            iOffset       += iCount;
            
        } else {
	    fprintf(stderr, "Error during slab reading\n");
	    iResult = -1;
        }
    }
    double dTM = omp_get_wtime();

    if (iResult == 0) {
        printf("post-processing\n");
        iResult = accumulateThreadData();
        iResult = makeCounts();
        iResult = calcOderedFreqs();
    }
    //fprintf(stderr,"\n");
    double dT1 = omp_get_wtime();
    printf("processGenome: %.2f secs\n", dTM-dT0);
    printf("makeCounts:    %.2f secs\n", dT1-dTM);


    qdf_closeDataSpace(hDataSpace);
    qdf_closeDataSet(hDataSet);
    delete[] aBuf;
    return iResult;
}


//----------------------------------------------------------------------------
// accumulateThreadData
//
int GenomeVarChecker::accumulateThreadData() {
    int iResult = 0;

    for (int i = 0; i < m_iGenomeSize; i++) {
        int acc[m_iNumNucs];
        memset(acc, 0, m_iNumNucs*sizeof(int));

        // merge entries for all threads into index 0
        for (int t = 0; t < omp_get_max_threads(); t++) {
            for (uint k = 0; k< m_iNumNucs; k++) {
                acc[k] += m_aNucCountTemp[t][k][i];
            }
        }
        for (uint k = 0; k< m_iNumNucs; k++) {
            m_aNucCount[k][i]=acc[k];
        }


        m_aNonZeroCount[i] = 0;
        for (int t = 0; t < omp_get_max_threads(); t++) {
            m_aNonZeroCount[i] += m_aNonZeroCountTemp[t][i];
        }

    }
    return iResult;
}


//----------------------------------------------------------------------------
// makeCounts
//
int GenomeVarChecker::makeCounts() {
    int iResult = 0;
    
    m_acCounts2 = new uchar[m_iGenomeSize];
    memset(m_acCounts2, 0, m_iGenomeSize*sizeof(uchar));
#pragma omp parallel for
    for (int i = 0; i < m_iGenomeSize; i++) {
        int h = 0;
        for (uint k = 0; k< m_iNumNucs; k++) {
            if (m_aNucCount[k][i] > 0) {
                h++;
            }
        }
        m_acCounts2[i]=h;
    }

    return iResult;
}


//----------------------------------------------------------------------------
// calcOderedFreqs
//
int GenomeVarChecker::calcOderedFreqs() {
    int iResult = 0;


    float **afOrderedFreqs = new float*[omp_get_max_threads()];
    for (int t = 0; t < omp_get_max_threads(); ++t) {
        afOrderedFreqs[t] = new float[m_iNumNucs];
        memset(afOrderedFreqs[t], 0, m_iNumNucs*sizeof(float));
    }
#pragma omp parallel for
    for (int i = 0; i < m_iGenomeSize; i++) {
        std::vector<int> sTemp;
        // merge entries for all threads into index 0
        for (uint k = 0; k< m_iNumNucs; k++) {
            sTemp.push_back(m_aNucCount[k][i]);
        }
        std::sort(sTemp.begin(), sTemp.end());
        int h = 0;
        std::vector<int>::const_iterator it;
        for (it = sTemp.begin(); it != sTemp.end(); ++it) {
            afOrderedFreqs[omp_get_thread_num()][h++] += *it;
        }
    }

    memset(m_afOrderedFreqs, 0, m_iNumNucs*sizeof(float));
    // accumulate
    for (uint k = 0; k< m_iNumNucs; k++) {
        for (int t = 0; t< omp_get_max_threads(); t++) {
            m_afOrderedFreqs[k] +=  afOrderedFreqs[t][k];
        }
    }
    for (uint k = 0; k< m_iNumNucs; k++) {
        m_afOrderedFreqs[k] /= m_iGenomeSize;
    }

    // clean up
    for (int t = 0; t< omp_get_max_threads(); t++) {
        delete[] afOrderedFreqs[t];
    }
    delete[] afOrderedFreqs;

    return iResult;
}


//----------------------------------------------------------------------------
// explicitCounts
//
int GenomeVarChecker::explicitCounts(ulong * pGenome) {
    int iResult = 0;
    
    uint iNucsInBlock = 0;
    uint iMask = 0;

    if (m_iBitsPerNuc == 1) {
        iNucsInBlock = BitGeneUtils::NUCSINBLOCK;
        iMask = 0x1;
    } else {
        iNucsInBlock = GeneUtils::NUCSINBLOCK;
        iMask = 0x3;
    }
    for (int i = 0; i < m_iNumBlocks; i++) {
        for (uint j = 0; j < iNucsInBlock; j++) {
            long x = (pGenome[i] >> m_iBitsPerNuc*j) & iMask;
            m_aNucCountTemp[omp_get_thread_num()][x][i*iNucsInBlock+j]++;
            m_aNonZeroCountTemp[omp_get_thread_num()][i*iNucsInBlock+j] += (x!=0);
        }
    }


    return iResult;
     
}
