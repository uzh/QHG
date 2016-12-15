#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <omp.h>

#include "ids.h"
#include "EventData.h"
#include "EventManager.h"
#include "strutils.h"
#include "colors.h"
#include "MessLogger.h"
#include "ParamReader.h"
#include "QDFUtils.h"

#include "ValReader.h"
#include "QMapReader.h"
#include "QMapUtils.h"

#include "SCell.h"
#include "SCellGrid.h"
#include "GridFactory.h"
#include "GridReader.h"
#include "Geography.h"
#include "GeoReader.h"
#include "Climate.h"
#include "ClimateReader.h"
#include "NPPVeg.h"

#include "IcoGridNodes.h"
#include "Surface.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "Lattice.h"

#include "VegReader.h"
#include "VegFactory.h"
#include "MoveStats.h"

#include "PopBase.h"
#include "PopLooper.h"
#include "PopReader.h"
#include "PopulationFactory.h"
#include "MoveStatReader.h"
#include "StatusWriter.h"
#include "IDGen.h"

#include "SimParams.h"

#define LAYERSIZE 16384
#define SEED_RANDOM "random"
#define SEED_HEADER "SEED"
#define SEED_FOOTER "SEED_END"

// use random numbers from a table for default state
static unsigned int s_aulDefaultState[] = {
    0x2ef76080, 0x1bf121c5, 0xb222a768, 0x6c5d388b, 
    0xab99166e, 0x326c9f12, 0x3354197a, 0x7036b9a5, 
    0xb08c9e58, 0x3362d8d3, 0x037e5e95, 0x47a1ff2f, 
    0x740ebb34, 0xbf27ef0d, 0x70055204, 0xd24daa9a,
};


//-----------------------------------------------------------------------------
// constructor
//
SimParams::SimParams() 
    : m_iNumIters(-1),
      m_iLayerSize(LAYERSIZE),
      m_iShuffle(0),
      m_hFile(H5P_DEFAULT),
      m_pLR(NULL),
      m_pPR( new ParamReader()),       
      m_pCG(NULL),  
      m_pGeo(NULL), 
      m_pCli(NULL),
      m_pVeg(NULL),
      m_pPopLooper(NULL),
      m_apIDG(NULL),
      m_pSW(NULL),
      m_pEM(new EventManager()),
      m_bHasMoveStats(false),
      m_pSurface(NULL),
      m_pPopFac(NULL),
      m_fStartTime(fNaN) {

    strcpy(m_sOutputPrefix, DEF_OUT_PREFIX);
    strcpy(m_sOutputDir, "./");
    m_vDataDirs.clear();
    *m_sHelpTopic = '\0';
    m_bHelp = false;
    int iNumThreads =  omp_get_max_threads();
    // the IDGen are allocated but not created yet (this happens later in Simulator
    m_apIDG = new IDGen*[iNumThreads];
    memset(m_apIDG, 0, iNumThreads*sizeof(IDGen*));
    //    memset(m_aulState, 0, STATE_SIZE*sizeof(uint32_t));
    memcpy(m_aulState, s_aulDefaultState, STATE_SIZE*sizeof(uint32_t));
    
    *m_sConfigOut = '\0';

}


//-----------------------------------------------------------------------------
// destructor
//
SimParams::~SimParams() {
    if (!m_bHelp && (*m_sHelpTopic == '\0')) {
        LOG_DISP("-------------------------------------\n");
        MessLogger::showLog(SHOW_ALL);
    }
    if (m_pPR != NULL) {
        delete m_pPR;
    }
    if (m_pCG != NULL) {
        delete m_pCG;
    }
    if (m_pGeo != NULL) {
        delete m_pGeo;
    }
    if (m_pCli != NULL) {
        delete m_pCli;
    }
    if (m_pVeg != NULL) {
        delete m_pVeg;
    }
    if (m_pPopLooper != NULL) {
        delete m_pPopLooper;
    }
    if (m_pSW != NULL) {
        delete m_pSW;
    }
    if (m_pEM != NULL) {
        delete m_pEM;
    }
    if (m_pLR != NULL) {
        delete m_pLR;
    }
    if (m_apIDG != NULL) {
        for (int iT = 0; iT < omp_get_max_threads(); iT++) {
            delete m_apIDG[iT];
        }
        delete[]  m_apIDG;
    }

    if (m_pSurface != NULL) {
        delete m_pSurface;
    }
    if (m_pPopFac != NULL) {
        delete m_pPopFac;
    }
    qdf_closeFile(m_hFile);
    MessLogger::free();
}


//-----------------------------------------------------------------------------
// setHelp
//
int SimParams::setHelp(bool bHelp) {
    m_bHelp = bHelp;
    return 0;
}


//-----------------------------------------------------------------------------
// setHelpTopic
//
int SimParams::setHelpTopic(const char *pHelpTopic) {
    strcpy(m_sHelpTopic, pHelpTopic);
    return 0;
}


//-----------------------------------------------------------------------------
// helpParams
//
void SimParams::helpParams() {
    printf("  -h,                        show help\n");            
    printf("  --help=<topic>             show help for topic (use name of option or \"all\")\n");            
    printf("  --log-file=<filename>      set name of log file (default:\"%s\")\n", DEF_LOG_FILE);            
    printf("  --grid=<grid-file>         set grid file\n");     
    printf("  --num-iters=<iters>        set number of iterations\n");  
    printf("  --geo=<geo-file>           set geography file\n");       
    printf("  --climate=<climate-file>   set climate file\n");    
    printf("  --pops=<pop-list>          set population files\n");       
    printf("  --output-prefix=<name>     prefix for output files (default: \"%s\")\n", DEF_OUT_PREFIX);    
    printf("  --output-dir=<dirname>     output directory (default: \"./\")\n");    
    printf("  --data-dirs=<dirnames>     data directories (default: \"./\")\n");    
    printf("  --read-config=<conf>       read config from file <conf>\n");
    printf("  --write-config=<conf>      write config to file <conf>\n");
    printf("  --events=<event-list>      set events\n");    
    printf("   -n <iters>                set number of iterations\n");
    printf("  --layer-size=<size>        set layer size (default: %d)\n", LAYERSIZE);
    printf("  --shuffle=<num>            shift random generator's sequence (default: 0)\n");
    printf("  --seed=<seedtype>          set seed for random number generators\n");
    printf("  --pop-params=<paramstring> set special population parameters\n");


}


//-----------------------------------------------------------------------------
// readOptions
//
int SimParams::readOptions(int iArgC, char *apArgV[]) {
    int iResult = -1;

    char sHelpTopic[SHORT_INPUT];
    char sGridFile[MAX_PATH];
    char sPops[MAX_PATH];
    char sGeoFile[MAX_PATH];
    char sClimateFile[MAX_PATH];
    char sVegFile[MAX_PATH];
    // this might be to big to place on the buffer
    //    char sEvents[2048*MAX_PATH];
    // better use a string allocated by ParamReader instead
    char *pEvents;
    char sOutputPrefix[MAX_PATH];
    char sOutputDir[MAX_PATH];
    char sDataDirs[MAX_PATH];
    char sConfigIn[MAX_PATH];
    char sConfigOut[MAX_PATH];
    char sSeed[MAX_PATH];
    char sDummyLog[MAX_PATH];
    char sPopParams[MAX_PATH];
 
    *sHelpTopic    = '\0';
    *sGridFile     = '\0';
    *sPops         = '\0';
    *sGeoFile      = '\0';
    *sClimateFile  = '\0';
    *sVegFile      = '\0';
    //    *sEvents       = '\0';
    pEvents = NULL;
    *sOutputPrefix = '\0';
    *sOutputDir    = '\0';
    *sDataDirs     = '\0';
    *sConfigIn     = '\0';
    *sConfigOut    = '\0';
    *sSeed         = '\0';
    *sDummyLog     = '\0';
    *sPopParams    = '\0';

    bool bHelp = false;
    m_bCalcGeoAngles = false;

    bool bOK = m_pPR->setOptions(21, 
                                 "-h:0",                  &bHelp,
                                 "--help:s",              sHelpTopic,
                                 "--log-file:s",          sDummyLog,
                                 "--grid:s!",             sGridFile,
                                 "--geo:s",               sGeoFile,
                                 "--climate:s",           sClimateFile,
                                 "--veg:s",               sVegFile,
                                 "--pops:s",              sPops,
                                 "--output-prefix:s",     sOutputPrefix,
                                 "--output-dir:s",        sOutputDir,
                                 "--data-dirs:s",         sDataDirs,
                                 "--read-config:s",       sConfigIn,
                                 "--write-config:s",      sConfigOut,
                                 "--events:S",           &pEvents,
                                 "--num-iters:i",        &m_iNumIters,
                                 "-n:i",                 &m_iNumIters,
                                 "--layer-size:i",       &m_iLayerSize,
                                 "--shuffle:i",          &m_iShuffle,
                                 "--seed:s",              sSeed,
                                 "--pop-params:s",        sPopParams,
                                 "--calc-geoangles:0",   &m_bCalcGeoAngles);
    if (bOK) {  
        // checkConfig(&iArgC, &apArgV);
        bool bOnProbation = false;
        bool bIntermediateResult = true;
        iResult = m_pPR->getParams(iArgC, apArgV);
        if (*sHelpTopic != '\0') {
             setHelpTopic(sHelpTopic);
             printf("HelpTopic [%s]\n", m_sHelpTopic);
             iResult = 2;
        } else  if (bHelp) {
            setHelp(bHelp);
            iResult = 3;
        } else {
            if ((*sConfigIn != '\0') && (iResult == PARAMREADER_ERR_MANDATORY_MISSING)) {
                iResult = PARAMREADER_OK;
                bOnProbation = true;
            }
            if (iResult >= 0) {
                std::vector<std::string> vsOptions;
                m_pPR->collectOptions(vsOptions);
                LOG_DISP("%s (r%s) called with\n", apArgV[0], REVISION);
                for (uint j = 0; j < vsOptions.size(); j++) {
                    LOG_DISP("  %s\n", vsOptions[j].c_str());
                }
                LOG_DISP("-----------------------------------------\n");
        


                if (iResult > 0) {
                    std::vector<std::string> vUnknown;
                    int iNum = m_pPR->getUnknownParams(vUnknown);
                    LOG_WARNING("[SimParams::readOptions] %d unknown param%s:\n", iNum, (iNum>1)?"s":"");
                    for (int i = 0; i < iNum; i++) {
                        LOG_WARNING("  %s\n", vUnknown[i].c_str());
                    }
                }
                
           
            
                int iRes2=0;
                // m_pPR->display();
                // use config file if given
                if (*sConfigIn != '\0') {
                    LOG_STATUS("[SimParams::readOptions] using config file [%s]\n", sConfigIn);
                    int iResult2 = m_pPR->getParams(sConfigIn);
                    if (bOnProbation && (iResult2 < 0)) {
                        iResult = iResult2;
                    }
                }
                printf("numiters: %d\n", m_iNumIters);
                if (m_iNumIters < 0) {
                    iRes2   = -1;
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                    LOG_ERROR("Negative number of loops (%d)\n", m_iNumIters);
                }

                if (*sDataDirs != '\0') {
                    iRes2   = setDataDirs(sDataDirs);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setDataDir %d\n", iResult);
                
                if (*sSeed != '\0') {
                    iRes2   = setSeed(sSeed);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setSeed %d\n", iResult);
                
                if (*sGridFile != '\0') {
                    iRes2   = setGrid(sGridFile);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setGrid %d\n", iResult);

                if (*sGeoFile != '\0') {
                    iRes2   = setGeo(sGeoFile);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setGeo %d\n", iResult);

                if (*sClimateFile != '\0') {
                    iRes2   = setClimate(sClimateFile);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setClimate %d\n", iResult);

                if (*sVegFile != '\0') {
                    iRes2   = setVeg(sVegFile);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setVeg %d\n", iResult);

                if (*sPops != '\0') {
                    iRes2   = setPopList(sPops);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setPopList %d\n", iResult);

                if (*sPopParams != '\0') {
                    iRes2   = setPopParams(sPopParams);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setPopParams %d\n", iResult);

                if (*sOutputPrefix != '\0') {
                    iRes2   = setOutputPrefix(sOutputPrefix);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setOutputPrefix %d\n", iResult);

                if (*sOutputDir != '\0') {
                    iRes2   = setOutputDir(sOutputDir);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setOutputDir %d\n", iResult);

                if ((pEvents != NULL) && (*pEvents != '\0')) {
                    iRes2   = addEventTriggers(pEvents);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After addEventTriggers %d\n", iResult);

                if (*sConfigOut != '\0') {
                    iRes2   = setConfigOut(sConfigOut);
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setConfigOut %d\n", iResult);

            } else {
                if (iResult == PARAMREADER_ERR_MISSING_PARAM) {
                    LOG_ERROR("[SimParams::readOptions] Missing parameter for option [%s]\n", m_pPR->getBadArg().c_str());
                } else if (iResult == PARAMREADER_ERR_OPTION_SET) {
                    LOG_ERROR("[SimParams::readOptions] Error setting option [%s] to [%s]\n", m_pPR->getBadArg().c_str(), m_pPR->getBadVal().c_str());
                } else if (iResult == PARAMREADER_ERR_BAD_CONFIG_FILE) {
                    LOG_ERROR("[SimParams::readOptions] Config file [%s] does not exist\n", sConfigIn);
                } else {
                    iResult = -2;
                    char sTempErr[MAX_LINE];
                    sprintf(sTempErr, "[SimParams::readOptions] %sissing mandatory params. Required:\n", ((*sConfigIn != '\0')?"Bad config file or m":"M"));
                    LOG_ERROR("[SimParams::readOptions] %sissing mandatory params. Required:\n", ((*sConfigIn != '\0')?"Bad config file or m":"M"));
                    
                    char sPar[256];
                    std::vector<std::string> vMand;
                    int iNum = m_pPR->getMandatoryParams(vMand);
                    for (int i = 0; i < iNum; i++) {
                        sprintf(sPar, "  %s\n", vMand[i].c_str());
                        LOG_ERROR(sPar, "  %s\n", vMand[i].c_str());
                        strcat(sTempErr, sPar);
                    }
                }
            }
        }
    } else {
        LOG_ERROR("[SimParams::readOptions] Error setting options\n");
        iResult = -3;
    }
    
    //    LOG_STATUS("****************  readOptions exited with %d\n", iResult);
    
    if (iResult == 0) {
        LOG_STATUS("Succesfully read params\n");
        LOG_DISP("-------------------------------------\n");
        
        if (*m_sConfigOut != '\0') {
            char sConfPath[MAX_PATH];
            sprintf(sConfPath, "%s%s", m_sOutputDir, m_sConfigOut);
            LOG_STATUS("writing to config file [%s]\n", sConfPath);
            m_pPR->writeConfigFile(sConfPath, "--write-config");
        }

        
    } else if ((iResult != 2) && (iResult != 3)) {
        /* done in the destructor
           MessLogger::showLog(SHOW_ERROR | SHOW_WARNING);
        */
        iResult = -1;
    
    }

    fflush(stdout);
    return iResult;
   
}


//----------------------------------------------------------------------------
// exists
//  if pFile exists in current directory, 
//    true is returned and pExists is set to pFile
//  otherwise, if data dirs are given, and pFile exists in a data dir,
//    true is returned and pExists is set to DataDirX/pFile, 
//    where DataDirX is the first directory in the list containg 
//    a file named pFile 
//  otherwise,
//    false is returned  
//
bool SimParams::exists(const char *pFile, char *pExists) {
    struct stat statbuf;
    
    int iResult = stat(pFile, &statbuf);
    //    printf("Errore1: [%s]\n", strerror(iResult));
    if (iResult != 0)  {
        if ((pExists != NULL) && !m_vDataDirs.empty()) {
            char sTest[MAX_PATH];
            *pExists = '\0';
            for (uint i = 0; (*pExists ==  '\0') && (i < m_vDataDirs.size()); i++) {
                sprintf(sTest, "%s%s", m_vDataDirs[i].c_str(), pFile);
                iResult = stat(sTest, &statbuf);
                if (iResult == 0) {
                    strcpy(pExists, sTest);
                }
            }
        } else {
            // pFile doesn't exist, and we don't have a datadir
            // so that's it
        } 
    } else {
        strcpy(pExists, pFile);
    }
    if (iResult == 0) {
        //        printf("[exists] [%s] -> [%s]\n", pFile, pExists);
        //        LOG_STATUS("[exists] [%s] -> [%s]\n", pFile, pExists);
    } else {
        *pExists = '\0';
        printf("[exists] [%s] not found\n", pFile);
        LOG_STATUS("[exists] [%s] not found\n", pFile);
    }
    
    return (0 == iResult);
}


////////////////////////////////////////////////////////////////////////////
//// INPUT DATA HANDLING 
////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------
// setGrid
//  if File is QDF file, use GridReader to create Grid
//  otherwise try the old way (def file)
//
int SimParams::setGrid(const char *pFile) {
    int iResult = -1;
    char sExistingFile[MAX_PATH];
    if (m_pCG == NULL) {
        if (exists(pFile, sExistingFile)) {
            m_hFile = qdf_openFile(sExistingFile);
            if (m_hFile > 0) {
                iResult = setGrid(m_hFile);
            } else {
                printf("use old way\n");
                // do it the "old" way
                m_pLR = LineReader_std::createInstance(sExistingFile, "rt");
                if (m_pLR != NULL) {
                    iResult = setGridFromDefFile(sExistingFile);
                }
            }
            if (iResult == 0) {
                printf("Random seed: ");
                LOG_STATUS("Random seed: ");
                char sState[128];
                for (uint i = 0; i < STATE_SIZE/4; i++) {
                    *sState = '\0';                
                    for (uint j = 0; j < 4; j++) {
                        char sDig[16];
                        sprintf(sDig, " %08x", m_aulState[4*i+j]);
                        strcat(sState, sDig);
                    }
                    LOG_STATUS("    %s\n", sState);
                    printf("    %s\n", sState);
                }
                printf("\n");

                /* moved top setPops
                m_pPopFac = new PopulationFactory(m_pCG, m_iLayerSize, m_apIDG, m_aulState);
                */
            }
        } else {
            // err doesn't exist
            printf("doesn't exist [%s]\n", sExistingFile);
            LOG_ERROR("Gridfile [%s] doesn't exist\n", pFile);
        }
    } else {
        printf("Can't have second gridfile [%s]\n", pFile);
        LOG_ERROR("Can't have second gridfile [%s]\n", pFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setGrid
//  from QDF file
//
int SimParams::setGrid(hid_t hFile, bool bUpdate /* = false */) {
    int iResult = -1;
    GridReader *pGR = GridReader::createGridReader(hFile);
    if (pGR != NULL) {
        int iNumCells = 0;
        /*
        int iGridType = GRID_TYPE_NONE;
        int aiFormat[2];
        aiFormat[0] = -1;
        aiFormat[1] = -1;
        bool bPeriodic = false;
        */
        stringmap smSurfData;
        char sTime[32];
        // get the timestamp of the initial qdf file (grid)
        iResult = qdf_extractSAttribute(hFile,  ROOT_TIME_NAME, 31, sTime);
        if (iResult != 0) {
            printf("Couldn't read time attribute from grid file\n");
            LOG_STATUS("Couldn't read time attribute from grid file\n");
            iResult = 0;
        } else {
            if (strToNum(sTime, &m_fStartTime)) {
                iResult = 0;
                printf("Have timestamp %f\n", m_fStartTime);
            } else {
                printf("Timestamp not valid [%s]\n", sTime);
                iResult = -1;
            }
	}
        iResult = pGR->readAttributes(&iNumCells, smSurfData);

        if (bUpdate) {
            if (m_pCG == NULL) {
                printf("[setGrid] updating non-existent grid!!!\n");
                iResult = -1;
            }
            if ((uint)iNumCells != m_pCG->m_iNumCells) {
                printf("[setGrid] updating grid failed: different cell number!!!\n");
                iResult = -1;
            }
        }

        if (iResult == 0) {
            if (!bUpdate) {
                m_pCG = new SCellGrid(0, iNumCells, smSurfData);
                m_pCG->m_aCells = new SCell[iNumCells];
            } else {
                printf("[setGrid] updating grid...\n");
            }
            iResult = pGR->readCellData(m_pCG);
            if (iResult == 0) {
                /* moved to setPops
                if (!bUpdate) {
                    m_pPopLooper = new PopLooper(iNumCells);
                }
                */
                printf("[setGrid] Grid read successfully: %p\n", m_pCG);
                LOG_STATUS("[setGrid] Grid read successfully: %p\n", m_pCG);
                int iRes = setGeo(hFile, false, bUpdate);
                if (iRes == 0) {
                    iRes = setClimate(hFile, false, bUpdate);
                    if (iRes == 0) {
                        iRes = setVeg(hFile, false, bUpdate);
                        if (iRes == 0) {

                            // ok                            
                        } else {
                            printf("[setGrid] No Vegetation found in QDF\n");
                            LOG_STATUS("[setGrid] No Vegetation found in QDF\n");
                        }
                    } else {
                        printf("[setGrid] No Climate found in QDF\n");
                        LOG_STATUS("[setGrid] No Climate found in QDF\n");
                    }
                } else {
                    printf("[setGrid] No Geography found in QDF\n");
                    LOG_STATUS("[setGrid] No Geography found in QDF\n");
                }

                if (!bUpdate) {
                    iRes = setPops(hFile, false);
                    if (iRes == 0) {
                        // ok
                    } else {
                        printf("[setGrid] No Populations found in QDF\n");
                        LOG_STATUS("[setGrid] No Populations found in QDF\n");
                    }
                }

                if (!bUpdate) {
                    iRes = setStats(hFile);
                    if (iRes == 0) {
                        // ok
                    } else {
                        printf("[setGrid] No MoveStats found in QDF\n");
                        LOG_STATUS("[setGrid] No MoveStats found in QDF\n");
                    }
                }
 
            } else {
                printf("[setGrid] GridReader couldn't read data\n");
                LOG_ERROR("[setGrid] GridReader couldn't read data\n");
            }
        } else {
            printf("[setGrid] GridReader couldn't read attributes\n");
            LOG_ERROR("[setGrid] GridReader couldn't read attributes\n");
        }
        delete pGR;
    } else {
        printf("[setGrid] Couldn't create GridReader\n");
        LOG_ERROR("[setGrid] Couldn't create GridReader\n");
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setPopParams
//  set special population parameters
//
int SimParams::setPopParams(const char *pParams) {
    int iResult = 0;
    
    char *pParamsLoc = new char[strlen(pParams)+1];
    strcpy(pParamsLoc, pParams);
    
    std::map<std::string, std::string> mNamedParams;
 
    // split param string into <popname>/<param> pairs
    char *pPos1;
    char *pBlock = strtok_r(pParamsLoc, ",", &pPos1);
    while ((iResult == 0) && (pBlock != NULL)) {
        char *p0 = strchr(pBlock, ':');
        if (p0 != NULL) {
            *p0 = '\0';
            p0++;
            // save in map
            mNamedParams[pBlock] = p0;
        } else {
            iResult = -1;
        }
        pBlock = strtok_r(NULL, ",", &pPos1);
    }

    // loop through population names
    if (iResult == 0) {
        // now loop through populations and check if name is present
        for (uint j = 0; (iResult == 0) && (j < m_pPopLooper->getNumPops()); j++) {
            PopBase *pPop = m_pPopLooper->getPops()[j];
            std::map<std::string, std::string>::const_iterator it = mNamedParams.find(pPop->getSpeciesName());
            // pass parameter string to population
            if (it != mNamedParams.end()) {
                iResult = pPop->setParams(it->second.c_str());
            }
        }

    } else {
        LOG_ERROR("[setPopParams] Invalid parameter string: [%s]\n", pParams);
        printf("[setPopParams] Invalid parameter string: [%s]\n", pParams);
    }
    delete[] pParamsLoc;
    return iResult;
}


//----------------------------------------------------------------------------
// setGridFromDefFile
//  try the old way (def file)
//
int SimParams::setGridFromDefFile(const char *pFile) {
    int iResult = -1;
    
    GridFactory *pGF = new GridFactory(pFile);
    iResult = pGF->readDef();
    if (iResult == 0) {
        m_pCG = pGF->getCellGrid();
    // not here:    m_pPopLooper = new PopLooper(m_pCG->m_iNumCells);
    }
    delete pGF;
    return iResult;
}


//----------------------------------------------------------------------------
// setGeo
//  if File is QDF file, use GeoReader to create Geography
//  otherwise try the old way (def file)
//
int SimParams::setGeo(const char *pFile) {
    int iResult = -1;
    char sExistingFile[MAX_PATH];
    if (exists(pFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(sExistingFile);
        if (hFile > 0) {
            iResult = setGeo(hFile, true);
        } else {
            // read qmap
            // or
            // read qmap from deffile
            iResult = setGeoFromDefFile(sExistingFile);
        }
    } else {
        // err doesn't exist
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setGeo
//  from QDF file
//
int SimParams::setGeo(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
    int iResult = -1;
    
     GeoReader *pGR = GeoReader::createGeoReader(hFile);
     if (pGR != NULL) {
        uint iNumCells;
        int iMaxNeighbors;
        double dRadius;
        double dSeaLevel;
        
        iResult = pGR->readAttributes(&iNumCells, &iMaxNeighbors, &dRadius, &dSeaLevel);
        if (iResult == 0) {
            if (iNumCells == m_pCG->m_iNumCells) {
                
                if (!bUpdate) {
                    m_pGeo = new Geography(iNumCells, iMaxNeighbors, dRadius);
                }
                iResult = pGR->readData(m_pGeo);
                if (iResult == 0) {
                    m_pCG->setGeography(m_pGeo);
                    printf("[setGeo] GeoReader readData succeeded - Geo: %p, CG: %p!\n", m_pGeo, m_pCG);
                    LOG_STATUS("[setGeo] GeoReader readData succeeded : %p!\n", m_pGeo);
                } else {
                    printf("[setGeo] Couldn't read data\n");
                    LOG_ERROR("[setGeo] Couldn't read data\n");
                }
            } else {
                iResult = -2;
                printf("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iNumCells, iNumCells);
                LOG_ERROR("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iNumCells, iNumCells);
            }

        } else {
            printf("[setGeo] Couldn't read attributes\n");
            LOG_ERROR("[setGeo] Couldn't read attributes\n");
        }

         delete pGR;
     } else {
        printf("[setGeo] Couldn't create GeoReader\n");
        if (bRequired) {
            LOG_ERROR("[setGeo] Couldn't create GeoReader\n");
        }
     }

    if (m_bCalcGeoAngles && (iResult == 0)) {
        m_pGeo->calcAngles(m_pCG);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setGeoFromDefFile
//  try the old way (def file)
//
int SimParams::setGeoFromDefFile(const char *pFile) {
    int iResult = -1;
    LineReader *pLR = LineReader_std::createInstance(pFile, "rt");
    if (pLR != NULL) {
        iResult = 0;
        char *pLine = trim(m_pLR->getNextLine(GNL_IGNORE_ALL));
        char sQMap[256];
        *sQMap = '\0';
        while ((*sQMap != '\0') && (pLine != NULL) && !pLR->isEoF()) {
            
            if (strstr(pLine, "ALT QMAP") == 0) {
                strcpy(sQMap, trim(pLine+strlen("ALT QMAP")));
            } else if (strstr(pLine, "ALTITUDE QMAP") == 0) {
                strcpy(sQMap, trim(pLine+strlen("ALTITUDE QMAP")));
            } else{
                pLine = trim(m_pLR->getNextLine(GNL_IGNORE_ALL));
            }
        }
        if (*sQMap != '\0') {
            bool bInterpol = ((m_pCG->m_iType == GRID_TYPE_ICO) || (m_pCG->m_iType == GRID_TYPE_IEQ));
            ValReader *pVRAlt  = QMapUtils::createValReader(sQMap, bInterpol);  // prepare to read
            if (pVRAlt != NULL) {
                geonumber dRadius = 0;
                int iMaxNeigh=0; 
                switch (m_pCG->m_iType) {
                case GRID_TYPE_FLAT4:
                    iMaxNeigh = 4;
                    dRadius = 0;
                    break;
                case GRID_TYPE_FLAT6:
                    iMaxNeigh = 6;
                    dRadius = 0;
                    break;
                case GRID_TYPE_ICO:
                    iMaxNeigh = 6;
                    dRadius =6371.3;
                    break;
                case GRID_TYPE_IEQ:
                    iMaxNeigh = 6;
                    dRadius =6371.3;
                    break;
                }
                m_pGeo = new Geography(m_pCG->m_iNumCells, iMaxNeigh, dRadius);  // create geography
                m_pCG->setGeography(m_pGeo);
                LOG_STATUS("[setGeo]  GeoReader readD from def file succeeded [%s]\n", m_pGeo);
                delete pVRAlt;
            } else {
                printf("[setGeo] Couldn't open [%s] (no QMap?)\n", sQMap);
                LOG_ERROR("[setGeo] Couldn't open [%s] (no QMap?)\n", sQMap);
            }
        }
        delete pLR;
    } else {
        printf("[setGeo] Couldn't open [%s] to read geo\n", pFile);
        LOG_ERROR("[setGeo] Couldn't open [%s] to read geo\n", pFile);
    }
    if (m_bCalcGeoAngles && (iResult == 0)) {
        m_pGeo->calcAngles(m_pCG);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setClimate
//  if File is QDF file, use ClimateReader to create Climate
//  otherwise try the old way (def file)
//
int SimParams::setClimate(const char *pFile) {
    int iResult = -1;
    
    char sExistingFile[MAX_PATH];
    if (exists(pFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(sExistingFile);
        if (hFile > 0) {
            iResult = setClimate(hFile, true);
        } else {
            // set from def file (2 qmaps)
            iResult = setClimateFromDefFile(sExistingFile);
        }
    } else {
        // err doesn't exist
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setClimate
//  from QDF file
//
int SimParams::setClimate(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
    int iResult = -1;
    
    ClimateReader *pCR = ClimateReader::createClimateReader(hFile);
    if (pCR != NULL) {
        uint iNumCells;
        int iNumSeasons;
        bool bDynamic;
        
        iResult = pCR->readAttributes(&iNumCells, &iNumSeasons, &bDynamic);
        if (iResult == 0) {
            if (iNumCells == m_pCG->m_iNumCells) {

                if (!bUpdate) {
                    m_pCli = new Climate(iNumCells, (climatecount)iNumSeasons, m_pGeo);
                }
                iResult = pCR->readData(m_pCli);
                if (iResult == 0) {
                    m_pCG->setClimate(m_pCli);
                    printf("[setClimate] ClimateReader readData succeeded : %p!\n", m_pCli);
                    LOG_STATUS("[setClimate] ClimateReader readData succeeded : %p!\n", m_pCli);
                } else {
                    printf("[setClimate] Couldn't read climate data\n");
                    LOG_ERROR("[setClimate] Couldn't read climate data\n");
                    delete m_pCli;
                }
                //	    delete pCR;
            } else {
                iResult = -2;
                printf("[setClimate] Cell number mismatch: CG(%d) Cli(%d)\n", m_pCG->m_iNumCells, iNumCells);
                LOG_ERROR("[setClimate] Cell number mismatch: CG(%d) Cli(%d)\n", m_pCG->m_iNumCells, iNumCells);
            }
            
        } else {
            printf("[setClimate] Couldn't read attributes\n");
            LOG_ERROR("[setClimate] Couldn't read attributes\n");
        }
        
        
        delete pCR;
    } else {
        printf("[setClimate] Couldn't create ClimateReader\n");
        if (bRequired) {
            LOG_ERROR("[setClimate] Couldn't create ClimateReader\n");
        }
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// setClimateFromDefFile
//  try the old way (def file)
//
int SimParams::setClimateFromDefFile(const char *pFile) {
    int iResult = -1;
    

    return iResult;
}


//----------------------------------------------------------------------------
// setVeg
//  if File is QDF file, use VegReader to create Vegetation
//  otherwise try the old way (def file)
//
int SimParams::setVeg(const char *pFile) {
    int iResult = -1;
    
    char sExistingFile[MAX_PATH];
    if (exists(pFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(pFile);
        if (hFile > 0) {
            iResult = setVeg(hFile, true);
        } else {
            iResult = setVegFromDefFile(pFile);
        }
    } else {
        // err doesn't exist
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setVeg
//  use QDF file
//
int SimParams::setVeg(hid_t hFile, bool bRequired, bool bUpdate /* = false */) {
    int iResult = -1;
    
    VegReader *pVR = VegReader::createVegReader(hFile);
    if (pVR != NULL) {
        uint iNumCells;
        int iNumVegSpc;
        bool bDynamic;

        iResult = pVR->readAttributes(&iNumCells, &iNumVegSpc, &bDynamic);
        if (iResult == 0) {
            printf("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", iNumVegSpc, iNumCells);
            //@@            LOG_STATUS("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", iNumVegSpc, iNumCells);

            if (iNumCells == m_pCG->m_iNumCells) {
                if (!bUpdate) {
                    m_pVeg = new Vegetation(iNumCells, iNumVegSpc, m_pCG->m_pGeography, m_pCG->m_pClimate);
                }
              
                iResult = pVR->readData(m_pVeg);
                if (iResult == 0) {
                    m_pCG->setVegetation(m_pVeg);
                    printf("[setVeg] VegReader readData succeeded!\n");
                    LOG_STATUS("[setVeg] VegReader readData succeeded!\n");
                } else {
                    printf("[setVeg] Couldn't read data\n");
                    LOG_ERROR("[setVeg] Couldn't read data\n");
                }
            } else {
                iResult = -2;
                printf("[setVeg] Cell number mismatch: CG(%d) Veg(%d)\n", m_pCG->m_iNumCells, iNumCells);
                LOG_ERROR("[setVeg] Cell number mismatch: CG(%d) Veg(%d)\n", m_pCG->m_iNumCells, iNumCells);
            }
        } else {
            printf("[setVeg] Couldn't read attributes\n");
            LOG_ERROR("[setVeg] Couldn't read attributes\n");
        }


        delete pVR;
    } else {
        if (bRequired) {
            printf("[setVeg] Couldn't create VegReader\n");
            LOG_ERROR("[setVeg] Couldn't create VegReader\n");
        }
     }

    
    return iResult;
}


//----------------------------------------------------------------------------
// setVegFromDefFile
//   try the old way (def file)
//
int SimParams::setVegFromDefFile(const char *pFile) {
    int iResult = -1;
    if (m_pCG != NULL) {
        VegFactory *pVF = new VegFactory(pFile, m_pCG->m_iNumCells,  m_pCG->m_pGeography, m_pCG->m_pClimate);
        iResult = pVF->readDef();
        if (iResult == 0) {
            m_pVeg =pVF->getVeg();
            m_pCG->setVegetation(m_pVeg);
        } else {
            delete m_pVeg;
            m_pVeg = NULL;
        }
        delete pVF;
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setPoplist
//  assume comma-separated list of pop files
//
int SimParams::setPopList(char *pList) {
    int iResult = 0;

    char *pPos;
    char *p0 = strtok_r(pList, ",", &pPos);
    while ((iResult == 0) && (p0 != NULL)) {
        iResult = setPops(p0);
        p0 = strtok_r(NULL, ",", &pPos);
    }


    return iResult;
}


//----------------------------------------------------------------------------
// setPops
//  if file contains a ':' try <clsfile>:<datafile> (as in Pop2QDF)
//  otherwise try qdf and  use PopReader to create Populations
//
int SimParams::setPops(const char *pFile) {
    int iResult = -1;
    LOG_ERROR("[setPops] Looking at [%s]\n", pFile);
    
    // create poplooper and popfactory  if they don't exist
    // 
    if (m_pPopLooper == NULL) {
        m_pPopLooper = new PopLooper();
    }
    if (m_pPopFac == NULL) {
        m_pPopFac = new PopulationFactory(m_pCG, m_pPopLooper, m_iLayerSize, m_apIDG, m_aulState);
    }
    
    char sFile[MAX_PATH];
    strcpy(sFile, pFile);
    char *pData = strchr(sFile, ':');
    if (pData != NULL) {
        *pData = '\0';
        pData++;
        char sClsFile[MAX_PATH];
        char sDataFile[MAX_PATH];
        if (exists(sFile, sClsFile) && exists(pData, sDataFile)) {
            iResult = setPopsFromPopFile(sClsFile, sDataFile);
        } else {
            if (*sClsFile == '\0') {
                LOG_ERROR("[setPops] File [%s] does not exist\n", sFile);
            }
            if (*sDataFile == '\0') {
                LOG_ERROR("[setPops] File [%s] does not exist\n", pData);
            }
        }
    } else {
        //@@        LOG_STATUS("[setPops] pop file [%s] must be qdf\n", pFile);
        char sExistingFile[MAX_PATH];
        if (exists(pFile, sExistingFile)) {
            hid_t hFile = qdf_openFile(sExistingFile);
            if (hFile > 0) {
                iResult = setPops(hFile, true);
            } else {
                LOG_ERROR("[setPops] COuldn't open [%s] as QDF file\n", pFile);
            }
        } else {
            // file does not exist
            LOG_ERROR("[setPops] File [%s] does not exist\n", pFile);
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setPops
//  use QDF file
//  read all populations in the QDF file
//
int SimParams::setPops(hid_t hFile, bool bRequired) {
    int iResult = -1;
    PopReader *pPR = PopReader::create(hFile);
    if (pPR != NULL) {
        if (m_pPopFac != NULL) {
            const popinfolist &pil = pPR->getPopList();
            if (pil.size() > 0) {
                for (uint i = 0; i < pil.size(); i++) {
                    //@@                    LOG_STATUS("[setPops] pil: have class(%d) cname(%s) spcid(%d) spcname(%s) numc(%d)\n", 
                    //@@                    pil[i].m_iClassID, pil[i].m_sClassName, pil[i].m_iSpeciesID, pil[i].m_sSpeciesName, pil[i].m_iNumCells);
                    // since the class ids may vary, we use the string-version of createPopulation
                    // PopBase *pPop = m_pPopFac->createPopulation((spcid)pil[i].m_iClassID);
                    PopBase *pPop = m_pPopFac->createPopulation(pil[i].m_sClassName);
                    if (pPop != NULL) {
                        //@@                        LOG_STATUS("[setPops] have pop\n");
            
                        iResult = pPR->read(pPop, pil[i].m_sSpeciesName, m_pCG->m_iNumCells);
                        if (iResult == 0) {
                        
                            //@@                            LOG_STATUS("[setPops] have read pop\n");
                            m_pPopLooper->addPop(pPop);
                            printf("[setPops] successfully added Population: %s(%d): %ld agents\n", pPop->getSpeciesName(), pPop->getSpeciesID(), pPop->getNumAgentsTotal());
                            LOG_STATUS("[setPops] successfully added Population: %s(%d): %ld agents\n", pPop->getSpeciesName(), pPop->getSpeciesID(), pPop->getNumAgentsTotal());
                            if (m_iShuffle > 0) {
                                pPop->randomize(m_iShuffle);
                            }
                        } else {
                            if (iResult == POP_READER_ERR_CELL_MISMATCH) {
                                LOG_ERROR("[setPops] Cell number mismatch: CG(%d), pop[%s](%d)\n",  
                                          m_pCG->m_iNumCells, pil[i].m_sSpeciesName, pPop->getNumCells());
                            } else if (iResult == POP_READER_ERR_READ_SPECIES_DATA) {
                                LOG_ERROR("[setPops] Couldn't read species data for [%s]\n",  pil[i].m_sSpeciesName);
                            } else if (iResult == POP_READER_ERR_NO_SPECIES_GROUP) { 
                                LOG_ERROR("[setPops] No group for [%s] found in QDF file\n",  pil[i].m_sSpeciesName);
                            }
                            delete pPop;
                        
                        }
                    } else {
                        printf("[setPops] Couldn't create Population %s(%d) %s(%d)\n", 
                               pil[i].m_sSpeciesName, pil[i].m_iSpeciesID, pil[i].m_sClassName, pil[i].m_iClassID);
                        LOG_ERROR("[setPops] Couldn't create Population %s(%d) %s(%d)\n", 
                                  pil[i].m_sSpeciesName, pil[i].m_iSpeciesID, pil[i].m_sClassName, pil[i].m_iClassID);
                    }
                }
            } else {
                LOG_ERROR("[setPops] poplist size %zd\n", pil.size());
                iResult = -1;
            }

            delete pPR;
        } else {
            printf("[setPops] no PopulationFactory (this should not happen!)\n");
            LOG_ERROR("[setPops] no PopulationFactory (this should not happen!)\n");
        }
    } else {
        if (bRequired) {
            printf("[setPops] Couldn't create PopReader:(\n");
            LOG_ERROR("[setPops] Couldn't create PopReader\n");
        }
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// setPopsFromPopFile
//  read data from Pop file with real coordinates (lon, lat)
//  use GridInformation to build a surface to call findNode(lon, lat)  
//
int SimParams::setPopsFromPopFile(const char *pClsFile, const char *pDataFile) {
    int iResult = 0;
    // if m_pSurface doesnot exist
    if (m_pSurface == NULL) {
        //    create m_pSurface from CellGrid info
        iResult = createSurface();
    }

    if (iResult == 0) {
        iResult = -1;
        if (m_pPopFac != NULL) {
            PopBase *pPop = m_pPopFac->readPopulation(pClsFile);
            if (pPop != NULL) {
                printf("doing species [%s]\n", pPop->getSpeciesName());
                // read agent data
                iResult = readAgentData(pPop, pDataFile);
                if (iResult == 0) {
                    m_pPopLooper->addPop(pPop);
                    LOG_STATUS("[setPopsFromPopFile] successfully added Population: %s(%d)\n", pPop->getSpeciesName(), pPop->getSpeciesID());
                    if (m_iShuffle > 0) {
                        pPop->randomize(m_iShuffle);
                    }
                }
            } else {
                printf("[setPopsFromPopFile] Couldn't create data from [%s]\n", pClsFile);
                LOG_ERROR("[setPopsFromPopFile] Couldn't create data from [%s]\n", pClsFile);
            }

        } else {
            printf("[setPopsFromPopFile] no PopulationFactory (this should not happen!)\n");
            LOG_ERROR("[setPopsFromPopFile] no PopulationFactory (this should not happen!)\n");
        }

    } 

    // use  readAgentData from Pop2QDF to create population
    return iResult;
}


//----------------------------------------------------------------------------
// setStats
//  only for QDF files; use MoveStat
//
int SimParams::setStats(const char *pFile) {
    int iResult = -1;
    
    char sExistingFile[MAX_PATH];
    if (exists(pFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(sExistingFile);
        if (hFile > 0) {
            iResult = setStats(hFile);
        }
    } else {
        // file doesn't exist
        printf("[setStats] StatFile doesn't exist [%s]\n", pFile);
        LOG_ERROR("[setStats] StatFile doesn't exist [%s]\n", pFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// setStats
//  use QDF file
//
int SimParams::setStats(hid_t hFile) {
    int iResult = -1;
    
    MoveStatReader *pMSR = MoveStatReader::createMoveStatReader(hFile);
    if (pMSR != NULL) {
        uint iNumCells;

        iResult = pMSR->readAttributes(&iNumCells);
        if (iResult == 0) {
            if (iNumCells == m_pCG->m_iNumCells) {
                MoveStats *pMStat = new MoveStats(iNumCells);
                iResult = pMSR->readData(pMStat);
                if (iResult == 0) {
                    // pMStat is managed by the cell grid
                    m_pCG->setMoveStats(pMStat);
                    m_bHasMoveStats = true;
                    printf("MoveStatReader readData succeeded!\n");
                } else {
                    printf("[setStats] Couldn't read data\n");
                    LOG_ERROR("[setStats] Couldn't read data\n");
                    delete pMStat;
                }
            } else {
                iResult = -2;
                printf("[setStats] Cell number mismatch: CG(%d) Stat(%d)\n", m_pCG->m_iNumCells, iNumCells);
                LOG_ERROR("[setStats] Cell number mismatch: CG(%d) Stat(%d)\n", m_pCG->m_iNumCells, iNumCells);
            }
        } else {
            printf("[setStats] Couldn't read attributes\n");
            LOG_ERROR("C[setStats] ouldn't read attributes\n");
        }
        delete pMSR;
     }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createQDFTimeStamp
//  creates a string with a trigger time from a qdf file.
//  Currently only geo events (EVENT_ID_GEO) are supported, may be extended 
//  to climate and veg events later.
//  We expect the EventParameter to be a string of the form
//    "qdf:geofile.qdf"
//  
int SimParams::createQDFTimeStamp(int iEventType, char *pEventParams, char *pTimeStamp){
    int iResult = 0;
    
    // open qdf file    
    char sExistingFile[MAX_PATH];
    // later, other qdf files (climate etc) may also have to use this
    if ((iEventType = EVENT_ID_GEO) && strncmp(pEventParams, EVENT_PARAM_GEO_QDF, strlen(EVENT_PARAM_GEO_QDF)) == 0) {
        if (!isnan(m_fStartTime)) {
            // move to beginning of file name (+1: jump over ':')
            char  *pQDFFile = pEventParams + strlen(EVENT_PARAM_GEO_QDF)+1;
            if (exists(pQDFFile, sExistingFile)) {
                float fTimeStamp = fNaN;
                hid_t hFile = qdf_openFile(sExistingFile);
                if (hFile > 0) {
                    // read time attribute t
                    iResult = qdf_extractAttribute(hFile,  ROOT_TIME_NAME, 1, &fTimeStamp);
                    if (iResult != 0) {
                        LOG_ERROR("[createQDFTimeStamp] no time stamp found in QDF file [%s]", pQDFFile);
                        printf("[createQDFTimeStamp] no time stamp found in QDF file [%s]", pQDFFile);
                        iResult = -1;
                    }
                    qdf_closeFile(hFile);
                } else {
                    LOG_ERROR("[createQDFTimeStamp] couldn't open [%s] as a QDF file", pQDFFile);
                    printf("[createQDFTimeStamp] couldn't open [%s] as a QDF file", pQDFFile);
                    iResult = -1;
                }

                if (iResult == 0) {
                    // create step value
                    sprintf(pTimeStamp, "[%d]", (int)(fTimeStamp - m_fStartTime));
                }
                
            } else {
                LOG_ERROR("[createQDFTimeStamp] [%s] does not exist", pQDFFile);
                printf("[createQDFTimeStamp] [%s] does not exist", pQDFFile);
                iResult = -1;
            }
        } else {
            LOG_ERROR("[createQDFTimeStamp] Can't use qdf timestamp if no initial time stamp is given (grid file)\n");
            printf("[createQDFTimeStamp] Can't use qdf timestamp if no initial time stamp is given (grid file)\n");
            iResult = -1;
        }
    } else {
        LOG_ERROR("[createQDFTimeStamp] Expected '@' in definition for [%s]", pEventParams);
        printf("[createQDFTimeStamp] Expected '@' in definition for [%s]", pEventParams);
        iResult = -1;
    }
    

    return iResult;
}


//----------------------------------------------------------------------------
// addEventTriggers
//  write output
//
int SimParams::addEventTriggers(char *pEventDescription, float fCurTime){
    int iResult = 0;

    if (*pEventDescription != '\0') {
        char *pCtx1;
        char *pEvent = strtok_r(pEventDescription, ",", &pCtx1);
        while ((iResult == 0) && (pEvent != NULL)) {
            // find position of "@"
            char *pTriggerTimes = strchr(pEvent, '@');
            if (pTriggerTimes != NULL) {
                *pTriggerTimes = '\0';
                pTriggerTimes++;
            }
            
            // separate parameters and get event type
            char *pEventParams = strchr(pEvent, '|');
            if (pEventParams != NULL) {
                *pEventParams = '\0';
                pEventParams++;
                // pEvent points to type name
                // find event type
                int iEventType = EVENT_ID_NONE;
                if (strcasecmp(pEvent, EVENT_TYPE_WRITE) == 0) {
                    iEventType = EVENT_ID_WRITE;
                } else if (strcasecmp(pEvent, EVENT_TYPE_GEO) == 0) {
                    iEventType = EVENT_ID_GEO;
                } else if (strcasecmp(pEvent, EVENT_TYPE_CLIMATE) == 0) {
                    //                    iEventType = EVENT_ID_CLIMATE;
                    iEventType = EVENT_ID_NONE;
                } else if (strcasecmp(pEvent, EVENT_TYPE_COMM) == 0) {
                    iEventType = EVENT_ID_COMM;
                }

                // for qdf files only we may use the file's time attribute
                // however, this only works if the initial time value has beeen set (e.g. tim stamp of the grid file)
                char sFakeTime[256]; 
                if (pTriggerTimes == NULL) {
                    iResult = createQDFTimeStamp(iEventType, pEventParams, sFakeTime);
                    if (iResult == 0) {
                        pTriggerTimes = sFakeTime;
                    }
                }

                // now create the eventdata and the trigger
                if (iResult == 0) {
                    // pEventParams points to event data
                    EventData *pED = new EventData(iEventType, pEventParams);
                    if (pED != NULL) {
                        // add last iteration to triggers for write events
                        char *pNewInts = new char[strlen(pTriggerTimes)+65];
                        
                        if (iEventType == EVENT_ID_WRITE) {
                            char sLast[64];
                            sprintf(sLast, "+[%d]", m_iNumIters);
                            // 64 bytes should be enought to hold "+[<number>]"
                            sprintf(pNewInts, "%s+[%d]", pTriggerTimes, m_iNumIters);
                        } else {
                            sprintf(pNewInts, "%s", pTriggerTimes);
                        }
                        printf("Setting triggers [%s]\n", pNewInts);
                        Triggers *pT = Triggers::createTriggers(pNewInts);
                        if (pT != NULL) {
                            m_pEM->loadEventItem(pED, pT, fCurTime);
                        } else {
                            printf("[addEventTriggers] Bad trigger definition: [%s]", pTriggerTimes);
                            LOG_ERROR("[addEventTriggers] Bad trigger definition: [%s]\n", pTriggerTimes);
                            iResult = -1;
                        }
                        delete[] pNewInts;
                    } else {
                        printf("[addEventTriggers] Bad Event Data [%s]", pEventParams);
                        LOG_ERROR("[addEventTriggers] Bad Event Data [%s]", pEventParams);
                        iResult = -1;
                    }
                    pEvent = strtok_r(NULL, ",", &pCtx1);
                }
            } else {
                printf("[addEventTriggers] Expected '|' in Event description [%s]\n", pEvent);
                iResult = -1;
            }
        }
        // if everything went ok, start the event manager
        if (iResult == 0) {
            m_pEM->start();

            std::vector<std::string> vs;
            m_pEM->toString(vs);
            for (uint i = 0; i < vs.size(); ++i) {
                printf("%s\n", addEventName(vs[i]).c_str());
            }


        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// setOutputPrefix
//
int SimParams::setOutputPrefix(const char *pOutputPrefix) {
    strcpy(m_sOutputPrefix, pOutputPrefix);
    return 0;
}


//-----------------------------------------------------------------------------
// setOutputDir
//
int SimParams::setOutputDir(const char *pOutputDir) {
    strcpy(m_sOutputDir, pOutputDir);
    if (m_sOutputDir[strlen(m_sOutputDir)-1] != '/') {
        strcat(m_sOutputDir, "/");
    }
    int iResult = createDir(pOutputDir);
    if (iResult == 0) {
        printf("[setOutputDir] Created output directory[%s]\n", pOutputDir);
        LOG_STATUS("Created output directory[%s]\n", pOutputDir);
    } else if (iResult == 1) {
        iResult = 0;
        printf("[setOutputDir] Directory [%s] already exists\n", pOutputDir);
        LOG_WARNING("[setOutputDir] Directory [%s] already exists\n", pOutputDir);
    } else if (iResult == -2) {
        printf("[setOutputDir] [%s] exists, but is not a directory\n", pOutputDir);
        LOG_ERROR("[setOutputDir] [%s] exists, but is not a directory\n", pOutputDir);
    } else if (iResult == -3) {
        printf("[setOutputDir] couldn't stat name [%s]: %s\n", pOutputDir, strerror(errno));
        LOG_ERROR("[setOutputDir] couldn't stat name [%s]: %s\n", pOutputDir, strerror(errno));
    } else if (iResult == -4) {
        printf("[setOutputDir] couldn't create directory [%s]:%s\n", pOutputDir, strerror(errno));
        LOG_ERROR("[setOutputDir] couldn't create directory [%s]:%s\n", pOutputDir, strerror(errno));
    } else {
        printf("Couldn't create output directory [%s]\n", pOutputDir);
        LOG_ERROR("Couldn't create output directory [%s]\n", pOutputDir);
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// setDataDirs
//
int SimParams::setDataDirs(const char *pDataDirs) {
    char sDataDirs[MAX_PATH];
    strcpy(sDataDirs, pDataDirs);
    char *p = strtok(sDataDirs, ",;");
    LOG_STATUS("[setDataDirs]Data directories:\n");
    while (p != NULL) {
        std::string s = p;
        if (p[strlen(p)-1] != '/') {
            s += '/';
        }
        LOG_STATUS("[setDataDirs]  %s\n", s.c_str());
        m_vDataDirs.push_back(s);
        p = strtok(NULL, ",;");
    }
    return 0;
}


//-----------------------------------------------------------------------------
//  setConfigOut
// 
int SimParams::setConfigOut(const char *pConfigOut) {
    int iResult = 0;

    strcpy(m_sConfigOut, pConfigOut);

    return iResult;
}


//-----------------------------------------------------------------------------
//  splitSequence
// 
int splitSequence(char *sSequence, std::vector<uint32_t> &vulState) {
    int iResult = 0;
    char *p = strtok(sSequence, ",; ");
    while ((p != 0) && (iResult == 0)) {
        int iX = 0;
        char *pEnd;
        iX = (int)strtol(p, &pEnd, 16);
        if (*pEnd == '\0') {
            vulState.push_back(iX & 0xffffffff);
            p = strtok(NULL, ",; ");
        } else {
            LOG_ERROR("Invalid hexnumber given in seed sequence [%s]\n", p);
            iResult = -1;
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
//  setSeed
// 
int SimParams::setSeed(const char *pSeed) {
    int iResult = 0;
    if (strcasecmp(pSeed, SEED_RANDOM) == 0) {
        // create random seeds
        LOG_STATUS("[setSeed]Creating random seeds\n");
        time_t t = time(NULL);
        srand((int)t);
        for (uint i = 0; i < STATE_SIZE; i++) {
            m_aulState[i] = rand()%0xffffffff;
        }
        iResult = 0;
    } else {
        iResult = 0;
        std::vector<uint32_t> vulState;
        char sSeed[MAX_PATH];
        // if it contains commas, it is a sequence
        const char *p = strchr(pSeed, ',');
        if (p!= NULL) {
            LOG_STATUS("[setSeed] reading sequence from string\n");
            strcpy(sSeed, pSeed);
            iResult = splitSequence(sSeed, vulState);
        } else {
            // otherwise its a file
            iResult = -1;
            LOG_STATUS("[setSeed] reading sequence from file\n");
            // later: check if it is a file; read first line
            char sSeedFile[MAX_PATH];
            if (exists(pSeed, sSeedFile)) {
                LineReader *pLR =LineReader_std::createInstance(sSeedFile, "rt");
                if (pLR != NULL) {
                    char *p = pLR->getNextLine();
                    if (p != NULL) {
                        if (strstr(p, SEED_HEADER) == p) {
                            iResult = 0;
                            char *p2 = strchr(p, ':');
                            if (p2 != NULL) {
                                *p2 = '\0';
                                p2++;
                                LOG_STATUS("Ignoring seed destination [%s]\n", p2);
                                // pDest = p;
                            }
                            p = pLR->getNextLine();
                            while ((p != NULL) && (iResult == 0) && (strcasecmp(p, SEED_FOOTER) != 0)) {
                                printf("checking [%s]\n", p);
                                iResult = splitSequence(p, vulState);
                                p = pLR->getNextLine();
                            }
                            if (strcasecmp(p, SEED_FOOTER) == 0) {
                                iResult = 0;
                            } else {
                                if (p == NULL) {
                                    LOG_ERROR("Expected seed footer [%s]\n", SEED_FOOTER);
                                    iResult = -1;
                                }
                            }
                        } else {
                            LOG_ERROR("Expected seed header [%s]\n", SEED_HEADER);
                            iResult = -1;
                        }
                    } else {
                        LOG_ERROR("Seed file [%s] empty?\n", sSeedFile);
                        iResult = -1;
                    }
                    delete pLR;
                } else {
                    LOG_ERROR("Couldn't open seed file [%s]\n", sSeedFile);
                    iResult = -1;
                }

            } else {
                LOG_ERROR("Seed file [%s] does not exist\n", pSeed);
                iResult = -1;
            }
        }
        if (iResult == 0) {
            // line or file have a vecor of uints
            if (vulState.size() >= STATE_SIZE) {
                iResult = 0;
                if (vulState.size() > STATE_SIZE) {
                    LOG_STATUS("Got %zd hex numbers; only using first %d of them\n", vulState.size(), STATE_SIZE);
                }
                for (uint i = 0; i < STATE_SIZE; i++) {
                    m_aulState[i] = vulState[i];
                }
            } else {
                LOG_ERROR("Require %d hexnumbers but only got %zd\n", STATE_SIZE, vulState.size());
                iResult = -1;
            }
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// createDir
//
int SimParams::createDir(const char *pDirName) {
    int iResult = -1;
    // create dir if doesn't exist
    mode_t m = S_IRWXU | S_IRGRP | S_IROTH;  // 744
    struct stat statbuf;

    iResult = mkdir(pDirName, m);

    if (iResult != 0) {
        if (errno == EEXIST) {
            iResult = stat(pDirName, &statbuf);
            if (iResult == 0) {
                if (S_ISDIR(statbuf.st_mode)) {
                    iResult = 1;
                } else {
                    iResult = -2;
                }
            } else {
                iResult = -3;
            }
        } else {
            iResult = -4;
        }
    } else {
        iResult = 0;
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// createSurface
//
int SimParams::createSurface() {
    int iResult = -1;

    if (m_pSurface == NULL) {
        if (m_pCG != NULL) {
            stringmap &smSurf = m_pCG->m_smSurfaceData;
            const char *pSurfType = smSurf["SURF_TYPE"].c_str();

            if (strcmp(pSurfType, SURF_LATTICE) == 0) {
                int iLinks;
                const char *pLinks = smSurf[SURF_LTC_LINKS].c_str();
                if (strToNum(pLinks, &iLinks)) {
                    if ((iLinks == 4) || (iLinks == 6)) {
                        const char *pProjT = smSurf[SURF_LTC_PROJ_TYPE].c_str();
                        const char *pProjG = smSurf[SURF_LTC_PROJ_GRID].c_str();
                        if ((*pProjT != '\0') &&  (*pProjG != '\0')) {
                            Lattice *pLat = new Lattice();
                            iResult = pLat->create(iLinks, pProjT, pProjG);
                            m_pSurface = pLat;
                            if (iResult == 0) {
                                LOG_STATUS("[createSurface] Have lattice\n");
                            } else {
                                LOG_ERROR("[createSurface] cioulnd't create lattice\n");
                            }
                        } else {
                            LOG_ERROR("[createSurface] projection data incomplete:Type [%s], Grid[%s]\n", pProjT, pProjG);
                        }
                    } else {
                        LOG_ERROR("[createSurface] number of links must be 4 or 6 [%s]\n", pLinks);
                    }
                } else {
                    LOG_ERROR("[createSurface] number of links is not  a number [%s]\n", pLinks);
                }

            } else if (strcmp(pSurfType, SURF_EQSAHEDRON) == 0) {
                int iSubDivs = -1;
                const char *pSubDivs = smSurf[SURF_IEQ_SUBDIVS].c_str();
                if (strToNum(pSubDivs, &iSubDivs)) {
                    if (iSubDivs >= 0) {
                        EQsahedron *pEQ = EQsahedron::createInstance(iSubDivs, true, NULL);
                        if (pEQ != NULL) {
                            pEQ->relink();
                            m_pSurface = pEQ;
                            iResult = 0;
                            LOG_STATUS("[createSurface] Have EQsahedron\n");
                        }
                    } else {
                        LOG_ERROR("[createSurface] subdivs must be positive [%s]\n", pSubDivs);
                    }
                } else {
                    LOG_ERROR("[createSurface] subdivs is not a number [%s]\n", pSubDivs);
                }

            } else if (strcmp(pSurfType, SURF_ICOSAHEDRON) == 0) {
                int iSubLevel = -1;
                const char *pSubLevel = smSurf[SURF_ICO_SUBLEVEL].c_str();
                if (strToNum(pSubLevel, &iSubLevel)) {
                    if (iSubLevel >= 0) {
                        Icosahedron *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
                        pIco->setStrict(true);
                        pIco->setPreSel(false);
                        pIco->subdivide(iSubLevel);
                        m_pSurface = pIco;
                        LOG_STATUS("[createSurface] Have Icosahedron\n");
                    } else {
                        LOG_ERROR("[createSurface] sublevels must be positive [%s]\n", pSubLevel);
                    }
                } else {
                    LOG_ERROR("[createSurface] subdivs is not a number [%s]\n", pSubLevel);
                }
                    
            } else {
                LOG_ERROR("[createSurface] unknown surface type [%s]\n", pSurfType);
            }
            
        } else {
            LOG_ERROR("[createSurface] can't create surface without CellGrid data\n");
        }
    }
    return iResult;
};


//-----------------------------------------------------------------------------
// readAgentData
//   we expect lines containing longitude and latitude followed by a colon ':'
//   and  specific agent data
//
int SimParams::readAgentData(PopBase *pPop, const char *pAgentDataFile) {
    int iResult = 0;
    
    LineReader *pLR = LineReader_std::createInstance(pAgentDataFile, "rt");
    if (pLR != NULL) {
        while ((iResult == 0) && !pLR->isEoF()) {
            char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
            if (pLine != NULL) {

                double dLon;
                double dLat;
                
                int iNum = sscanf(pLine, "%lf %lf", &dLon, &dLat);
                if (iNum == 2) {
                    gridtype lNode = m_pSurface->findNode(dLon, dLat);
                    if (lNode >= 0) {
//                        printf("%f %f -> %d\n", dLon, dLat, lNode);
                        
                        char *pData = strchr(pLine, ':');
                        if (pData != NULL) {
                            *pData = '\0';
                            pData++;
                            // here we assume the index is equal to the ID
                            int iIndex = m_pCG->m_mIDIndexes[lNode];
                            iResult = pPop->addAgent(iIndex, pData);
                        } else {
                            printf("[readAgentData] No colon ':' in line:[%s]\n", pLine);
                            LOG_ERROR("[readAgentData] No colon ':' in line:[%s]\n", pLine);
                            iResult = -1;
                        }
                    } else {
                        printf("[readAgentData] coordinates (%f, %f) could not be mapped to a node\n", dLon ,dLat);
                        LOG_WARNING("[readAgentData] coordinates (%f, %f) could not be mapped to a node\n", dLon ,dLat);
                    }
                } else {
                    iResult = -1;
                    printf("[readAgentData] Couldn't extract Lon, Lat from [%s]\n", pLine);
                    LOG_ERROR("[readAgentData] Couldn't extract Lon, Lat from [%s]\n", pLine);
                }
            }
        }

        delete pLR;
    } else {
        printf("[readAgentDataCouldn't open [%s] for reading\n", pAgentDataFile);
        LOG_ERROR("[readAgentData] Couldn't open [%s] for reading\n", pAgentDataFile);
    }
    return iResult;
}


/*********************************
Help
*********************************/


//-----------------------------------------------------------------------------
// printHeaderLine
//
void printHeaderLine(int iL, const char *pTopic) {

   
    char sDash[256];
    memset(sDash, '-', iL);
    sDash[iL] = '\0';
    char sDashi[256];
    *sDashi = '\0';    
    if (*pTopic != '\0') {
        char sTemp1[256];
        sprintf(sTemp1, " Help for topic: %s ",  pTopic);
        size_t i1 = strlen(sTemp1);
        char sTemp2[256];
        sprintf(sTemp2, " Help for topic: %s%s%s%s ",  BOLDBLUE, pTopic, OFF, BOLD);
        size_t i2 = strlen(sTemp2);
        size_t iPos = (iL-i1)/2;
        memset(sDashi, '-', (i2-i1));
        sDash[i2-i1] = '\0';
        strcat(sDash, sDashi);
        memcpy(sDash+iPos, sTemp2, i2);
    }
    printf("\n%s%s%s%s\n\n", BOLD, sDash, sDashi, OFF);
}


//-----------------------------------------------------------------------------
// showTopicHelp
//
void SimParams::showTopicHelp(const char *pTopic) {
    bool bAll = strcmp(pTopic, "all") == 0;
    bool bFound = false;

    int iL = 80;
    //-- topic "help"
    if (bAll || (strcmp(pTopic, "help") == 0)) {
        printHeaderLine(iL, "help");
        printf("  %s--help=<topic>%s    show detailed help for specified topic\n", BOLDBLUE, OFF);
        printf("    Use 'all' to see info for all options.\n");
        bFound = true;
    }

    //-- topic "log-file"
    if (bAll || (strcmp(pTopic, "log-file") == 0)) {
        printHeaderLine(iL, "log-file");
        printf("  %s--log-file=<filename>%s    set name of logfile (default: \"%s\")\n", BOLDBLUE, OFF, DEF_LOG_FILE);
        bFound = true;
    }

    //-- topic "grid"
    if (bAll || (strcmp(pTopic, "grid") == 0)) {
        printHeaderLine(iL, "grid");
        printf("  %s--grid=<grid-file>%s    set grid file (required option)\n", BOLDBLUE, OFF);
        printf("    grid-file is a QDF file which may also contain geography, climate, \n");
        printf("    and vegetation data.\n");
        printf("    In this case the corresponding geo, cilmate, or veg options can be omitted.\n");
        bFound = true;
    }

    //-- topic "num-iters"
    if (bAll || (strcmp(pTopic, "num-iters") == 0)) {
        printHeaderLine(iL, "num-iters");
        printf("  %s--num-iters=<iters>%s    set number of iterations (required option)\n", BOLDBLUE, OFF);
        bFound = true;
    } 

    //-- topic "geo"
    if (bAll || (strcmp(pTopic, "geo") == 0)) {
        printHeaderLine(iL, "geo");
        printf("  %s--geo=<geo-file>%s    set geography file\n", BOLDBLUE, OFF);
        printf("    geo-file is a QDF-file containing geography data.\n");
        printf("    It must contain data for the same number of cells as the grid file\n");
        bFound = true;
    } 

    //-- topic "climate"
    if (bAll || (strcmp(pTopic, "climate") == 0)) {
        printHeaderLine(iL, "climate");
        printf("  %s--climate=<climate-file>%s    set climate file\n", BOLDBLUE, OFF);
        printf("    climate-file is a QDF-file containing climate data.\n");
        printf("    It must contain data for the same number of cells as the grid file\n");
        bFound = true;
    } 

    //-- topic "veg"
    if (bAll || (strcmp(pTopic, "veg") == 0)) {
        printHeaderLine(iL, "veg");
        printf("  %s--veg=<veg-file>%s    set vegetation file\n", BOLDBLUE, OFF);
        printf("    veg-file is a QDF-file containing vegetation data.\n");
        printf("    It must contain data for the same number of cells as the grid file\n");
        bFound = true;
    } 

    //-- topic "pops"
    if (bAll || (strcmp(pTopic, "pops") == 0)) {
        printHeaderLine(iL, "pops");
        printf("  %s--pops=<pop-list>%s    set populations data\n", BOLDBLUE, OFF);
        printf("    pop-list is a comma-separated list of QDF-files, each containing population data.\n");
        printf("    It must contain data for the same number of cells as the grid file\n");
        printf("    Alternatively you may use\n");
        printf("      --pops=<cls-file>:<data-file>\n");
        printf("    where cls-file is a class definition file,\n");
        printf("    and data-file is a file containing agent data.\n");
        bFound = true;
    } 

    //-- topic "output-prefix"
    if (bAll || (strcmp(pTopic, "output-prefix") == 0)) {
        printHeaderLine(iL, "output-prefix");
        printf("  %s--output-prefix=<prefix>%s    prefix for output files (default: \"%s\")\n", BOLDBLUE, OFF, DEF_OUT_PREFIX);    
        printf("    This prefix is prepended to all output files\n");
        bFound = true;
    } 

    //-- topic "output-dir"
    if (bAll || (strcmp(pTopic, "output-dir") == 0)) {
        printHeaderLine(iL, "output-dir");
        printf("  %s--output-dir=<dir-name>%s    set output directory (default: \"./\")\n", BOLDBLUE, OFF);
        printf("    If the output directory does not exist, it is created\n");
        bFound = true;
    }

    //-- topic "data-dirs"
    if (bAll || (strcmp(pTopic, "data-dirs") == 0)) {
        printHeaderLine(iL, "data-dirs");
        printf("  %s--data-dirs=<dir-names>%s    search-directories for input files (default: \"./\")\n", BOLDBLUE, OFF);
        printf("    dir-names is a comma-separated list of directories.\n");
        printf("    The order of the directores in the list defines the search order.\n");
        bFound = true;
    } 

    //-- topic "read-config"
    if (bAll || (strcmp(pTopic, "read-config") == 0)) {
        printHeaderLine(iL, "read-config");
        printf("  %s--read-config=<conf>%s    read options from configuration file.\n", BOLDBLUE, OFF);
        printf("    A configuration file must consist of lines\n");
        printf("    each of which is a option setting of the form\n");
        printf("      --<option-name>=value\n");
        printf("    Options passed as parameters to the application supercede the settings in\n");
        printf("    the config file\n");
        bFound = true;
    } 

    //-- topic "write-config"
    if (bAll || (strcmp(pTopic, "write-config") == 0)) {
        printHeaderLine(iL, "write-config");
        printf("  %s--write-config=<conf>%s    write configuration file.\n", BOLDBLUE, OFF);
        printf("    The configuration file created with this call can be used\n");
        printf("    with the option \"--read-config\"\n");
        bFound = true;
    }

    //-- topic "events"
    if (bAll || (strcmp(pTopic, "events") == 0)) {
        printHeaderLine(iL, "events");
        printf("  %s--events=<event-list>%s    set event list\n", BOLDBLUE, OFF);
        printf("    The event-list string is a comma-separated list of events:\n");
        printf("      %sevent-list%s       ::= \"%s'%s\" <event> [\"%s,%s\" <event>]*\"%s'%s\"\n", 
               BOLD, OFF, BOLD, OFF, BOLD, OFF, BOLD, OFF);
        printf("    The entire event list must be enclosed in quotes;\n");
        printf("    if the event list contains environment variables, use double quotes.\n");
        printf("      %sevent%s            ::= <event-type> \"%s|%s\" <event-params> \"%s@%s\" <event-times>\n", 
               BOLD, OFF, BOLD, OFF, BOLD, OFF);
        printf("      %sevent-type%s       ::= \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\"\n", BOLD, OFF, 
               BOLD, EVENT_TYPE_WRITE, OFF, 
               BOLD, EVENT_TYPE_GEO, OFF, 
               BOLD, EVENT_TYPE_CLIMATE, OFF, 
               BOLD, EVENT_TYPE_COMM, OFF);
        printf("    <event-times> defines at which times the event should be triggered.\n"); 
        printf("      %sevent-times%s      ::= <full-trigger> [ \"%s+%s\" <full-trigger>]*\n", 
               BOLD, OFF, BOLD, OFF);
        printf("      %sfull-trigger%s     ::= <normal-trigger> | <point-trigger>\n", BOLD, OFF);
        printf("      %snormal-trigger%s   ::= [<trigger-interval>] <step-size>\n", BOLD, OFF);
        printf("      %strigger-interval%s ::= \"%s[%s\"[<minval>] \"%s:%s\" [<maxval>] \"%s]%s\"\n", 
               BOLD, OFF, BOLD, OFF, BOLD, OFF, BOLD, OFF);
        printf("    This form causes events to be triggered at time <minval>+k*<step-size>, for\n");
        printf("    k = 0 ... (<maxval>-<minval>)/<step-size>.\n");
        printf("    If <minval> is omitted, it is set to fNegInf,\n");
        printf("    if <maxval> is omitted, it is set to fPosInf.\n");
        printf("    If <trigger-interval> is omitted, it is set to [fNegInf:fPosInf].\n");
        printf("      %spoint-trigger%s    ::= \"%s[%s\"<time>\"%s]%s\"\n", BOLD, OFF, BOLD, OFF, BOLD, OFF);
        printf("    This form causes a single event at the specified time.\n");
        printf("      Example:\n");
        printf("         20+[305:]500+[250:600]10+[37]\n");
        printf("         fire events every 20 steps,\n");
        printf("         and every 500 steps starting from step 305,\n");
        printf("         and every 10 steps between steps 250 and 600,\n");
        printf("         and a single event at step 37.\n");
        printf("    The event parameters differ for the event types:\n");
        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_WRITE, OFF);
        printf("      %sevent-params%s   ::= \"%s%s%s\" | \"%s%s%s\"   | \"%s%s%s \" |\n", BOLD, OFF, 
               BOLD, EVENT_PARAM_WRITE_GRID, OFF, 
               BOLD, EVENT_PARAM_WRITE_GEO, OFF, 
               BOLD, EVENT_PARAM_WRITE_CLIMATE, OFF);
        printf("                         \"%s%s%s\"  | \"%s%s%s\" | \"%s%s:%s\"<speciesname>\n", 
               BOLD, EVENT_PARAM_WRITE_VEG, OFF, 
               BOLD, EVENT_PARAM_WRITE_STATS, OFF, 
               BOLD, EVENT_PARAM_WRITE_POP, OFF);
        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_GEO, OFF); 
        printf("      %sevent-params%s   ::= \"%s%s%s:\"<altitude-qmap> | \"%s%s%s:\"<sea-level> | \n", BOLD, OFF, 
               BOLD, EVENT_PARAM_GEO_ALT, OFF, 
               BOLD, EVENT_PARAM_GEO_SEA, OFF);
        printf("                         \"%s%s%s:\"<geo/climate/veg qdf>\n",
               BOLD, EVENT_PARAM_GEO_QDF, OFF);
        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_CLIMATE, OFF); 
        printf("      %sevent-params%s   ::= <climate-file>\n", BOLD, OFF);
        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_COMM, OFF);
        printf("      %sevent-params%s   ::= <cmd-file>\n", BOLD, OFF);
        printf("      The format of <cmd-file>\n");
        printf("        cmd-file  ::= <command>*\n");
        printf("        command   ::= <iter_cmd> | <event>\n");
        printf("        iter_cmd  ::= \"%sSET ITERS%s\" <num_iters>\n", BOLD, OFF);
        printf("        event     : any event description; see definition above\n");
        printf("      If the <cmd-file> has chaned since the last triggered time the contents will be executed:\n");
        printf("        iter_cmd:  set new iteration limit\n");
        printf("        event:     add event to event manager's list\n");
        printf("    Full example:\n");
        printf("      %s--events='geo|qdf:map120.qdf@[120],write|pop:GrassLover|grid@5+[20:30]1'%s\n", BOLD, OFF); 
        printf("    This loads a QDF file containing new geography, vegetation and climate data\n");
        printf("    at step 120, and writes a QDF file containing grid data\n");
        printf("    and population data for species \"GrassLover\" every 5 steps\n");
        printf("    and additiionally every step between steps 20 and 30.\n");
        bFound = true;
    } 

    //-- topic "layer-size"
    if (bAll || (strcmp(pTopic, "layer-size") == 0)) {
        printHeaderLine(iL, "layer-size");
        printf("  %s--layer-size=<size>%s    %s(technical)%s set layer size of data structures\n", BOLDBLUE, OFF, BOLDRED, OFF);
        printf("                                                     (default: %d)\n", LAYERSIZE);
        printf("    Only change this option if you know what you are doing\n");
        bFound = true;
    }

    //-- topic "shuffle"
    if (bAll || (strcmp(pTopic, "shuffle") == 0)) {
        printHeaderLine(iL, "shuffle");
        printf("  %s--shuffle=<num>%s    %s(technical)%s shift random generators' sequence by <num>\n", BOLDBLUE, OFF, BOLDRED, OFF);
        printf("    This is done by generating <num> random numbers on each random\n");
        printf("    number generator before the start of the simulation.\n");
        printf("    I.e., this option can be used to change the random number sequence\n");
        bFound = true;
    } 

    //-- topic "seed"
    if (bAll || (strcmp(pTopic, "seed") == 0)) {
        printHeaderLine(iL, "seed");
        printf("  %s--seed=<seedtype>%s    %s(technical)%s seed the random generators\n", BOLDBLUE, OFF, BOLDRED, OFF);
        printf("    %sseedtype%s   ::= \"%srandom%s\" | <sequence> | <seed-file>\n", BOLD, OFF, BOLD, OFF);
        printf("    %ssequence%s   ::= (<hexnumber> \"%s,%s\")*\n", BOLD, OFF, BOLD, OFF);
        printf("    %sseed-file%s  : name of a seed file\n", BOLD, OFF);
        printf("    If seedType is \"%srandom%s\", the seeds are filled with random numbers\n", BOLD, OFF);
        printf("    based on the current time\n");
        printf("    A sequence must be consist of %d comma-separated 8-digit hexnumbers.\n", STATE_SIZE);
        printf("    If sequence is given, it is passed directly to the random nuber generators.\n");
        printf("    A seed file in its simplest form is a text file containing\n");
        printf("    %d 8-digit hexnumbers on a line\n", STATE_SIZE);
        printf("    The general format of a seed file:\n");
        printf("      %sseed-file%s ::= <seed-def>*\n", BOLD, OFF);
        printf("      %sseed-def%s  ::= <header> <seed-line>* <footer>\n", BOLD, OFF);
        printf("      %sheader%s    ::= \"%s%s%s\"[:<dest>]\n", BOLD, OFF, BOLD, SEED_HEADER, OFF);
        printf("      %sseed-line%s ::= (<hexnumber> \"%s,%s\")*\n", BOLD, OFF, BOLD, OFF);
        printf("      %sdest%s      :   A string describing which RNG the sequence should be\n", BOLD, OFF);
        printf("                   passed to (%sNOT IMPLEMENTED YET%s)\n", BOLDRED, OFF);
        printf("      %sfooter%s    ::= \"%s%s%s\"\n", BOLD, OFF, BOLD, SEED_FOOTER, OFF);
        printf("    In total, %d 8-digit hex numbers must be given between header and footer\n", STATE_SIZE);
        printf("    Lines starting with \"%s#%s\" are ignored\n", BOLD, OFF);
        bFound = true;
    } 

    //-- topic "pop-params"
    if (bAll || (strcmp(pTopic, "pop-params") == 0)) {
        printHeaderLine(iL, "pop-params");
        printf("  %s--pop-params=<param-strings>%s    set special population parameters\n", BOLDBLUE, OFF);
        printf("    %sparam-strings%s   ::= <param-string> [\"%s,%s\" <param-string>]*\n", BOLD, OFF, BOLD, OFF);
        printf("    %sparam-string%s    ::= <popname> \"%s:%s\" <pop-param>\n", BOLD, OFF, BOLD, OFF);
        printf("    The  %spop-param%s strings are passed to the specified populations;\n", BOLD, OFF);
        printf("    the format of %spopparam%s is specific for the particular population\n", BOLD, OFF);
        bFound = true;
    } 
        

    //-- unknown topic
    if (!bFound) {
        printHeaderLine(iL, "");
        printf("  %s%s%s: %sUnknown topic%s\n", BOLDBLUE, pTopic, OFF, RED, OFF);
        printf("    select an option from this list\n");
        helpParams();
    }

     printHeaderLine(iL, "");
}


//----------------------------------------------------------------------------
// addEventName
//
std::string SimParams::addEventName(std::string &sEventString) {
    int iStart = sEventString.find('(');
    int iEnd = sEventString.find(')', iStart);
    std::string s1=sEventString.substr(iStart+1, iEnd-iStart-1);
    int iType = atoi(s1.c_str());
    const char *p = "(unknown)";
    switch (iType) {
    case EVENT_ID_WRITE:
        p = EVENT_TYPE_WRITE;
        break;
    case EVENT_ID_GEO:
        p = EVENT_TYPE_GEO;
        break;
    case EVENT_ID_CLIMATE:
        p = EVENT_TYPE_CLIMATE;
        break;
    case EVENT_ID_COMM:
        p = EVENT_TYPE_COMM;
        break;
    }
    return p + sEventString;
}
