#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <vector>
#include <string>

#include <hdf5.h>

#include "ParamReader.h"
#include "SurfaceGrid.h"
#include "QDFImageExtractor.h"
#include "strutils.h"
#include "LineReader.h"

typedef struct desc_entry {
    double dTime;
    std::string sQDFList;
    std::string sOutPat;
    desc_entry(double dTime0, char *pQDFList, char *pOutPat):dTime(dTime0),sQDFList(pQDFList),sOutPat(pOutPat){};
} desc_entry;

typedef  std::vector<desc_entry> desc_list;

//----------------------------------------------------------------------------
// usage
//
void usage(const char *pApp) {
    printf("%s - export QDF arrays to PNG\n", pApp);
    printf("Usage:\n");
    printf("  %s -g <qdf_grid> -q <qdf_data>[,<qdf_data>]*]  \n", pApp);
    printf("     -s <w>x<h> -o <outpat>  -a <arrayspec>,[<arrayspec>]*\n");
    printf("    [-c <operation>] [-r <lon_roll>] [-t <timestamp>] [-v]\n");
    printf("or");
    printf("  %s -g <qdf_grid> -f <batch_file>\n", pApp);
    printf("     -s <w>x<h> -o <outpat>  -a <arrayspec>,[<arrayspec>]*\n");
    printf("    [-c <operation>] [-r <lon_roll>] [-t <timestamp>] [-v]\n");
    printf("where\n");
    printf("  qdf_grid       QDF containing at least a grid\n");
    printf("  qdf_data       QDF file containing data to be extracted\n");
    printf("                 which is not contained in <qdf_grid>\n");
    printf("  batch_file     file containing lines of the form:\n");
    printf("                 <timestamp>\":\"<qdf_data>[,<qdf_data>]*\":\"<outpat>\n");
    printf("  w              width  of output png\n");
    printf("  h              height of output png\n");
    printf("  outpat         pattern for output pngs: the substring '###' will be replaced with the array name\n");
    printf("  arrayspec      array specification. Format:\n");
    printf("                 array_spec ::= <array_name>[@<index>][|<lookup>]\n");
    printf("                 array_name  :   name of array (s. below)\n");
    printf("                 index       :   index of qdf in which to look (0: qdf_geogrid, 1-N: qdf-data in given order)\n");
    printf("                 lookup      :   info for lookup, with format <lookup_name>[:<data>]* (s. below)\n");
    printf("  operation      compositing operator (currenly only: 'over' (simple alpha compositing)\n");
    printf("  longroll       longitude for smallest x value (rolls image)\n");
    printf("  timestamp      timestamp to be rendered on image\n");
    printf("  -v             verbose\n");
    printf("Arraynames:\n");
    printf("  lon       longitude   (Geography::m_adLongitude)\n");
    printf("  lat       latitude    (Geography::m_adLatitude)\n");
    printf("  alt       altitudes   (Geography::m_adAltitude)\n");
    printf("  ice       ice cover   (Geography::m_abIce)\n");
    printf("  water     water       (Geography::m_adWater)\n");
    printf("  coastal   coastal     (Geography::m_abCoastal)\n");
    printf("  temp      temperature (Climate::m_adAnnualMeanTemp)\n");
    printf("  rain      rainfall    (Climate::m_adAnnualRainfall)\n");
    printf("  npp       total npp   (NPPVegetation::m_adTotalANPP)\n");
    printf("  npp_b     base npp    (NPPVegetation::m_adBaseANPP)\n");
    printf("  dist      travel distance (MoveStats::m_adDist)\n");
    printf("  time      travel time     (MoveStats::m_adTime)\n");
    printf("  pop       population number\n");
    printf("Lookups:\n");
    printf("  rainbow   data: min, max\n");
    printf("  rainbow2  data: min, max\n");
    printf("  geo       data: min, sealevel, max\n");
    printf("  twotone   data: Sepvalue, RGBA1, RGBA2\n");
    printf("  fadeout   data: min,max, RGBAmax\n");
    /*
    printf("Postprocessing:\n");
    printf("For superposition use imagemagick:\n");
    printf("  composite -compose Over onklop_ice.png onklop_alt.png destination.png\n");
    */
    printf("Call example\n");
    printf("./QDF2PNG -g zworld_22.0_kya_256.qdf\\\n");
    printf("          -q ooa_pop-Sapiens_ooa__010000.qdf,aternative_ice.qdf \\\n");
    printf("          -s 720x360\\\n");
    printf("          -o onklop_###_024.PNG \\\n");
    printf("          -a alt|geo:-6000:0:6000,ice@2|twotone:0.5:#00000000:#FFFFFFFF,\\\n");
    printf("             pop_Sapiens_ooa@1|fadeout:0:40:#00FF00FF\\\n"); 
    printf("          -r -25\\\n");
    printf("          -c over\n");
}  


//----------------------------------------------------------------------------
// splitQDFs
//
int splitQDFs(char *pQDFData, string_list &vQDFs) {
    int iResult = 0;
    if (pQDFData != NULL) {
        char *pCtx;
        char *p = strtok_r(pQDFData, ",", &pCtx);
        while (p != NULL) {
            vQDFs.push_back(p);
            p = strtok_r(NULL, ",", &pCtx);
        }
        if (vQDFs.size() == 0) {
            iResult = -1;
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// splitDescFile
//
int splitDescFile(char *sDescFile,  desc_list &tsl) {
    int iResult = -1;

    LineReader *pLR = LineReader_std::createInstance(sDescFile, "rt");
    if (pLR != NULL) {
        iResult = 0;
        char *pLine = pLR->getNextLine();
        while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
            char *p = strtok(pLine, ":");
            if (p != NULL) {
                double dTime = 0;
                if (strToNum(p, &dTime)) {
                    p = strtok(NULL, ":");
                    if (p != NULL) {
                        char *p1 = strtok(NULL, ":");
                        if (p1 != NULL) {

                            tsl.push_back(desc_entry(dTime, p, p1));
                            pLine = pLR->getNextLine();
                            
                        } else {
                            printf("Expected third entry (output pat)\n");
                            iResult = -1;
                        }
                    } else {
                        printf("Expected second entry (qdf list)\n");
                        iResult = -1;
                    }
                } else {
                    printf("first entry must be a double [%s]\n", p);
                    iResult = -1;
                }

            } else {
                printf("DescFile empty string?\n");
                iResult = -1;
            }
            
            
        }
        delete pLR;
    } else {
        printf("Couldn't open Descfile [%s]\n", sDescFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// doSingleImage
//
int doSingleImage(const char *sQDFData, SurfaceGrid *pSG, char *sQDFGrid, char *sArrayData, img_prop &ip, const char *sOutPat, char *sCompOp, int iTime, bool bVerbose) {
    int iResult = 0;

    string_list vQDFs;

    char *pCopy = new char[strlen(sQDFData)+1];
    strcpy(pCopy, sQDFData);
    iResult = splitQDFs(pCopy, vQDFs);
    if (iResult == 0) {
        //        if (bVerbose) {
            printf("QDF files:\n");
            for (uint i = 0; i < vQDFs.size(); ++i) {
                printf("%2u: %s\n", i, vQDFs[i].c_str());
            }
            //        }
        
        QDFImageExtractor *pQIE = QDFImageExtractor::createInstance(pSG, sQDFGrid, vQDFs, sArrayData, ip, bVerbose);
        if (pQIE != NULL) {
            pQIE->extractAll(sOutPat, sCompOp, iTime);
            delete pQIE;
        } else {
            printf("Couldn't create QDFImageExtractor\n");
            iResult = -1;
        }
        
        
        
    } else {
        printf("Couldn't split qdf data\n");
        iResult = -1;
    }
    delete[] pCopy;
    return iResult;
}

//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    char *sQDFGrid     = NULL;
    char *sQDFData     = NULL;
    char *sSize        = NULL;
    char *sOutPat      = NULL;
    char *sArrayData   = NULL;
    char *sCompOp      = NULL;
    char *sDescFile    = NULL;
    double dLonRoll    = -180.0;
    int    iTime       = -1;             
    bool   bVerbose    = false;

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(10,  
                               "-g:S!",   &sQDFGrid,
                               "-q:S",    &sQDFData,
                               "-f:S",    &sDescFile,
                               "-s:S!",   &sSize,
                               "-o:S",    &sOutPat,
                               "-a:S!",   &sArrayData,
                               "-c:S",    &sCompOp,
                               "-r:d",    &dLonRoll,
                               "-t:i",    &iTime,
                               "-v:0",    &bVerbose);
    
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            if (bVerbose) {
                pPR->display();
            }
            int iW = -1;
            int iH = -1;
            printf("Verbose: %s\n", bVerbose?"yes":"no");
            bOK = splitSizeString(sSize, &iW, &iH);
            if (bOK) {
                img_prop ip(iW, iH, dLonRoll);

                desc_list tsl;

                if (sDescFile != NULL) {
                    iResult = splitDescFile(sDescFile, tsl);
                } else if (sQDFData != NULL) {
                    if (sOutPat != NULL) {
                        tsl.push_back(desc_entry(iTime, sQDFData, sOutPat));
                    } else {
                        printf("OutPat (-o) must be given if no BatchFile (-f) is used\n");
                        iResult = -1;
                    }
                } else {
                    printf("Neither QDFData (-q) nor DescFile (-f)  specified\n");
                    iResult = -1;
                }


                if (iResult == 0) {
                    SurfaceGrid *pSG = SurfaceGrid::createInstance(sQDFGrid);
                
                    if (pSG != NULL) {
                        
                        for (uint i = 0; (iResult == 0) && (i < tsl.size()); ++i) {
                            printf("List: [%s]\n", tsl[i].sQDFList.c_str());
                            iResult = doSingleImage(tsl[i].sQDFList.c_str(), 
                                                    pSG, 
                                                    sQDFGrid, 
                                                    sArrayData, 
                                                    ip, 
                                                    tsl[i].sOutPat.c_str(), 
                                                    sCompOp, 
                                                    tsl[i].dTime,
                                                    bVerbose);
                        }
                        delete pSG;
                        if (iResult == 0) {
                            printf("+++ success +++\n");
                        } else {
                            printf("--- failed ---\n");
                        }
                    } else {
                        printf("Couldn't create SurfaceGrid\n");
                    }
                }
            } else {
                printf("Couldn't split size string\n");
                iResult = -1;
            }
        } else {
            printf("ParamReader result: %d\n", iResult);
            printf("%s: %s %s\n", pPR->getErrorMessage(iResult).c_str(),  pPR->getBadArg().c_str(), pPR->getBadVal().c_str());
            usage(apArgV[0]);
            iResult = -1;
        }
    } else {
        printf("Error in setOptions\n");
    }
    delete pPR;
    return iResult;
}
