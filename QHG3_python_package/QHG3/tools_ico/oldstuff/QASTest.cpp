#include <stdio.h>
#include <string.h>


#include "QDFArraySniffer.h"
#include "ValueProvider.h"
#include "QDFValueProvider.h"


int main(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sHDF[256];
    char sHDFP[256];
    *sHDF = '\0';
    *sHDFP = '\0';

    if (iArgC > 1) {
        strcpy(sHDF, apArgV[1]);
        if (iArgC > 2) {
            strcpy(sHDFP, apArgV[2]);
        }
        hid_t hFile = qdf_openFile(sHDF);
        if (hFile > 0) {
            QDFArraySniffer *pQAS = new QDFArraySniffer(hFile);
            printf("Scanning [%s]\n", sHDF);
            iResult = pQAS->scan();
            if ((iResult == 0) && (*sHDFP != '\0')) {
                hid_t hFile2 = qdf_openFile(sHDFP);
                if (hFile2 > 0) {
                    printf("Scanning [%s]\n", sHDFP);
                    iResult = pQAS->scanPopFile(hFile2);
                    qdf_closeFile(hFile2);
                } else {
                    iResult = -1;
                    printf("%s is not a hdf file\n", sHDFP);
                }
            }
            
            if (iResult == 0) {
                std::vector<std::string> &vItems = pQAS->getItems();
                printf("Found %lu compatible data sets:\n", vItems.size());
                for (uint i = 0; i < vItems.size(); ++i) {
                    printf("  %s\n", vItems[i].c_str());
                }
            } else {
                printf("scan failed\n");
            } 
            printf("now starting QDFValueProvider\n");
            if (iResult == 0) {
                ValueProvider *pQVP = QDFValueProvider::createValueProvider(sHDF);
                if (pQVP != NULL) {
                    dynamic_cast<QDFValueProvider *>(pQVP)->show();
                    std::vector<std::string> &vItems = pQAS->getItems();
		    for (uint i = 0; i < vItems.size(); ++i) {
                        std::string s = vItems[i];
                        if (pQVP->setMode(s.c_str()) == 0) {
                            printf("Val(0): %f\n", pQVP->getValue(0));
                            printf("Val(777): %f\n", pQVP->getValue(777));
                            printf("Val(778): %f\n", pQVP->getValue(778));
                            printf("Val(32768): %f\n", pQVP->getValue(32768));
                        }
                    }
                    dynamic_cast<QDFValueProvider *>(pQVP)->show();
                    printf("--- now loading [%s]\n", sHDFP);
                    if (*sHDFP != '\0') {
                        pQVP->addFile(sHDFP);
                        dynamic_cast<QDFValueProvider *>(pQVP)->show();
                        std::vector<std::string> &vItems = pQAS->getItems();
                        for (uint i = 0; i < vItems.size(); ++i) {
                            std::string s = vItems[i];
                            if (pQVP->setMode(s.c_str()) == 0) {
                                printf("Val(0): %f\n", pQVP->getValue(0));
                                printf("Val(777): %f\n", pQVP->getValue(777));
                                printf("Val(778): %f\n", pQVP->getValue(778));
                                printf("Val(32768): %f\n", pQVP->getValue(32768));
                            }
                        }
                        dynamic_cast<QDFValueProvider *>(pQVP)->show();


                    }
                    delete pQVP;
                }
            }
            delete pQAS;
            qdf_closeFile(hFile);
        } else {
            printf("%s is not a hdf file\n", sHDF);
        }
    } else {
        printf("%s <qdffile>\n", apArgV[0]);
    }
    

    return iResult;
}
