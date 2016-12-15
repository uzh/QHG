#include <stdio.h>
#include <string.h>
#include <math.h>

#include <algorithm>

#include "strutils.h"
#include "colors.h"
#include "GeneUtils.h"
#include "GenomeProvider.h"
#include "GeneWriter.h"

//----------------------------------------------------------------------------
// format accepted
//
bool GeneWriter::formatAccepted(const char *pFormat) {
    bool bAccepted = false;
    if (strcasecmp(pFormat, FORMAT_PLINK) == 0) {
        bAccepted = true;
    } else if (strcasecmp(pFormat, FORMAT_BIN) == 0) {
        bAccepted = true;
    } else if (strcasecmp(pFormat, FORMAT_ASC) == 0) {
        bAccepted = true;
    } else {
        bAccepted = false;
    }
    return bAccepted;
}


//----------------------------------------------------------------------------
// writeGenes
//  write ID, mom id, dad id, gender and genome
//  with sub headers for different locations
//
int GeneWriter::writeGenes(const char *pFormat, GenomeProvider *pGP, const char *pOutputFile,  const locdata &mLocData,  const IDSample *pSample) {
    int iResult = 0;

    if (pOutputFile != NULL) {

        
        if (strcasecmp(pFormat, FORMAT_PLINK) == 0) {
            printf("Writing plink output (genome size %d) to [%s]\n", 
                   pGP->getGenomeSize(), pOutputFile);
            iResult = writeGenesPlink(pGP, pOutputFile, mLocData, pSample);

        } else if (strcasecmp(pFormat, FORMAT_BIN) == 0) {
            printf("Writing binary output (genome size %d) to [%s]\n", 
                   pGP->getGenomeSize(), pOutputFile);
            iResult = writeGenesBin(pGP, pOutputFile, mLocData, pSample);

        } else if (strcasecmp(pFormat, FORMAT_ASC) == 0) {
            printf("Writing ascii output (genome size %d) to [%s]\n", 
                   pGP->getGenomeSize(), pOutputFile);
            iResult = writeGenesAsc(pGP, pOutputFile, mLocData, pSample);
            
        } else {
            printf("%sUnknown format [%s]\n", RED, pFormat);
            iResult = -1;
        }
    } else {
        printf("%sNo file name given\n", RED);
        iResult = -1;
    }
    return iResult;
}



//----------------------------------------------------------------------------
// writeGenesPlink
//  write a file in the plink ".ped" format and a dummy map file
//
int GeneWriter::writeGenesPlink(GenomeProvider *pGP, const char *pOutputFile,  const locdata &mLocDefs,  const IDSample *pSample) {
    int iResult = -1;
    
    FILE *fOut = fopen(pOutputFile, "wt");
    if (fOut != NULL) {
        iResult = 0;
        
        int iGenomeSize = pGP->getGenomeSize();
        //int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
        locagd msLocAD;
        pSample->getLocationADSet(msLocAD);
        locagd::const_iterator it_ltd;
        for (it_ltd = msLocAD.begin(); (iResult == 0) && (it_ltd != msLocAD.end()); ++it_ltd) {
            int iC = 0;
            std::set<agdata *>::const_iterator it_ta;
            for (it_ta = it_ltd->second.begin(); it_ta != it_ltd->second.end(); ++it_ta) {
                const agdata *pAD = *it_ta;
                const ulong *p1 = pGP->getGenome(pAD->iID);
                if (p1 != NULL) {
                    // remove 'special' characters from location name
                    size_t pos =  it_ltd->first.find_first_of("[ ]");
                    
                    GeneUtils::writePlinkHeader(fOut, it_ltd->first.substr(0, pos).c_str(), iC, pAD->iID, pAD->iDadID, pAD->iMomID, 2-pAD->iGender);
                    /*
                    // we have F=0, M=1, plink has M=1, F=2; plink_gender = 2 - QHG_Gender
                    fprintf(fOut, "%s_%03d % 10ld % 10ld % 10ld %d 1 ", it->first.substr(0, pos).c_str(), i, iID, mAgentData[iID]->iDadID, mAgentData[iID]->iMomID, 2-mAgentData[iID]->iGender);
                    */
                    
                    GeneUtils::writePlinkNucleotides(fOut, p1, iGenomeSize);
                    iC++;
                } else {
                    printf("%sno genome - Bad ID? [%ld]\n", RED, pAD->iID);
                    iResult = -1;
                }
                
            }
        }
              
        fclose(fOut);

        // a dummy mapfile consisting of lines
        // 1 abc-<NNNN> 0 <NNNN>
        
        char sOutMap[512];
        strcpy(sOutMap, pOutputFile);
        char *pDot = strrchr(sOutMap, '.');
        if (pDot != NULL) {
            *pDot = '\0';
        }
        strcat(sOutMap, ".map");
        fOut = fopen(sOutMap, "wt");
        if (fOut != NULL) {
            for (int i = 0; i < iGenomeSize; i++) {
                fprintf(fOut, "1 abc-%04d 0 %04d\n", i, i);
            }
            fclose(fOut);
        } else {
            printf("%sCouldn't open [%s] for writing\n", RED, sOutMap);
            iResult = -1;
        }
    } else {
        printf("%sCouldn't open [%s] for writing\n", RED, pOutputFile);
        iResult = -1;
    }
    
    return iResult;
}





//----------------------------------------------------------------------------
// writeGenesBin
//   write genome data to binary file.
//   NOTE: the two chromosomes are written on after the other!
//   Format:
//     File       ::= <Header><LocData>*
//     Header     ::= <Magic><GenomeSize><NumGenomes><NumLocs>
//     Magic      ::= "GENB"
//     Mode       ::= "G" | "N"
//     LocData    ::= <LocNameLen><LocName><LocLon><LocLat><LocDist><NumLocGenomes>
//     GenomeData ::= <GenomeInfo><BinGenome>)
//     GenomeInfo ::= <ID><MomID><DadID><Gender><CellID><GeneLon><GeneLat>
//     BinGenome  ::= <ulong>*
//    GenomeSize    : int
//    NumGenomes    : int
//    NumLocs       : int
//    LocNameLen    : int  
//    LocName       : char[]    // name of sampling location
//    LocLon        : double    // longitude  of sampling location
//    LocLat        : double    // longitude  of sampling location  
//    LocDist       : double    // sampling radius for sampling location  
//    NumLocGenomes : int       // number of genomes for location
//    ID            : int
//    MomID         : int
//    DadID         : int
//    CellID        : int
//    Gender        : int
//    GeneLon       : double    // longitude of genome
//    GeneLat       : double    // latitude of genome
//
int GeneWriter::writeGenesBin(GenomeProvider *pGP, const char *pOutputFile, const locdata &mLocDefs,  const IDSample *pSample) {
    int iResult = 0;


    FILE *fOut = fopen(pOutputFile, "wb");
    if (fOut != NULL) {
        size_t iGenomeSize = pGP->getGenomeSize();
        int iNumBlocks = GeneUtils::numNucs2Blocks((int)iGenomeSize);
        size_t iLen = 5*sizeof(int) + 2*sizeof(double) + 2*iNumBlocks*sizeof(long);
        
        locids mSelected;
        pSample->getLocationIDSet(mSelected);
        idagd mIDAD;
        pSample->getIDADMap(mIDAD);

        size_t iNumGenes = 0;
        locids::const_iterator it0;
        for (it0 = mSelected.begin(); it0 != mSelected.end(); ++it0) {
            iNumGenes += it0->second.size();
        }
        size_t iNumLocs = mSelected.size();
        
        char *pLine = new char[iLen];
        // write file header
        char *p0 = pLine;
        p0 = putMem(p0, "GENS", 4*sizeof(char));
        p0 = putMem(p0, &iGenomeSize, sizeof(int));
        p0 = putMem(p0, &iNumGenes, sizeof(int));
        p0 = putMem(p0, &iNumLocs, sizeof(int));
        size_t iWritten = fwrite(pLine, 4*sizeof(char)+3*sizeof(int), 1, fOut);
        if (iWritten == 1) {
        
            char *pSpecial = NULL;
            size_t iSpecial = 0;
            
            printf("writing %zd locations\n",  mSelected.size());
            locids::const_iterator it;
            for (it = mSelected.begin(); (iResult == 0) && (it != mSelected.end()); ++it) {
                const locitem &li = mLocDefs.at(it->first);
                double dLon = li.dLon*180/M_PI;
                double dLat = li.dLat*180/M_PI;
                // write the location header
                size_t iNumGenomes = it->second.size();
                const char *pName = it->first.c_str();
                size_t iNameLen = 1+strlen(pName);
                size_t iFullLength = iNameLen+2*sizeof(int)+3*sizeof(double);
                if (iSpecial < iFullLength) {
                    if (pSpecial != NULL) {
                        delete[] pSpecial;
                    }
                    iSpecial = iFullLength;
                    pSpecial = new char[iSpecial];
                }
                char *p = pSpecial;
                p = putMem(p, &(iNameLen), sizeof(int));
                p = putMem(p, pName, iNameLen);
                p = putMem(p, &(dLon), sizeof(double));
                p = putMem(p, &(dLat), sizeof(double));
                p = putMem(p, &(li.dDist), sizeof(double));
                p = putMem(p, &(iNumGenomes), sizeof(int));
                iWritten = fwrite(pSpecial, iFullLength, 1, fOut);
                if (iWritten == 1) {
                
                    // id are sorted because in set
                    idset v2 = it->second;
                    printf("  [%s]: %zd genomes\n",  pName, v2.size());
                    
                    idset::const_iterator its;
                    for (its=it->second.begin(); its != it->second.end(); ++its) {
                        char *p = pLine;
                        idtype iID = *its;
                        const ulong *pGenome = pGP->getGenome(iID);
                        if (pGenome != NULL) {
                            agdata *pAD = mIDAD[iID];
                            // write agent data
                            double dLon = pAD->dLon*180/M_PI;
                            double dLat = pAD->dLat*180/M_PI;
                            p = putMem(p, &iID, sizeof(int));
                            p = putMem(p, &(pAD->iMomID), sizeof(int));
                            p = putMem(p, &(pAD->iDadID), sizeof(int));
                            p = putMem(p, &(pAD->iGender), sizeof(int));
                            p = putMem(p, &(pAD->iCellID), sizeof(int));
                            p = putMem(p, &dLon, sizeof(double));
                            p = putMem(p, &dLat, sizeof(double));
                            
                            // write the genome
                            p = putMem(p, pGenome, 2*iNumBlocks*sizeof(long));
                            
                            iWritten = fwrite(pLine, iLen, 1, fOut);
                            if (iWritten != 1) {
                                printf("%sCouldn't write genome data to [%s]\n", RED, pOutputFile);
                                iResult = -1;
                            }
                        } else {
                            printf("%sBad ID (no genome): %ld\n", RED, iID);
                            iResult = -1;
                        }
                    }
                
                } else {
                    printf("%sCouldn't write genome header to [%s]\n", RED, pOutputFile);
                    iResult = -1;
                }
            }
            delete[] pSpecial;
        } else {
            printf("%sCouldn't write file header to [%s]\n", RED, pOutputFile);
            iResult = -1;
        }
        
        fclose(fOut);
        delete[] pLine;

    } else {
        printf("%sCouldn't open [%s] for writing\n", RED, pOutputFile);
        iResult = -1;
    }


    return iResult;
}


//----------------------------------------------------------------------------
// agdatacomp
//   compare function for sorting a ontainer of agdata*
//
bool agdatacomp(agdata *p1, agdata *p2) {
    return p1->iID < p2->iID;
}

//----------------------------------------------------------------------------
// writeGenesAsc
//   write genome data to ascii file
//   NOTE: the two chromosomes are written on after the other!
//   Format:
//    File          ::= <LocationBlock>*
//    LocationBlock ::= <LocHeader><LocData>
//    LocHeader     ::= "# GROUP" <LocName> "("<LocLon>","<LocLat>") d" <LocDist> "T "<Time>
//    LocData       ::= <GeneHeader><GeneData>
//    GeneHeader    ::= <ID> <MoomID> <DadID> <Gender> <CellID> <GeneLon> <GeneLat>
//    GeneData      ::= ( "A" | "C" | "G" | "T" )*
//    LocName       : char[]    // name of sampling location
//    LocLon        : double    // longitude  of sampling location
//    LocLat        : double    // longitude  of sampling location  
//    LocDist       : double    // sampling radius for sampling location  
//    ID            : int
//    MomID         : int
//    DadID         : int
//    CellID        : int
//    Gender        : int
//    GeneLon       : double    // longitude of genome
//    GeneLat       : double    // latitude of genome
//
int GeneWriter::writeGenesAsc(GenomeProvider *pGP, const char *pOutputFile,  const locdata &mLocDefs,   const IDSample *pSample) {
    int iResult = -1;

    FILE *fOut = fopen(pOutputFile, "wt");
    if (fOut != NULL) {
        iResult = 0;
        int iGenomeSize = pGP->getGenomeSize();
        int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
        
        locagd msLocAD;
        pSample->getLocationADSet(msLocAD);
        sampleinfo::const_iterator it_ltd;
        const sampleinfo &mmvAgentData = pSample->getSampleInfo();
        for (it_ltd = mmvAgentData.begin(); it_ltd != mmvAgentData.end(); ++it_ltd) {
            timeagdata::const_iterator it_td;
            for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
                    
                const locitem &li = mLocDefs.at(it_ltd->first);
                // location header
                fprintf(fOut, "# GROUP %s (%f,%f) d %f T %f\n", it_ltd->first.c_str(), li.dLon*180/M_PI, li.dLat*180/M_PI, li.dDist, it_td->first);
                
                // sort the IDs
                std::vector<agdata*> v2 = it_td->second;
                std::sort(v2.begin(),v2.end(), agdatacomp);
                
                for (uint i = 0; i < v2.size(); ++i) {
                    agdata *pAD = v2[i];
                    idtype iID = pAD->iID;
                    const ulong *p = pGP->getGenome(iID);
                    if (p != NULL) {
                                // print agent data ...
                        fprintf(fOut, "%9ld %9ld %9ld %9d %9d % 9.4f % 8.4f  ", 
                                iID, 
                                pAD->iMomID, pAD->iDadID, pAD->iGender,
                                pAD->iCellID, pAD->dLon*180/M_PI, pAD->dLat*180/M_PI);
                        
                        // ... and agent genome
                        for (int iB = 0; iB < 2*iNumBlocks; iB++) {
                            char s[GeneUtils::NUCSINBLOCK+1];
                            GeneUtils::blockToNucStr(*p, s);
                            if (iB == iNumBlocks) {
                                fprintf(fOut, " ");
                            }
                            fprintf(fOut, "%s", s);
                            p++;
                        }
                        fprintf(fOut, "\n");
                    } else {
                        printf("%sBad ID (no genome): %ld\n", RED, v2[i]->iID);
                        iResult = -1;
                    }
                }
            }
        }                        
        fclose(fOut);
    } else {
        printf("%sCouldn't open [%s] for writing\n", RED, pOutputFile);
        iResult = -1;
    }

    return iResult;
}
