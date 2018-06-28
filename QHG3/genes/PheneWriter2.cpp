#include <stdio.h>
#include <string.h>
#include <math.h>

#include <algorithm>

#include "types.h"
#include "strutils.h"
#include "SequenceProvider.h"
#include "PheneWriter2.h"


bool s_bVerbose = false;

//----------------------------------------------------------------------------
// format accepted
//
bool PheneWriter2::formatAccepted(const char *pFormat) {
    bool bAccepted = false;
    if (strcasecmp(pFormat, FORMAT_BIN) == 0) {
        bAccepted = true;
    } else if (strcasecmp(pFormat, FORMAT_ASC) == 0) {
        bAccepted = true;
    } else {
        bAccepted = false;
    }
    return bAccepted;
}


//----------------------------------------------------------------------------
// writeSequence
//  write ID, mom id, dad id, gender and phenome
//  with sub headers for different locations
//
int PheneWriter2::writeSequence(const char *pFormat, SequenceProvider<float> *pGP, const char *pOutputFile,  const loc_data &mLocData,  const IDSample *pSample, bool bFull) {
    int iResult = 0;

    if (pOutputFile != NULL) {

        
        if (strcasecmp(pFormat, FORMAT_BIN) == 0) {
            if (s_bVerbose) printf("Writing binary output (genome size %d) to [%s]\n", 
                   pGP->getSequenceSize(), pOutputFile);
            iResult = writePhenesBin(pGP, pOutputFile, mLocData, pSample, bFull);

        } else if (strcasecmp(pFormat, FORMAT_ASC) == 0) {
            if (s_bVerbose) printf("Writing ascii output (genome size %d) to [%s]\n", 
                   pGP->getSequenceSize(), pOutputFile);
            iResult = writePhenesAsc(pGP, pOutputFile, mLocData, pSample, bFull);
            
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
// writePhenesBin
//   write genome data to binary file.
//   NOTE: the two chromosomes are written on after the other!
//   Format:
//     File       ::= <Header><LocPhenomes>*
//     Header     ::= <Magic><PhenomeSize><NumPhenomes><NumLocs><Full>
//     Magic      ::= "PHNY"
//     LocGenomes ::= <LocData><PhenomeData>*
//     LocData    ::= <LocNameLen><LocName><LocLon><LocLat><LocDist><Time><NumLocPhenomes>
//     PhenomeData ::= <PhenomeInfo><BinPhenome>
//     PhenomeInfo ::= <ID>[<MomID><DadID><Gender>]<CellID><PheneLon><PheneLat>
//     BinPhenome  ::= <Block>*
//    PhenomeSize    : int
//    NumPhenomes    : int
//    NumLocs        : int
//    Full           : bool
//    LocNameLen     : int  
//    LocName        : char[]    // name of sampling location
//    LocLon         : double    // longitude  of sampling location
//    LocLat         : double    // longitude  of sampling location  
//    LocDist        : double    // sampling radius for sampling location  
//    Time           : double    // sampling time
//    NumLocPhenomes : int       // number of genomes for location
//    ID             : long
//    MomID          : long      // -1 if not specified
//    DadID          : long      // -1 if not specified
//    CellID         : int       //  0 if not specified
//    Gender         : int       //  0: female, 1: male
//    PheneLon       : double    // longitude of genome
//    PheneLat       : double    // latitude of genome
//    Block          : float      // consists of 1- or 2-bit nucleotides
//
int PheneWriter2::writePhenesBin(SequenceProvider<float> *pGP, const char *pOutputFile, const loc_data &mLocDefs,  const IDSample *pSample, bool bFull) {
    int iResult = 0;


    FILE *fOut = fopen(pOutputFile, "wb");
    if (fOut != NULL) {
        size_t iPhenomeSize = pGP->getSequenceSize();
        int iNumBlocks = iPhenomeSize;
	
        size_t iLen = sizeof(idtype) + 
                      sizeof(int) + 
                      2*sizeof(double) + 
                      2*iNumBlocks*sizeof(float);
        if (bFull) {
            iLen += 2*sizeof(idtype) + sizeof(int);
        }

        tloc_ids mSelected;
        pSample->getTimeLocationIDSet(mSelected);
        id_agd mIDAD;
        pSample->getIDADMap(mIDAD);

        size_t iNumPhenes = 0;
        tloc_ids::const_iterator it0;
        for (it0 = mSelected.begin(); it0 != mSelected.end(); ++it0) {
            iNumPhenes += it0->second.size();
        }
        size_t iNumLocs = mSelected.size();
        
        int iLenH =  4*sizeof(char)+3*sizeof(int)+sizeof(bool);
        char *pLineH = new char[iLenH];
        // write file header
        char *p0 = pLineH;
        p0 = putMem(p0, "PHNY", 4*sizeof(char));
        p0 = putMem(p0, &iPhenomeSize, sizeof(int));
        p0 = putMem(p0, &iNumPhenes, sizeof(int));
        p0 = putMem(p0, &iNumLocs, sizeof(int));
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
                size_t iNumPhenomes = it->second.size();
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
                p = putMem(p, &(iNumPhenomes), sizeof(int));
                iWritten = fwrite(pSpecial, iFullLength, 1, fOut);
                if (iWritten == 1) {
                
                    // id are sorted because in set
                    idset v2 = it->second;
                                
                    idset::const_iterator its;
                    for (its=it->second.begin(); its != it->second.end(); ++its) {

                        p = pLine;
                        idtype iID = *its;

                        const float *pPhenome = pGP->getSequence(iID);
                        if (pPhenome != NULL) {
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
                            p = putMem(p, pPhenome, 2*iNumBlocks*sizeof(float));
                            

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
        fprintf(stderr, "%sCouldn't open [%s] for writing\n", RED, pOutputFile);
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
// writePhenesAsc
//   write genome data to binary file.
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
//
int PheneWriter2::writePhenesAsc(SequenceProvider<float> *pGP, const char *pOutputFile, const loc_data &mLocDefs,  const IDSample *pSample, bool bFull) {
    int iResult = 0;

    FILE *fOut = fopen(pOutputFile, "wt");
    if (fOut != NULL) {
        // check format string for agent entry below
        int iGOffset = 12+1+2*(9+1)+8+2;
        if (bFull) {
            iGOffset += 3*(9+1);
        }
        fprintf(fOut, "# GENES %s G-OFFSET %d\n", bFull?"full":"red",  iGOffset);

        iResult = 0;
        int iPhenomeSize = pGP->getSequenceSize();

        int iNumBlocks = iPhenomeSize;
        
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
                    const float *p = pGP->getSequence(iID);
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
                            if (iB == iNumBlocks) {
                                fprintf(fOut, " ");
                            }
                            fprintf(fOut, "%f ", *p);
                            p++;
                        }
                        fprintf(fOut, "\n");
                    } else {
                        fprintf(stderr, "%sBad ID (no phenome): %ld\n", RED, v2[i]->iID);
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
