#include <stdio.h>
#include <string.h>
#include <math.h>

#include <algorithm>

#include "types.h"
#include "strutils.h"
#include "colors.h"
#include "GeneUtils.h"
#include "BitGeneUtils.h"
#include "SequenceProvider.h"
#include "GeneWriter2.h"

bool s_bVerbose = false;
//----------------------------------------------------------------------------
// format accepted
//
bool GeneWriter2::formatAccepted(const char *pFormat) {
    bool bAccepted = false;
    if (strcasecmp(pFormat, FORMAT_PLINK) == 0) {
        bAccepted = true;
    } else if (strcasecmp(pFormat, FORMAT_BIN) == 0) {
        bAccepted = true;
    } else if (strcasecmp(pFormat, FORMAT_ASC) == 0) {
        bAccepted = true;
    } else if (strcasecmp(pFormat, FORMAT_NUM) == 0) {
        bAccepted = true;
    } else {
        bAccepted = false;
    }
    return bAccepted;
}


//----------------------------------------------------------------------------
// writeSequence
//  write ID, mom id, dad id, gender and genome
//  with sub headers for different locations
//
int GeneWriter2::writeSequence(const char *pFormat, SequenceProvider<ulong> *pGP, const char *pOutputFile,  const loc_data &mLocData,  const IDSample *pSample, bool bFull, bool bBitNucs) {
    int iResult = 0;

    if (pOutputFile != NULL) {

        
        if (strcasecmp(pFormat, FORMAT_PLINK) == 0) {
            if (s_bVerbose) printf("Writing plink output (genome size %d) to [%s]\n", 
                   pGP->getSequenceSize(), pOutputFile);
            iResult = writeGenesPlink(pGP, pOutputFile, mLocData, pSample);

        } else if (strcasecmp(pFormat, FORMAT_BIN) == 0) {
            if (s_bVerbose) printf("Writing binary output (genome size %d) to [%s]\n", 
                   pGP->getSequenceSize(), pOutputFile);
            iResult = writeGenesBin(pGP, pOutputFile, mLocData, pSample, bFull, bBitNucs);

        } else if (strcasecmp(pFormat, FORMAT_ASC) == 0) {
            if (s_bVerbose) printf("Writing ascii output (genome size %d) to [%s]\n", 
                   pGP->getSequenceSize(), pOutputFile);
            iResult = writeGenesAsc(pGP, pOutputFile, mLocData, pSample, bFull, bBitNucs);
            
        } else if (strcasecmp(pFormat, FORMAT_NUM) == 0) {
            if (s_bVerbose) printf("Writing num output (genome size %d) to [%s]\n", 
                   pGP->getSequenceSize(), pOutputFile);
            iResult = writeGenesNum(pGP, pOutputFile, pSample, bFull, bBitNucs); // here "Full" means "add ID"
            
        } else {
            fprintf(stderr, "%sUnknown format [%s]\n", RED, pFormat);
            iResult = -1;
        }
    } else {
        fprintf(stderr, "%sNo file name given\n", RED);
        iResult = -1;
    }
    return iResult;
}



//----------------------------------------------------------------------------
// writeGenesPlink
//  write a file in the plink ".ped" format and a dummy map file
//
int GeneWriter2::writeGenesPlink(SequenceProvider<ulong> *pGP, const char *pOutputFile,  const loc_data &mLocDefs,  const IDSample *pSample) {
    int iResult = -1;
    
    FILE *fOut = fopen(pOutputFile, "wt");
    if (fOut != NULL) {
        iResult = 0;
        
        int iGenomeSize = pGP->getSequenceSize();
        //int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
        loc_agd msLocAD;
        pSample->getLocationADSet(msLocAD);
        loc_agd::const_iterator it_ltd;
        for (it_ltd = msLocAD.begin(); (iResult == 0) && (it_ltd != msLocAD.end()); ++it_ltd) {
            int iC = 0;
            std::set<agdata *>::const_iterator it_ta;
            for (it_ta = it_ltd->second.begin(); it_ta != it_ltd->second.end(); ++it_ta) {
                const agdata *pAD = *it_ta;
                const ulong *p1 = pGP->getSequence(pAD->iID);
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
                    fprintf(stderr, "%sno genome - Bad ID? [%ld]\n", RED, pAD->iID);
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
            fprintf(stderr, "%sCouldn't open [%s] for writing\n", RED, sOutMap);
            iResult = -1;
        }
    } else {
        fprintf(stderr, "%sCouldn't open [%s] for writing\n", RED, pOutputFile);
        iResult = -1;
    }
    
    return iResult;
}





//----------------------------------------------------------------------------
// writeGenesBin
//   write genome data to binary file.
//   NOTE: the two chromosomes are written on after the other!
//   Format:
//     File       ::= <Header><LocGenomes>*
//     Header     ::= <Magic><GenomeSize><NumGenomes><NumLocs><BitsPerNuc><Full>
//     Magic      ::= "GENY"
//     LocGenomes ::= <LocData><GenomeData>*
//     LocData    ::= <LocNameLen><LocName><LocLon><LocLat><LocDist><Time><NumLocGenomes>
//     GenomeData ::= <GenomeInfo><BinGenome>
//     GenomeInfo ::= <ID>[<MomID><DadID><Gender>]<CellID><GeneLon><GeneLat>
//     BinGenome  ::= <Block>*
//    GenomeSize    : int
//    NumGenomes    : int
//    NumLocs       : int
//    BitsPerNuc    : int
//    Full          : bool
//    LocNameLen    : int  
//    LocName       : char[]    // name of sampling location
//    LocLon        : double    // longitude  of sampling location
//    LocLat        : double    // longitude  of sampling location  
//    LocDist       : double    // sampling radius for sampling location  
//    Time          : double    // sampling time
//    NumLocGenomes : int       // number of genomes for location
//    ID            : long
//    MomID         : long      // -1 if not specified
//    DadID         : long      // -1 if not specified
//    CellID        : int       //  0 if not specified
//    Gender        : int       //  0: female, 1: male
//    GeneLon       : double    // longitude of genome
//    GeneLat       : double    // latitude of genome
//    Block         : ulng      // consists of 1- or 2-bit nucleotides
//
int GeneWriter2::writeGenesBin(SequenceProvider<ulong> *pGP, const char *pOutputFile, const loc_data &mLocDefs,  const IDSample *pSample, bool bFull, bool bBitNucs) {
    int iResult = 0;


    FILE *fOut = fopen(pOutputFile, "wb");
    if (fOut != NULL) {
        size_t iGenomeSize = pGP->getSequenceSize();
        int iBitsPerNuc = 0;
        int iNumBlocks = 0;
	if (bBitNucs) {
            iNumBlocks = BitGeneUtils::numNucs2Blocks((int)iGenomeSize);
            iBitsPerNuc = BitGeneUtils::BITSINNUC;
	} else {
            iNumBlocks = GeneUtils::numNucs2Blocks((int)iGenomeSize);
            iBitsPerNuc = GeneUtils::BITSINNUC;
        }
	
        size_t iLen = sizeof(idtype) + 
                      sizeof(int) + 
                      2*sizeof(double) + 
                      2*iNumBlocks*sizeof(long);
        if (bFull) {
            iLen += 2*sizeof(idtype) + sizeof(int);
        }

        tloc_ids mSelected;
        pSample->getTimeLocationIDSet(mSelected);
        id_agd mIDAD;
        pSample->getIDADMap(mIDAD);

        size_t iNumGenes = 0;
        tloc_ids::const_iterator it0;
        for (it0 = mSelected.begin(); it0 != mSelected.end(); ++it0) {
            iNumGenes += it0->second.size();
        }
        size_t iNumLocs = mSelected.size();
        
        int iLenH =  4*sizeof(char)+4*sizeof(int)+sizeof(bool);
        char *pLineH = new char[iLenH];
        // write file header
        char *p0 = pLineH;
        p0 = putMem(p0, "GENY", 4*sizeof(char));
        p0 = putMem(p0, &iGenomeSize, sizeof(int));
        p0 = putMem(p0, &iNumGenes, sizeof(int));
        p0 = putMem(p0, &iNumLocs, sizeof(int));
        p0 = putMem(p0, &iBitsPerNuc, sizeof(int));
        p0 = putMem(p0, &bFull, sizeof(bool));
        size_t iWritten = fwrite(pLineH, iLenH, 1, fOut);
        delete[] pLineH;
        if (iWritten == 1) {

            char *pLine = new char[iLen];
        
            char *pSpecial = NULL;
            size_t iSpecial = 0;
            
            tloc_ids::const_iterator it;
            for (it = mSelected.begin(); (iResult == 0) && (it != mSelected.end()); ++it) {
                const locitem &li = mLocDefs.at(it->first.first);
                double dLon = li.dLon;//*180/M_PI;
                double dLat = li.dLat;//*180/M_PI;
                // write the location header
                size_t iNumGenomes = it->second.size();
                const char *pName = it->first.first.c_str();
                size_t iNameLen = 1+strlen(pName);
                size_t iFullLength = iNameLen+2*sizeof(int)+4*sizeof(double);
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
                p = putMem(p, &(it->first.second), sizeof(double));
                p = putMem(p, &(iNumGenomes), sizeof(int));
                iWritten = fwrite(pSpecial, iFullLength, 1, fOut);
                if (iWritten == 1) {
                
                    // id are sorted because in set
                    idset v2 = it->second;
                                
                    idset::const_iterator its;
                    for (its=it->second.begin(); its != it->second.end(); ++its) {
                        p = pLine;
                        idtype iID = *its;
                        const ulong *pGenome = pGP->getSequence(iID);
                        if (pGenome != NULL) {
                            agdata *pAD = mIDAD[iID];
                            // write agent data
                            double dLonS = pAD->dLon;
                            double dLatS = pAD->dLat;

                            p = putMem(p, &iID, sizeof(idtype));
                            if (bFull) {
                                p = putMem(p, &(pAD->iMomID), sizeof(idtype));
                                p = putMem(p, &(pAD->iDadID), sizeof(idtype));
                                p = putMem(p, &(pAD->iGender), sizeof(int));
                            }
                            p = putMem(p, &(pAD->iCellID), sizeof(int));
                            p = putMem(p, &dLonS, sizeof(double));
                            p = putMem(p, &dLatS, sizeof(double));
                            
                            // write the genome
                            p = putMem(p, pGenome, 2*iNumBlocks*sizeof(long));
                            
                            iWritten = fwrite(pLine, iLen, 1, fOut);
                            if (iWritten != 1) {
                                fprintf(stderr, "%sCouldn't write genome data to [%s]\n", RED, pOutputFile);
                                iResult = -1;
                            }
                        } else {
                            fprintf(stderr, "%sBad ID (no genome): %ld\n", RED, iID);
                            iResult = -1;
                        }
                    }
                
                } else {
                    fprintf(stderr, "%sCouldn't write genome header to [%s]\n", RED, pOutputFile);
                    iResult = -1;
                }
            }
            delete[] pSpecial;
            delete[] pLine;
        } else {
            fprintf(stderr, "%sCouldn't write file header to [%s]\n", RED, pOutputFile);
            iResult = -1;
        }
        
        fclose(fOut);

    } else {
        fprintf(stderr,"%sCouldn't open [%s] for writing\n", RED, pOutputFile);
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
//    File          ::= <Info><LocationBlock>*
//    Info          ::= "# GENES " ("FULL" | "RED")
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
int GeneWriter2::writeGenesAsc(SequenceProvider<ulong> *pGP, const char *pOutputFile,  const loc_data &mLocDefs,   const IDSample *pSample, bool bFull, bool bBitNucs) {
    int iResult = -1;

    FILE *fOut = fopen(pOutputFile, "wt");
    if (fOut != NULL) {
        // check format string for agent entry below
        int iGOffset = 12+1+2*(9+1)+8+2;
        if (bFull) {
            iGOffset += 3*(9+1);
        }
        fprintf(fOut, "# GENES %s G-OFFSET %d\n", bFull?"full":"red",  iGOffset);

        iResult = 0;
        int iGenomeSize = pGP->getSequenceSize();

        char *(*blockToNucStr)(ulong lBlock, char *pNuc);
        int iNucsInBlock = 0;
        int iNumBlocks = 0;
	if (bBitNucs) {
            iNumBlocks    =  BitGeneUtils::numNucs2Blocks((int)iGenomeSize);
            iNucsInBlock  =  BitGeneUtils::NUCSINBLOCK;
            blockToNucStr = &BitGeneUtils::blockToNucStr;
	} else {
            iNumBlocks    =  GeneUtils::numNucs2Blocks((int)iGenomeSize);
            iNucsInBlock  =  GeneUtils::NUCSINBLOCK;
            blockToNucStr = &GeneUtils::blockToNucStr;
        }
        
        loc_agd msLocAD;
        pSample->getLocationADSet(msLocAD);
        sampleinfo::const_iterator it_ltd;
        const sampleinfo &mmvAgentData = pSample->getSampleInfo();
        for (it_ltd = mmvAgentData.begin(); it_ltd != mmvAgentData.end(); ++it_ltd) {
            time_vagdata::const_iterator it_td;
            for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
                    
                const locitem &li = mLocDefs.at(it_ltd->first);
                // location header
                // fprintf(fOut, "# GROUP %s (%f,%f) d %f T %f\n", it_ltd->first.c_str(), li.dLon*180/M_PI, li.dLat*180/M_PI, li.dDist, it_td->first);
		// location files have coordinates in degrees
                fprintf(fOut, "# GROUP %s (%f,%f) d %f T %f\n", it_ltd->first.c_str(), li.dLon, li.dLat, li.dDist, it_td->first);
                
                // sort the IDs
                std::vector<agdata*> v2 = it_td->second;
                std::sort(v2.begin(),v2.end(), agdatacomp);
                
                for (uint i = 0; i < v2.size(); ++i) {
                    agdata *pAD = v2[i];
                    idtype iID = pAD->iID;
                    const ulong *p = pGP->getSequence(iID);
                    if (p != NULL) {
                        // print agent data ...
                        if (bFull) {
                            // changes here must be reflected in the G-Offset-value
                            fprintf(fOut, "%12ld %9ld %9ld %9d %9d % 9.4f % 8.4f  ", 
                                    iID, 
                                    pAD->iMomID, pAD->iDadID, pAD->iGender,
                                    pAD->iCellID, pAD->dLon/* *180/M_PI*/, pAD->dLat/* *180/M_PI*/);
                        } else {
                            // changes here must be reflected in the G-Offset-value
                            fprintf(fOut, "%12ld %9d % 9.4f % 8.4f  ", 
                                    iID, 
                                    pAD->iCellID, pAD->dLon/**180/M_PI*/, pAD->dLat/* *180/M_PI*/);
                        }

                        // ... and agent genome
                        for (int iB = 0; iB < 2*iNumBlocks; iB++) {
                            char s[iNucsInBlock+1];
                            (*blockToNucStr)(*p, s);
                            if (iB == iNumBlocks) {
                                fprintf(fOut, " ");
                            }
                            fprintf(fOut, "%s", s);
                            p++;
                        }
                        fprintf(fOut, "\n");
                    } else {
                        fprintf(stderr,"%sBad ID (no genome): %ld\n", RED, v2[i]->iID);
                        iResult = -1;
                    }
                }
            }
        }                        
        fclose(fOut);
    } else {
        fprintf(stderr, "%sCouldn't open [%s] for writing\n", RED, pOutputFile);
        iResult = -1;
    }

    return iResult;
}

//----------------------------------------------------------------------------
// writeGenesNum
//   write genome data to ascii file
//   NOTE: the two chromosomes are written on after the other!
//   Format:
//    File          ::= <genes>*
//    genes         ::= <chr1> <chr2> <NL>
//    chr1          ::= ( "0" | "1" | "2" | "3" )*
//    chr2          ::= ( "0" | "1" | "2" | "3" )*
//
//    0=A, 1=C, 2=G, 3=T
//
int GeneWriter2::writeGenesNum(SequenceProvider<ulong> *pGP, const char *pOutputFile,   const IDSample *pSample, bool bAddID, bool bBitNucs) {
    int iResult = -1;

    FILE *fOut = fopen(pOutputFile, "wt");
    if (fOut != NULL) {
        iResult = 0;
        int iGenomeSize = pGP->getSequenceSize();
        //int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
        int iNumBlocks = 0;
        int iNucsInBlock = 0;
        char *(*blockToNumStr)(ulong lBlock, char *pNuc);
	if (bBitNucs) {
            iNumBlocks    =  BitGeneUtils::numNucs2Blocks((int)iGenomeSize);
            iNucsInBlock  =  BitGeneUtils::NUCSINBLOCK;
            blockToNumStr = &BitGeneUtils::blockToNumStr;
	} else {
            iNumBlocks    =  GeneUtils::numNucs2Blocks((int)iGenomeSize);
            iNucsInBlock  =  GeneUtils::NUCSINBLOCK;
            blockToNumStr = &GeneUtils::blockToNumStr;
        }
        
        loc_agd msLocAD;
        pSample->getLocationADSet(msLocAD);
        sampleinfo::const_iterator it_ltd;
        const sampleinfo &mmvAgentData = pSample->getSampleInfo();
        for (it_ltd = mmvAgentData.begin(); it_ltd != mmvAgentData.end(); ++it_ltd) {
            time_vagdata::const_iterator it_td;
            for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
                    
                // sort the IDs
                std::vector<agdata*> v2 = it_td->second;
                std::sort(v2.begin(),v2.end(), agdatacomp);
                
                for (uint i = 0; i < v2.size(); ++i) {
                    agdata *pAD = v2[i];
                    idtype iID = pAD->iID;
                    const ulong *p = pGP->getSequence(iID);
                    if (p != NULL) {
                        if(bAddID) {
                            fprintf(fOut, "%12ld ", iID);
                        }
                        int iNucCount = iGenomeSize;
                        // ... and agent genome
                        for (int iB = 0; iB < 2*iNumBlocks; iB++) {
                            char s[2*iNucsInBlock+1];
                            (*blockToNumStr)(*p, s);
                            iNucCount -= iNucsInBlock;
                            if ((iNucCount <  iNucsInBlock) && (iNucCount > 0)) {
                                s[2*iNucCount] = '\0';
                            }  
                            if (iB == iNumBlocks) {
                                fprintf(fOut, " ");
                                iNucCount = iGenomeSize;
                            }
                            fprintf(fOut, "%s", s);
                            p++;
                        }
                        fprintf(fOut, "\n");
                    } else {
                        fprintf(stderr, "%sBad ID (no genome): %ld\n", RED, v2[i]->iID);
                        iResult = -1;
                    }
                }
            }
        }                        
        fclose(fOut);
    } else {
        fprintf(stderr, "%sCouldn't open [%s] for writing\n", RED, pOutputFile);
        iResult = -1;
    }

    return iResult;
}

