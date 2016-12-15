#include <math.h>
#include "types.h"
#include "ParamReader.h"
#include "ConvKernel.h"
#include "BinConvolver.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "QMapHeader.h"

bool splitSizeString(char *pSizeString, size *piW, size *piH) {
    int iRead = sscanf(pSizeString, "%hux%hu", piW, piH);
    return (iRead == 2); 
}

void showParams(char *pApp, 
                char *pInputImage, 
                char *pOutputImage, 
                int iResamp,
                int iKernelType,
                double dParam,
                bool bMaj,
                bool bAllowNaN) {

    printf("%s called with:\n", pApp);
    printf("  Input  file  : %s\n", pInputImage);
    printf("  Output file  : %s\n", pOutputImage);
    printf("  Resampling   : %d\n", iResamp);
    printf("  Kernel type  : %s (%d)\n", ConvKernel::getName(iKernelType), iKernelType);
    printf("  Parameter    : %f\n", dParam);
    if (bMaj) {
        printf("  Use majority\n");
    }
    if (bAllowNaN) {
        printf("  Allow NaN values as convolution centers\n");
    }
}

//-----------------------------------------------------------------------------
// convolveData
//
bool convolveData(BinConvolver *pConv, char *pInputFile, const char *pOutputFile, int iResample, double dBackground, bool bMaj, bool bAllowNaN) {
    bool bOK = false;
    
    QMapReader<double> *pQMR = new QMapReader<double>(pInputFile, false);
    
    bOK = pQMR->extractData();
    if (bOK) {
        int iW = pQMR->getNLon();
        int iH = pQMR->getNLat();

        double **ppdData  = pQMR->getData();
        

        double **pDataOut = pConv->convolve(ppdData, iW, iH, iResample, dBackground, false, bMaj, bAllowNaN, NULL);

        // write output

        QMapHeader *pQMHO = new QMapHeader(QMAP_TYPE_DOUBLE, 
                                           pQMR->getLonMin() - (iResample/2)*pQMR->getDLon()/iResample, 
                                           pQMR->getLonMax() + (iResample/2)*pQMR->getDLon()/iResample, pQMR->getDLon()/iResample,
                                           pQMR->getLatMin() - (iResample/2)*pQMR->getDLat()/iResample, 
                                           pQMR->getLatMax() + (iResample/2)*pQMR->getDLat()/iResample, pQMR->getDLat()/iResample);

        printf("New Header:\n");
        pQMHO->display();

        FILE *fOut = fopen(pOutputFile, "wb");
        if (fOut != NULL) {
            pQMHO->addHeader(fOut);
            for (int i = 0; bOK && (i < iH*iResample); i++) {
                double *p = pDataOut[i];
                int iWritten = fwrite(p, sizeof(double), iW*iResample, fOut);
                if (iWritten != iW*iResample) {
                    printf("Only written %d instead of %d\n", iWritten, iW*iResample);
                    bOK = false;
                }
            }
            fclose(fOut);
        } else {
            printf("Couldn't open %s for writing\n", pOutputFile);
            bOK = false;
        }
        delete pQMHO;
    }        
        
    delete pQMR;
    return bOK;
}



int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sInputFile[256];
    char sOutputFile[256];
    char sKernelSize[256];
    char sDataSize[256];
    int  iKernelType=0;
    double dParam = 1;
    int iResampling = 1;
    double dBackground = dNaN;

    *sInputFile   = '\0';
    *sOutputFile  = '\0';
    *sKernelSize  = '\0';
    *sDataSize    = '\0';

    bool bMaj      = false;
    bool bAllowNaN = false;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(9, "-i:s!",   sInputFile,
                                  "-o:s!",   sOutputFile,
                                  "-s:s!",   sKernelSize,
                                  "-k:i!",   &iKernelType,
                                  "-r:i",    &iResampling,
                                  "-b:d",    &dBackground,
                                  "-p:d",    &dParam,
                                  "-m:0",    &bMaj,
                                  "-n:0",    &bAllowNaN);
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (QMapHeader::getQMapType(sInputFile) == QMAP_TYPE_DOUBLE) {
            
                size iWK;
                size iHK;
                if (splitSizeString(sKernelSize, &iWK, &iHK)) {
                    if ((iWK%2!=0)&&(iHK%2!=0)) {
                        ConvKernel *pK = ConvKernel::createKernel(iKernelType, iWK, iHK, dParam);
                        if (pK != NULL) {

                            //------
                            showParams(apArgV[0], 
                                       sInputFile, 
                                       sOutputFile, 
                                       iResampling, 
                                       iKernelType,
                                       dParam, bMaj, bAllowNaN);
                            printf("The kernel is %p\n", pK);
                            BinConvolver *pConv = new BinConvolver(pK);
                            bOK = convolveData(pConv, sInputFile, sOutputFile, iResampling, dBackground, bMaj, bAllowNaN);
                        
                            delete pConv;
                            delete pK;
                        } else {
                            printf("Invalid Kernel type : %d\n", iKernelType);
                        }
                    
                    } else {
                        printf("Bad Kernel size - must be odd : %s\n", sKernelSize);
                    }
                } else {
                    printf("Bad Kernel size : %s\n", sKernelSize);
                }
            } else {
                printf("Input file [%s] must be a valid QHG Binary map file of doubles\n", sInputFile);
            }
            
        } else {
            //usage
            printf("%s - convolve an image\n", apArgV[0]);
            printf("Usage:\n");
            printf("  %s [-a] -i <InputFile> -o <OutputFile> -S <DataSize> -k <KernelType> -s <KernelSize> [-p <Param>]\n", apArgV[0]);
            printf("     [-r <resampling>] [-m] [-n]\n");
            printf("where:\n");
            printf("  InputFile:     name of input file (a binary file of doubles)\n");
            printf("  OutputFile:    name of output file (will be a binary file of doubles)\n");
            printf("  DataSize:      size of data (e.g. 720x360)\n");
            printf("  KernelType:    number of Kernel type to use (see below)\n");
            printf("  KernelSize:    size of kernel (eg. 31x31)\n");
            printf("  Param:         parameter for kernel\n");
            printf("  resampling:    resample size\n");
            printf("  -m             majority filter\n");  
            printf("  -n             allow NaN values for convolve centers\n");  
            printf("\n");
            printf("Kernel types:\n");
            for (int i = 0; i < ConvKernel::getNumKernelTypes(); ++i) {
                printf("%d  -  %s\n", i, ConvKernel::getName(i));
            }
            

                   
        }
    } else {
        printf("Error in Parameter Reader\n");
    }

    delete pPR;
    return iResult;
}
 
