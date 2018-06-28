#include <stdio.h>

#include <iostream>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkDataArray.h>

#include "types.h"
#include "strutils.h"
#include "ParamReader.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "QDFUtils.h"
#include "GeneUtils.h"
#include "IDSample.h"
#include "IDSampler2.h"
#include "QDFGenomeExtractor2.h"
#include "GenomeSegments.h"

/* complete compile and link
g++ SegmentCompare2.cpp QDFGenomeExtractor2.cpp QDFSequenceExtractor.cpp \
    IDSampler2.cpp IDSample.cpp GeneUtils.cpp BitGeneUtils.cpp GenomeSegments.cpp \
    -I ../common                                                         \
    -I /opt/visit-2.12.3/2.12.3/linux-x86_64/include/vtk/vtk-6.1/        \
    -I ../io -L ../io -lIO                                               \
    -L../common -lCommon                                                 \
    -lhdf5 -lcrypto -lz -fopenmp                                         \
    -L/opt/visit-2.12.3/2.12.3/linux-x86_64/lib/                         \
    -lvtkCommonCore -lvtkCommonDataModel  -lvtkIOCore -lvtkIOLegacy
*/

#define PLINK_ORDER true

#define DEF_PHRASE "klops fideli987"

#define ATTR_GENETICS_GENOME_SIZE       "Genetics_genome_size"
#define ATTR_GENETICS_BITS_PER_NUC      "Genetics_bits_per_nuc"

#define GENOME_DATASET_NAME    "Genome"
 
static const uint  BUFSIZE_READ_ADDITIONAL=10000;
//----------------------------------------------------------------------------
// usage
void usage(const char *pApp) {
    printf("%s - comparing genomes by segments\n", pApp);
    printf("usage:\n");
    printf("  %s - pop-file=<popQDF> --grid-file=<gridQDF>\n", pApp);
    printf("    [--pop-name=<popName>] --location-file=<locFile>\n");
    printf("    --segment-def=<segDef> --ref-genome=<refGenome>:<gformat>[:offset]\n");
    printf("    --output-file=<outputName>\n");
    printf("where\n");
    printf("  popQDF      QDF Population file with Genetics group\n");
    printf("  popName     name of population to select\n");
    printf("              (if no name is given,the first population in popQDF is used)\n"); 
    printf("  gridQDF     QDF grid file corresponding to popQDF\n");
    printf("  locFile     location file\n");
    printf("  segDef      segment definition\n");
    printf("  refGenome   1-line file containing reference genome\n");
    printf("  gformat     format of reference genome (\"plink\" or \"lin\")\n");
    printf("  offset      number of characters to skip in reference genome file\n");
    printf("\n");
    printf("Formats:\n");

    printf("  LocationFile contents:\n");
    printf("    LocationFile ::= <LocationLine>*\n");     
    printf("    LocationLine ::= <identifier> <lon> <lat> <dist> <num> NL\n");     
    printf("    identifier   : string (name of location)\n");
    printf("    longitude    : double (longitude in degrees)\n");
    printf("    latitude     : double (latitude in degrees)\n");
    printf("    dist         : double (sample radius in km)\n");
    printf("    num          : int    (number of elements to sample)\n");
    printf("\n");
    
    printf("  segment definition\n");
    printf("    SegDef    ::= <NumSegs> | <DefList>\n");
    printf("    NumSegs   : int (number of equally sized segments)\n");
    printf("    DefList  ::=  <StartNuc>\":\"<LengthNuc> (\",\" <StartNuc>\":\"<LengthNuc>)*\n");
    printf("    StartNuc  : int (nucleotide number of segment start)\n");
    printf("    LengthNuc : int (length of segment in nucleotides)\n");
    printf("\n");
    
    printf("  reference genome file\n");
    printf("    a sequence of the letters 'A', 'C', 'G', 'T'; all other characters are ignored\n");
    printf("   format \"plink\": mother and father contributions alternate\n");
    printf("   format \"lin\": mother and father contributions follow each other\n");
}


//----------------------------------------------------------------------------
// readReferenceGenome
//
ulong *readReferenceGenome(char *sReferenceGenome, int iGenomeSize) {
    ulong *pRGenome = NULL;

    char *pOrdering = strchr(sReferenceGenome, ':');
    int iOffset = 0;
    bool bOrdering = false;

    int iResult = -1;
    if (pOrdering != NULL) {
        *pOrdering = '\0';
        pOrdering++;
        iResult = 0;
        
        if (strcmp(pOrdering, "plink") == 0) {
            bOrdering = true;
        }

        char *pOffset = strchr(pOrdering, ':');
        if (pOffset != NULL) {
            *pOffset = '\0';
            pOffset++;
            if (strToNum(pOffset, &iOffset)) {
                iResult = 0;
            } else {
                printf("offset [%s] should be a positive integer\n", pOffset);
                iResult = -1;
            }
        }
    } else {
        printf("expected ':' + gformat (\"plink\" or \"lin\")\n");
        iResult =-1;
    }

    if (iResult == 0) {
        std::vector<uchar> vNucs;
        FILE *fIn = fopen(sReferenceGenome, "rt");
        if (fIn != NULL) {
            fseek(fIn, iOffset, SEEK_SET);
            while (!feof(fIn)) {
                char c = fgetc(fIn);
                uchar iVal = 0xff;
                switch(c) {
                case 'A':
                    iVal = 0;
                    break;
                case 'C':
                    iVal = 1;
                    break;
                case 'G':
                    iVal = 2;
                    break;
                case 'T':
                    iVal = 3;
                    break;
                }
                if (iVal <= 3) {
                    vNucs.push_back(iVal);
                }
            }

            int iNumNucs = vNucs.size();
        
            if (iNumNucs == 2*iGenomeSize) {
                pRGenome = new ulong[2*iGenomeSize];
                memset(pRGenome, 0, 2*iGenomeSize*sizeof(ulong));
                if (bOrdering == PLINK_ORDER) {
                    // plink ordering
                    int iPos = 0;
                    ulong *p0 = pRGenome;
                    ulong *p1 = pRGenome+iGenomeSize;
                    int i = 0;
                    while (i < iNumNucs) {
                        ulong iVal = vNucs[i++];
                        *p0 += iVal << iPos;
                        iVal = vNucs[i++];
                        *p1 += iVal << iPos;
                        iPos += 2;
                        if (iPos == 64) {
                            p0++;
                            p1++;
                            iPos = 0;
                        }
                    }
                } else {
                    // strand 1 then strand 2
                    int iPos = 0;
                    ulong *p0 = pRGenome;
                    int k = 0;
                    for (int i = 0; i < 2; i++) {
                        for (int j = 0; j < iGenomeSize; j++) {
                            ulong iVal = vNucs[k++];
                            *p0 += iVal<<iPos;
                            iPos += 2;
                            if (iPos == 64) {
                                p0++;
                                iPos = 0;
                            }
                        }
                        if (iPos != 0) {
                            printf(" : %lx\n", *p0);
                            p0++;
                            iPos = 0;
                        }
                    }
                }
            
            } else {
                printf("input file [%s] has %d Nucs (expected %d\n", sReferenceGenome, iNumNucs, iGenomeSize);
            }
            fclose(fIn);
        } else {
            printf("Couldn't open [%s] for reading\n", sReferenceGenome);
        }
    }
    return pRGenome;
}


//----------------------------------------------------------------------------
// createGenomeSegments
//
GenomeSegments *createGenomeSegments(int iGenomeSize, char *sSegments) {
    GenomeSegments *pGS = NULL;
    // if it contains separator : start/len list
    char *p = strchr(sSegments, ',');
    if (p != NULL) {
        int iResult = 0;
        pGS = GenomeSegments::createInstance(iGenomeSize, 0);
        p = strtok(sSegments, ",");
        while ((iResult == 0) && (p != NULL)) {
            char *pL = strchr(p, ';');
            if (pL != NULL) {
                *pL = '\0';
                pL++;
                int iStart=0;
                int iLen = 0;
                if (strToNum(p, &iStart)) {
                    if (strToNum(pL, &iLen)) {
                        pGS->addSegment(iStart, iLen);
                    } else {
                        iResult = -1;
                        printf("Expected number for segment start\n");
                    }
                } else {
                    iResult = -1;
                    printf("Expected number for segment start\n");
                }
            } else {
                printf("No ':' found in segment def\n");
                iResult = -1;
            }
            p = strtok(NULL, ",");
        }
    } else {
        int iNS = 0;
        if (strToNum(sSegments, &iNS)) {
            pGS = GenomeSegments::createInstance(iGenomeSize, iNS);
        } else {
            printf("Expected number of regular segments\n");
        }                
    }
    return pGS;
};
    
 
//----------------------------------------------------------------------------
// getPopName
char *getPopName(const char *sQHGPop, const char *sPopName) {
    char *pName = NULL;
    if (sPopName == NULL) {
        printf("Searching popname for [%s]\n", sQHGPop);
        pName = qdf_getFirstPopulation(sQHGPop);
    } else {
        printf("Checking popname for [%s]\n", sQHGPop);
        pName = qdf_checkForPop(sQHGPop, sPopName);
    }
     
    return pName;
}

//----------------------------------------------------------------------------
// getGenomeSize
int getGenomeSize(const char *sQHGPop,  char *sPopName) {
    int iGenomeSize = -1;

    hid_t hFile = qdf_openFile(sQHGPop);
    if (hFile >= 0) {
        hid_t hPops = qdf_openGroup(hFile, POPGROUP_NAME);
        if (hPops >= 0) {
            hid_t hSpc = qdf_openGroup(hPops, sPopName);
            if (hSpc >= 0) {
                int iRes = qdf_extractAttribute(hSpc, ATTR_GENETICS_GENOME_SIZE, 1, &iGenomeSize);
                if (iRes != 0) {
                    iGenomeSize = -1;
                }
                qdf_closeGroup(hSpc);
            } else {
                printf("Couldn't open sub group [%s]\n", sPopName);
            }
            qdf_closeGroup(hPops);
        } else {
            printf("Couldn't open group [%s]\n", POPGROUP_NAME);
        }

        qdf_closeFile(hFile);
    } else {
        printf("Couldn't open [%s] as QDF file\n", sQHGPop);
    }
    

    return iGenomeSize;
}



//----------------------------------------------------------------------------
// calcSegDist
//  create 2 pseudo genomes of size 2*masklen
//  by applying a msak to each of the original genomes' two strands
//  then use GeneUtils::calcDist()
// 
int calcSegDist(GenomeSegments *pGS, ulong *pGenome1, ulong *pGenome2, int iNumBlocks, double *pSegSum) {
    ulong *p1 = NULL;
    ulong *p2 = NULL;
    int iPrevLen = 0;
    int iNumSegments = pGS->getNumSegments();
    printf("Comparing\n");
    for (int i = 0; i < iNumSegments; i++) {
        printf("-- segment %d; size %d\n", i, pGS->getSegmentSize(i));
        int iMaskLen =  pGS->getMaskLength(i);
        if (iPrevLen < iMaskLen) {
            iPrevLen = iMaskLen;
            if (p1 != NULL) {
                delete[] p1;
            }
            p1 = new ulong[2*iMaskLen];
            if (p2 != NULL) {
                delete[] p2;
            }
            p2 = new ulong[2*iMaskLen];
        }
        ulong *p;
        // create masked parts for first strand
        p = pGS->getMaskedSegment(pGenome1, i);
        memcpy(p1, p, iMaskLen*sizeof(ulong));
        p = pGS->getMaskedSegment(pGenome2, i);
        memcpy(p2, p, iMaskLen*sizeof(ulong));

        // create masked parts for second strand
        p = pGS->getMaskedSegment(pGenome1+iNumBlocks, i);
        memcpy(p1+iMaskLen, p, iMaskLen*sizeof(ulong));
        p = pGS->getMaskedSegment(pGenome2+iNumBlocks, i);
        memcpy(p2+iMaskLen, p, iMaskLen*sizeof(ulong));

        int iDist = GeneUtils::calcDist(p1, p2,iMaskLen*GeneUtils::NUCSINBLOCK); 
        printf("Dist#%d : %d\n", i, iDist);
        pSegSum[i] += iDist;
    }
    return 0;
}

//----------------------------------------------------------------------------
// createPolyData
//
vtkPolyData *createPolyData(int iNumSegments, std::vector<double *> &vPoints, std::vector<double *>  &vSegSum) {
    vtkSmartPointer<vtkPolyData> pPolyData = NULL;
    

    if (vPoints.size() == vSegSum.size()) {
        pPolyData = vtkPolyData::New();
        
        vtkSmartPointer<vtkPoints> points =
            vtkSmartPointer<vtkPoints>::New();
        
        for (uint i = 0; i < vPoints.size(); i++) {
            points->InsertNextPoint(vPoints[i]);
        }
        pPolyData->SetPoints(points);
        
        
        vtkSmartPointer<vtkPointData> pointData = pPolyData->GetPointData();
        vtkSmartPointer<vtkDoubleArray> vda =
            vtkSmartPointer<vtkDoubleArray>::New();
        vda->SetNumberOfComponents(iNumSegments);
        vda->SetName("SegData");

        printf("adding to vda: ");
        for (uint i = 0; i < vSegSum.size(); i++) {
            for (int j = 0; j < iNumSegments; j++) {
                printf(" %f", vSegSum[i][j]);
                vda->InsertNextValue(pow(2,vSegSum[i][j]));
            }
            printf(" | ");
        }
        printf("\n");
        printf("Values:\n");vda->Print(std::cout);
 
        int iNumTup = vda->GetNumberOfTuples();
        int iNumComp = vda->GetNumberOfComponents();
        double *dTuple = new double[iNumComp];
        printf("VDA: %d tuples, %d components\n", iNumTup, iNumComp);
        for (int i=0; i < iNumTup; i++) {
            vda->GetTuple(i, dTuple);
            printf("Tuple %d: ", i);
            for (int j = 0; j < iNumComp; j++) {
                printf(" %f", dTuple[j]);
            }
            printf("\n");
        }
        delete[] dTuple;


        pointData->AddArray(vda);

    } else {
        printf("Must have same number of points as data items\n");
        printf("Num points %zd, num segsums %zd\n", vPoints.size(), vSegSum.size());
    }


    return pPolyData;
}

//----------------------------------------------------------------------------
// writePolyData
//
int writePolyData(vtkPolyData *pPolyData, const char *pOutputName) {
    int iResult = -1;
    pPolyData->Print(std::cout);

    vtkPolyDataWriter *pdw = vtkPolyDataWriter::New();
    pdw->SetFileName(pOutputName);
    pdw->SetInputData(pPolyData);
    int iRes = pdw->Write();
    if (iRes == 1) {
        iResult = 0;
    }
    return iResult;
}
/*
//----------------------------------------------------------------------------
// getSamples
//
int getSamples(const char *sQDFGrid, const char *pQDFPop, const char *sLocFile, const char *pPopName,
               locdata &mLocData) {
    int iResult = -1;
    WELL512 *pWELL = createWELL(pPhrase);
    printf("--- creating IDSampler from [%s]\n", sQDFGrid);
    IDSampler2 *pIS = IDSampler2::createInstance(sQDFGrid, pWELL);
    if (pIS != NULL) {
        printf("--- getting samples from [%s]\n", pQDFPop);
        stringvec vQDFPop;
        vQDFPop.push_back(pQDFPop);
        locspec ls(sLocFile);
        IDSample *pSample =  pIS->getSamples(vQDFPop, pPopName, &ls, mLocData, NULL);
        if (iResult == 0) {
 
            printf("Sampled: Total %d ids\n", pIS->getNumAgents);fflush(stdout);

        } else {
            printf("Couldn't get samples\n");
        }
        delete pIS;
    } else {
        printf("Couldn't create IDSampler for Grid [%s]\n", sQDFGrid);
    }
    return iResult;
}

//----------------------------------------------------------------------------
// readGenomes
//
int readGenomes(const char *sQHGPop,  char *pPopName, int iGenomeSize, int iNumAgents, idtype *pIDs, loctimeids &mmvIDs, std::map<idtype, ulong *> &mIDGen) {
    int iResult = -1;
    
    idset sIDs;
    for (loctimeids::const_iterator it1 = mmvIDs.begin(); it1 != mmvIDs.end(); ++it1) {
        for (std::map<float, std::vector<idtype> > ::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
            sIDs.insert(it2->second.begin(), it2->second.end());
        }
    }
    int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
    hid_t hFile = qdf_openFile(sQHGPop);
    if (hFile >= 0) {
        hid_t hPops = qdf_openGroup(hFile, POPGROUP_NAME);
        if (hPops >= 0) {
            hid_t hSpc = qdf_openGroup(hPops, pPopName);
            if (hSpc >= 0) {
                iResult = 0;
                
                
                // the buffer must hold a multiple of the array length
                uint iReadBufSize = BUFSIZE_READ_ADDITIONAL*2*iNumBlocks;
                long *aBuf = new long[iReadBufSize];
                
                // open the DataSet and data space
                hid_t hDataSet = H5Dopen2(hSpc, GENOME_DATASET_NAME, H5P_DEFAULT);
                hid_t hDataSpace = H5Dget_space(hDataSet);
                
                // get toal number of elements in dataset
                hsize_t dims;
                herr_t status = H5Sget_simple_extent_dims(hDataSpace, &dims, NULL);
                printf("Dataspace extent: %lld\n", dims);
                
                // initialize some counters and indexes
                hsize_t iCount;
                hsize_t iOffset = 0;
                
                int iIndex = 0;
                // loop until all elements have been read
                while ((iResult == 0) && (dims > 0)) {
                    // can we get a full load of the buffer?
                    if (dims > iReadBufSize) {
                        iCount = iReadBufSize;
                    } else {
                        iCount = dims;
                    }
                    //@@            printf("Slabbing offset %lld, count %lld\n", iOffset,iCount);
                    
                    // read a slab
                    hid_t hMemSpace = H5Screate_simple (1, &iCount, NULL); 
                    status = H5Sselect_hyperslab(hDataSpace, H5S_SELECT_SET, 
                                                 &iOffset, NULL, &iCount, NULL);
                    status = H5Dread(hDataSet, H5T_NATIVE_LONG, hMemSpace,
                                     hDataSpace, H5P_DEFAULT, aBuf);
                    
                    
                    if (status >= 0) {
                        // distribute the data 
                        int iNumThisPass = iCount/(2*iNumBlocks);
                        //                        int iNumToCopy = iNumThisPass;
                        //                        printf("Looking at %lld entries (%d longs) from pos %d\n", iCount, iNumThisPass, iIndex);
                        //@@                printf("starting inner loop: iTotalToDo %d, iNumThisPass %d\n", iTotalToDo, iNumThisPass);

                        long *pCurBlock = aBuf;
                        for (int k = 0; k < iNumThisPass; k++) {
                            idset::iterator its = sIDs.find(pIDs[iIndex+k]);
                            if (its != sIDs.end()) {
                                ulong *pBuf = new ulong[2*iNumBlocks];
                                memcpy(pBuf, pCurBlock, 2*iNumBlocks*sizeof(ulong));
                                mIDGen[pIDs[iIndex+k]] = pBuf;
                            }
                            pCurBlock += 2*iNumBlocks;
                        }
                         
                        iIndex += iNumThisPass;
                        
                        
                    } else {
                        printf("Error during slab reading\n");
                        iResult = -1;
                    }
                    
                    dims    -= iCount;
                    iOffset += iCount;
                    
                }
        

                qdf_closeDataSpace(hDataSpace);
                qdf_closeDataSet(hDataSet);
        
                delete[] aBuf;

                qdf_closeGroup(hSpc);
            } else {
                printf("Couldn't open sub group [%s]\n", pPopName);
            }
            qdf_closeGroup(hPops);
        } else {
            printf("Couldn't open group [%s]\n", POPGROUP_NAME);
        }

        qdf_closeFile(hFile);
    } else {
        printf("Couldn't open [%s] as QDF file\n", sQHGPop);
    }


    return iResult;
}

*/

//----------------------------------------------------------------------------
// compare
//
int compare(GenomeSegments *pGS, const ulong *pGenome1, const ulong *pGenome2, int iNumBlocks, double *pSegSum) {
    
    ulong *p1 = NULL;
    ulong *p2 = NULL;
    int iPrevLen = 0;
    int iNumSegments = pGS->getNumSegments();
    //    printf("Comparing\n");
    for (int i = 0; i < iNumSegments; i++) {
        //        printf("-- segment %d; size %d\n", i, pGS->getSegmentSize(i));
        int iMaskLen =  pGS->getMaskLength(i);
        if (iPrevLen < iMaskLen) {
            iPrevLen = iMaskLen;
            if (p1 != NULL) {
                delete[] p1;
            }
            p1 = new ulong[2*iMaskLen];
            if (p2 != NULL) {
                delete[] p2;
            }
            p2 = new ulong[2*iMaskLen];
        }
        ulong *p;
        // create masked parts for first strand
        p = pGS->getMaskedSegment(pGenome1, i);
        memcpy(p1, p, iMaskLen*sizeof(ulong));
        p = pGS->getMaskedSegment(pGenome2, i);
        memcpy(p2, p, iMaskLen*sizeof(ulong));

        // create masked parts for second strand
        p = pGS->getMaskedSegment(pGenome1+iNumBlocks, i);
        memcpy(p1+iMaskLen, p, iMaskLen*sizeof(ulong));
        p = pGS->getMaskedSegment(pGenome2+iNumBlocks, i);
        memcpy(p2+iMaskLen, p, iMaskLen*sizeof(ulong));

        int iDist = GeneUtils::calcDist(p1, p2,iMaskLen*GeneUtils::NUCSINBLOCK); 
        pSegSum[i] += iDist;
        //        printf("Dist#%d : %d\n", i, iDist);
    }
    return 0;
}
const double R = 6371.1;

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    char *sQDFPop          = NULL;
    char *sPopName         = NULL;
    char *sQDFGrid         = NULL;
    char *sLocList         = NULL;
    char *sReferenceGenome = NULL;
    char *sOutputName      = NULL;
    char *sSegments        = NULL;
    char *sPhrase          = NULL;

    printf("tralalal\n");
    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(8,
                                   "--pop-file:S!",         &sQDFPop,
                                   "--grid-file:S!",        &sQDFGrid,
                                   "--poop-name:S",         &sPopName,
                                   "--location-file:S!",    &sLocList,
                                   "--segment-def:S!",      &sSegments,
                                   "--ref-genome:S!",       &sReferenceGenome,
                                   "--output-file:S!",      &sOutputName,
                                   "--seed:S",              &sPhrase);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {
                char *pPhrase = NULL;
                if (sPhrase != NULL) {
                    pPhrase = new char[strlen(sPhrase)+1];
                    strcpy(pPhrase, sPhrase);
                } else {
                    pPhrase = new char[strlen(DEF_PHRASE)+1];
                    strcpy(pPhrase, DEF_PHRASE);
                }

                char *pPopName = getPopName(sQDFPop, sPopName);
                if (pPopName != NULL) {
                    printf("--- popName: [%s]\n", pPopName);
                    int iGenomeSize = getGenomeSize(sQDFPop, pPopName);
                    if (iGenomeSize > 0) {
                        int iNumBlocks = GeneUtils::numNucs2Blocks(iGenomeSize);
                        printf("--- iGenomeSize: [%d]\n", iGenomeSize);
                        GenomeSegments *pGS = createGenomeSegments(iGenomeSize, sSegments);
                        if (pGS != NULL) {
                            int iNumSegments = pGS->getNumSegments();
                            printf("--- iNumSegments: [%d]\n", iNumSegments);
                            
                            
                            ulong *pReferenceGenome = readReferenceGenome(sReferenceGenome, iGenomeSize);
                            if (pReferenceGenome != NULL) {
                                std::vector<double *> vPoints;
                                std::vector<double *>  vSegSum;

                                
                                WELL512 *pWELL = createWELL(pPhrase);
                                
                                QDFGenomeExtractor2 *pQGE = QDFGenomeExtractor2::createInstance(sQDFPop,
                                                                                                pPopName, 
                                                                                                ATTR_GENETICS_GENOME_SIZE,
                                                                                                ATTR_GENETICS_BITS_PER_NUC, 
                                                                                                GENOME_DATASET_NAME,
                                                                                                pWELL);
                                if (pQGE != NULL) {
                                    iResult = pQGE->createSelection(sLocList, NULL, true, 1000000);
                                    if (iResult == 0) {
                                        const locids &mLocIDs = pQGE->getLocIDs();
                                        printf("Have %zd locations\n", mLocIDs.size());

                                        locids::const_iterator it;
                                        for (it = mLocIDs.begin(); it != mLocIDs.end(); ++it) {
                                            printf("  %s\n", it->first.c_str());
                                            // one segment um/avg per location
                                            double *pSegSum = new double[iNumSegments];
                                            memset(pSegSum, 0, iNumSegments*sizeof(double));
                                            int iC = 0;
                                            
                                            idset::const_iterator itS;
                                            for (itS = it->second.begin(); itS != it->second.end(); ++itS) {
                                                const ulong *pGenome = pQGE->getSequence(*itS);
                                                
                                                iResult = compare(pGS, pReferenceGenome, pGenome, iNumBlocks, pSegSum);
                                                iC++;
                                                
                                            }
                                        
                                        
                                            printf("  before scaling with %d", iC);
                                            for (int j = 0; j< iNumSegments; j++) {
                                                
                                                printf(" %8.4f", pSegSum[j]);
                                                pSegSum[j] /= iC;
                                            }
                                            printf("\n");
                                            // show the result
                                            printf("  ");
                                            for (int i = 0; i < iNumSegments; i++) {
                                                printf(" %8.4f", pSegSum[i]);
                                            }
                                            printf("\n");
                                            
                                            
                                            vSegSum.push_back(pSegSum);
                                            const locdata &mLocData = pQGE->getLocData();
                                            const locitem &li = mLocData.at(it->first);
                                            printf("lon %f, lat %f\n", li.dLon,  li.dLat); 
                                            double dLon = li.dLon;
                                            double dLat = li.dLat;
                                            double *x = new double[3];
                                            x[0] = R*cos(dLon)*cos(dLat);
                                            x[1] = R*sin(dLon)*cos(dLat);
                                            x[2] = R*sin(dLat);
                                            
                                            vPoints.push_back(x);
                                        }
                                        
                                    }
                                    
                                    //   segsum = new double[NSeg]
                                    //   loop through ids in loc
                                    //     get genome for id
                                    //     calcSegDist(pS, refgen, genome, numblocks, segsum) (accumulate in segsum)
                                    //   divide segsum by numgenomes
                                    //   add location.position (3D) to vtkPolyData::Points
                                    //   add segsum to vtkPoyData::PointData
                                    
                                    
                                    vtkPolyData *pPolyData = createPolyData(iNumSegments, vPoints, vSegSum);
                                    if (pPolyData != NULL) {
                                        // write polydata to file
                                        iResult = writePolyData(pPolyData, sOutputName);
                                        if (iResult == 0) {
                                            printf("+++ success +++\n");
                                        } else {
                                            printf("Error writing vtkPolyData to [%s]\n", sOutputName);
                                        }
                                    }
                                    
                                    delete pQGE;
                                    delete pWELL;
                                    //!!!! clean up vSegSum
                                    for (uint i = 0; i < vSegSum.size(); i++) {
                                        delete[] vSegSum[i];
                                    }
                                    //!!!! clean up vPoints
                                    for (uint i = 0; i < vPoints.size(); i++) {
                                        delete[] vPoints[i];
                                    }
                                }
                            } else {
                                iResult =-1;
                                printf("Couldn't read reference genome from [%s]\n", sReferenceGenome);
                            }
                            delete pGS;
                        } else {
                            iResult = -1;
                            printf("Couldn't create enomeSegements from [%s]\n", sSegments);
                        }
                    } else {
                        iResult = -1;
                        printf("Couldn't determine genome size\n");
                    }
                    
                } else {
                    iResult = -1;
                    printf("Couldn't determine popname (%s) for [%s]\n", pPopName, sQDFPop);
                }
                delete[] pPhrase;
            } else {
                iResult = -1;
                usage(apArgV[0]);
            }

        } else {
            iResult = -1;
            printf("Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        iResult = -1;
        printf("Couldn't create ParamReader\n");
    }

    return iResult;
}

