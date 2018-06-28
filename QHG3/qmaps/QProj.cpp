#include <stdio.h>

#include "ParamReader.h"

#include "QMapHeader.h"
#include "QMapReader.h"
#include "ValReader.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"

#include "GeoInfo.h"
#include "GeoMapper.h"
#include "GeoMapper.cpp"


void usage(const char *pApp) {
    printf("%s - QMap re-projection\n", pApp);
    printf("usage:\n");
    printf("  %s -i <input_qmap>  --t1 <proj_type_in>  --g1 <proj_grid_in>\n", pApp);
    printf("     -o <output_qmap> --t2 <proj_type_out> --g2 <proj_grid_out>\n");
    printf("     [-d]\n");
    printf("where\n");
    printf("  inputfile      name of input qmap\n");
    printf("  outputfile     name of output qmap\n");
    printf("  proj_type_in   projection type of input qmap\n");
    printf("  proj_grid_in   projection grid of input qmap\n");
    printf("  proj_type_out  projection type for output qmap\n");
    printf("  proj_grid_out  projection grid for output qmap\n");
    printf("  -d             longitude and latitude in in degrees\n");
    printf("\n");
    printf("  projection type ::= <projection-id>\":\"<lon0>\":\"<lat0>\":\"<numParams>[\": \"<param>]\n");
    printf("  projection grid  ::= <GridW>\":\"<GridH>\":\"<RealW>\":\"<RealH>\":\"<OffsX>\":\"<OffsY>\":\"<R>\n");
    printf("\n");
    printf("  numParams: number of additional params (usually 0)\n");
    printf("  OffsX, OffsY; offset in grid coordinates (use 'c' for centered)\n");
    printf("\n");
}

template <class T>
int remapT(QMapReader<T> *pQR, const ProjGrid *pPG0, const ProjGrid *pPG1, Projector *pP0, Projector *pP1, const char *pOutFile, QMapHeader *pQMH, T tDef) {
    int iResult = 0;
    // get data array
    T **aatData0 = pQR->getData();

    // create new data array 
    T **aatData1 = QMapUtils::createArray(pPG1->m_iGridW, pPG1->m_iGridH, tDef);
    // create GeoMapper
    GeoMapper<T> *pGM = new GeoMapper<T>(pPG0, pP0, aatData0, pPG1, pP1, aatData1);

    // map
    pGM->setClosure(true,true);
    pGM->map(tDef);
    
    //
    FILE *fOut = fopen(pOutFile, "wb");
    if (fOut != NULL) {
        pQMH->addHeader(fOut);
        bool bOK =QMapUtils::writeArray(fOut, pPG1->m_iGridW, pPG1->m_iGridH, aatData1);
        QMapUtils::deleteArray(pPG1->m_iGridW, pPG1->m_iGridH, aatData1);
        fclose(fOut);
        iResult = bOK?0:-1;
    } else {
        iResult = -1;
    }
    return iResult;
}


int remap(char *pInFile, const ProjGrid *pPG0, const ProjGrid *pPG1, Projector *pP0, Projector *pP1, const char *pOutFile) {
    int iResult = 0;
    int iType = 0;
    ValReader *pVR = QMapUtils::createValReader(pInFile, true, &iType);
    if (pVR != NULL) {

        QMapHeader *pQMH = new QMapHeader(iType,
                                          0, pPG1->m_iGridW, 1,
                                          0, pPG1->m_iGridH, 1,
                                          "Alt", "X", "Y");


        switch (iType) {
        case QMAP_TYPE_UCHAR: 
            iResult = remapT(dynamic_cast<QMapReader<unsigned char>*>(pVR), pPG0, pPG1, pP0, pP1, pOutFile, pQMH, (uchar)0xfe);
            break;
        case QMAP_TYPE_SHORT: 
            iResult = remapT(dynamic_cast<QMapReader<short int>*>(pVR),     pPG0, pPG1, pP0, pP1, pOutFile, pQMH, (short int) 0);
            break;
        case QMAP_TYPE_INT:
            iResult = remapT(dynamic_cast<QMapReader<int>*>(pVR),           pPG0, pPG1, pP0, pP1, pOutFile, pQMH, (int) 0);
            break;
        case QMAP_TYPE_LONG:
            iResult = remapT(dynamic_cast<QMapReader<long>*>(pVR),          pPG0, pPG1, pP0, pP1, pOutFile, pQMH, (long) 0);
            break;
        case QMAP_TYPE_FLOAT: 
            iResult = remapT(dynamic_cast<QMapReader<float>*>(pVR),         pPG0, pPG1, pP0, pP1, pOutFile, pQMH, fNaN);
            break;
        case QMAP_TYPE_DOUBLE: 
            iResult = remapT(dynamic_cast<QMapReader<double>*>(pVR),        pPG0, pPG1, pP0, pP1, pOutFile, pQMH, dNaN);
            break;
        default:
            iResult = -3;
        }
    }
    return iResult;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char sInputFile[256];
    char sOutputFile[256];
    char sProjType0[256];
    char sProjGrid0[256];
    char sProjType1[256];
    char sProjGrid1[256];

    bool bDegrees = false;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(7,  
                               "-i:s!",    sInputFile,
                               "--t1:s!",  sProjType0,
                               "--t2:s",   sProjType1,
                               "--g1:s!",  sProjGrid0,
                               "--g2:s",   sProjGrid1,
                               "-o:s!",    sOutputFile,
                               "-d:0",     &bDegrees);
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            printf("i:   %s\n", sInputFile);
            printf("t1:  %s\n", sProjType0);
            printf("g1:  %s\n", sProjGrid0);
            printf("t2:  %s\n", sProjType1);
            printf("g2:  %s\n", sProjGrid1);
            printf("out: %s\n", sOutputFile);
            printf("d:   %s\n", bDegrees?"yes":"no");

            ProjType *pPT0 = ProjType::createPT(sProjType0, bDegrees);
            ProjType *pPT1 = ProjType::createPT(sProjType1, bDegrees);
            ProjGrid *pPG0 = ProjGrid::createPG(sProjGrid0);
            ProjGrid *pPG1 = ProjGrid::createPG(sProjGrid1);

            GeoInfo *pGI = GeoInfo::instance();
            Projector *pP0 = pGI->createProjector(pPT0);
            Projector *pP1 = pGI->createProjector(pPT1);

            
            

            iResult = remap(sInputFile, pPG0, pPG1, pP0, pP1, sOutputFile);

            
            delete pPT0;
            delete pPT1;
            delete pPG0;
            delete pPG1;
        } else {
            printf("res %d\n", iResult);
            usage(apArgV[0]);
        }
    } else {
        printf("Error in setOptions\n");
    }
            

}
