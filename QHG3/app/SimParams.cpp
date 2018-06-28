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
#include "EventConsts.h"
#include "strutils.h"
#include "colors.h"
#include "MessLogger.h"
#include "WELL512.h"
#include "WELLUtils.h"
#include "ParamReader.h"
#include "QDFUtils.h"
/*
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapUtils.h"
**/

#include "GroupReader.h"
#include "SCell.h"
#include "SCellGrid.h"
#include "GridFactory.h"
#include "GridGroupReader.h"
#include "Geography.h"
#include "GeoGroupReader.h"
#include "Climate.h"
#include "ClimateGroupReader.h"
#include "Vegetation.h"
#include "VegGroupReader.h"
#include "MoveStats.h"
#include "MoveStatGroupReader.h"
#include "Navigation.h"
#include "NavGroupReader.h"

#include "IcoGridNodes.h"
#include "Surface.h"
#include "Icosahedron.h"
#include "EQsahedron.h"
#include "Lattice.h"

#include "VegFactory.h"

#include "PopBase.h"
#include "PopLooper.h"
#include "PopReader.h"
#include "PopulationFactory.h"
#include "StatusWriter.h"
#include "IDGen.h"

#include "SimParams.h"

#define LAYERSIZE 65536
#define SEED_RANDOM "random"
#define SEED_SEQ    "seq:"
#define SEED_FILE   "file:"
#define SEED_PHRASE "phrase:"
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
      m_hFile(H5P_DEFAULT),
      m_pLRGrid(NULL),
      m_pPR( new ParamReader()),       
      m_pCG(NULL),  
      m_pGeo(NULL), 
      m_pCli(NULL),
      m_pVeg(NULL),
      m_pNav(NULL),
      m_pPopLooper(NULL),
      m_apIDG(NULL),
      m_pSW(NULL),
      m_pEM(new EventManager()),
      m_bHasMoveStats(false),
      m_pSurface(NULL),
      m_pPopFac(NULL),
      m_bZipOutput(false),
      m_bResume(false),
      m_fStartTime(0) {

    strcpy(m_sOutputPrefix, DEF_OUT_PREFIX);
    strcpy(m_sOutputDir, "./");
    m_vDataDirs.clear();
    *m_sHelpTopic = '\0';
    m_bHelp = false;
    int iNumThreads =  omp_get_max_threads();
    // the IDGen are allocated and created (valid offset and step)
    // the real base and offset are set once the max id of all populations is known
    // (must be done here, sothey exust whne restores values are read fron populations)
    m_apIDG = new IDGen*[iNumThreads];
    for (int iT = 0; iT < iNumThreads; ++iT) {
        m_apIDG[iT] = new IDGen(0, iT, iNumThreads);
    }      
    
    // set deault state for WELLs
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
    if (m_pNav != NULL) {
        delete m_pNav;
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
    if (m_pLRGrid != NULL) {
        delete m_pLRGrid;
    }
    if (m_apIDG != NULL) {
        for (int iT = 0; iT < omp_get_max_threads(); iT++) {
            delete m_apIDG[iT];
        }
        delete[]  m_apIDG;
    }
    std::vector<eventdata*>::const_iterator it;
    for (it = m_vEvents.begin(); it != m_vEvents.end(); ++it) {
        delete *it;
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
    printf("  --veg=<veg-file>           set vegetation file\n");    
    printf("  --nav=<nav-file>           set navigation file\n");    
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
    printf("  --zip-output               use gzip to zip all output qdf files\n");
    printf("  --resume                   resume from previously dumped env and pop files\n");
    printf("  --dump-on-interrupt        set interrupt handler for Ctrl-C (SIG_INT): dump and exit\n");


}

#define ERRINFO(x,y) std::string((x))+std::string((y))

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
    char sNavFile[MAX_PATH];
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
    char sShuffles[MAX_PATH];
 
    *sHelpTopic    = '\0';
    *sGridFile     = '\0';
    *sPops         = '\0';
    *sGeoFile      = '\0';
    *sClimateFile  = '\0';
    *sVegFile      = '\0';
    *sNavFile      = '\0';
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
    *sShuffles     = '\0';

    bool bHelp = false;
    m_bCalcGeoAngles = false;
    m_bZipOutput = false;

    bool bOK = m_pPR->setOptions(24, 
                                 "-h:0",                  &bHelp,
                                 "--help:s",              sHelpTopic,
                                 "--log-file:s",          sDummyLog,
                                 "--grid:s!",             sGridFile,
                                 "--geo:s",               sGeoFile,
                                 "--climate:s",           sClimateFile,
                                 "--veg:s",               sVegFile,
                                 "--nav:s",               sNavFile,
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
                                 "--shuffle:s",           sShuffles,
                                 "--seed:s",              sSeed,
                                 "--pop-params:s",        sPopParams,
                                 "--calc-geoangles:0",   &m_bCalcGeoAngles,
                                 "--zip-output:0",       &m_bZipOutput,
                                 "--resume:0",           &m_bResume);
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
                if (iResult > 0) {
                    LOG_WARNING("ParamReader Warning:\n%s", m_pPR->getErrorMessage(iResult).c_str());
                    printf("ParamReader Warning:\n%s", m_pPR->getErrorMessage(iResult).c_str());

                    iResult = 0;
                }
                std::vector<std::string> vsOptions;
                m_pPR->collectOptions(vsOptions);
                LOG_DISP("%s (r%s) on %d threads called with\n", apArgV[0], REVISION, omp_get_max_threads());
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
                std::vector<std::string> vsErrorInfo;
                printf("numiters: %d\n", m_iNumIters);
                if (m_iNumIters < 0) {
                    iRes2   = -1;
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                    LOG_ERROR("Negative number of loops (%d)\n", m_iNumIters);
                }

                if (*sDataDirs != '\0') {
                    iRes2   = setDataDirs(sDataDirs);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setDataDirs");
                    }
                }
                if (bIntermediateResult) printf("After setDataDir %d\n", iResult);
                
                if (*sSeed != '\0') {
                    iRes2   = setSeed(sSeed);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setSeed");
                    }
                }
                if (bIntermediateResult) printf("After setSeed %d\n", iResult);

                if (*sShuffles != '\0') {
                    iRes2   = setShuffles(sShuffles);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setShuffles");
                    }
                }
                if (bIntermediateResult) printf("After setShuffles %d\n", iResult);
                
                if (*sGridFile != '\0') {
                    iRes2   = setGrid(sGridFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setGrid");
                    }
                }
                if (bIntermediateResult) printf("After setGrid %d\n", iResult);

                if (*sGeoFile != '\0') {
                    iRes2   = setGeo(sGeoFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setGeo");
                    }
                }
                if (bIntermediateResult) printf("After setGeo %d\n", iResult);

                if (*sClimateFile != '\0') {
                    iRes2   = setClimate(sClimateFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setClimate");
                    }
                }
                if (bIntermediateResult) printf("After setClimate %d\n", iResult);

                if (*sVegFile != '\0') {
                    iRes2   = setVeg(sVegFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setVeg");
                    }
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setVeg %d\n", iResult);

                if (*sNavFile != '\0') {
                    iRes2   = setNav(sNavFile);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setNav");
                    }
                    iResult = (iRes2 < iResult)?iRes2:iResult;
                }
                if (bIntermediateResult) printf("After setNav %d\n", iResult);

                if (*sPops != '\0') {
                    iRes2   = setPopList(sPops);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setPopList");
                    }
                }
                if (bIntermediateResult) printf("After setPopList %d\n", iResult);

                if (*sPopParams != '\0') {
                    iRes2   = setPopParams(sPopParams);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setPopParams");
                    }
                }
                if (bIntermediateResult) printf("After setPopParams %d\n", iResult);

                if (*sOutputPrefix != '\0') {
                    iRes2   = setOutputPrefix(sOutputPrefix);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setOutputPrefix");
                    }
                }
                if (bIntermediateResult) printf("After setOutputPrefix %d\n", iResult);

                if (*sOutputDir != '\0') {
                    iRes2   = setOutputDir(sOutputDir);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setOutputDir");
                    }
                }
                if (bIntermediateResult) printf("After setOutputDir %d\n", iResult);

                if ((pEvents != NULL) && (*pEvents != '\0')) {
                    iRes2   = addEventTriggers(pEvents);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("addEventTriggers");
                    }
                }
                if (bIntermediateResult) printf("After addEventTriggers %d\n", iResult);

                if (*sConfigOut != '\0') {
                    iRes2   = setConfigOut(sConfigOut);
                    if (iRes2 != 0) {
                        iResult = iRes2;
                        vsErrorInfo.push_back("setConfigOut");
                    }
                }
                if (bIntermediateResult) printf("After setConfigOut %d\n", iResult);

                if (iResult != 0) {
                    printf("[SimParams::readOptions] Errors in the following method%s:\n", (vsErrorInfo.size()!=1)?"s":"");
                    LOG_ERROR("[SimParams::readOptions] Errors in the following method%s:\n", (vsErrorInfo.size()!=1)?"s":"");
                    for (uint i = 0; i < vsErrorInfo.size(); i++) {
                        LOG_ERROR("  %s\n", vsErrorInfo[i].c_str());
                        printf("  %s\n", vsErrorInfo[i].c_str());
                    }
                }
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
            char sConfPath[2*MAX_PATH];
            sprintf(sConfPath, "%s%s", m_sOutputDir, m_sConfigOut);
            LOG_STATUS("writing to config file [%s]\n", sConfPath);
            // omit the option  "--write-config"
            m_pPR->writeConfigFile(sConfPath, "--write-config");
        }

        /*
        printf("Random seed:\n");
        LOG_STATUS("Random seed:\n");
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
        */
        printf("Layer Size: %d\n", m_iLayerSize);
        LOG_STATUS("Layer Size: %d\n", m_iLayerSize);
        
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
//  if File is QDF file, use GridGroupReader to create Grid
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
                m_pLRGrid = LineReader_std::createInstance(sExistingFile, "rt");
                if (m_pLRGrid != NULL) {
                    iResult = setGridFromDefFile(sExistingFile);
                }
            }
            if (iResult == 0) {

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
    GridGroupReader *pGR = GridGroupReader::createGridGroupReader(hFile);
    if (pGR != NULL) {
        /*
        int iGridType = GRID_TYPE_NONE;
        int aiFormat[2];
        aiFormat[0] = -1;
        aiFormat[1] = -1;
        bool bPeriodic = false;
        */
        GridAttributes gridatt;
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
        iResult = pGR->readAttributes(&gridatt);

        if (bUpdate) {
            if (m_pCG == NULL) {
                printf("[setGrid] updating non-existent grid!!!\n");
                iResult = -1;
            }
            if ((uint)gridatt.m_iNumCells != m_pCG->m_iNumCells) {
                printf("[setGrid] updating grid failed: different cell number!!!\n");
                iResult = -1;
            }
        }

        if (iResult == 0) {
            if (!bUpdate) {
                m_pCG = new SCellGrid(0, gridatt.m_iNumCells, gridatt.smData);
                m_pCG->m_aCells = new SCell[gridatt.m_iNumCells];
            } else {
                printf("[setGrid] updating grid...\n");
            }
            iResult = pGR->readData(m_pCG);
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
                    iRes = setPops(hFile, NULL, false); // NULL: read all pops, false: not required
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

                iRes = setNav(hFile, bUpdate);
                if (iRes == 0) {
                    // ok
                } else {
                    printf("[setGrid] No Navigation found in QDF\n");
                    LOG_STATUS("[setGrid] No Navigation found in QDF\n");
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
//  if File is QDF file, use GeoGroupReader to create Geography
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
    
     GeoGroupReader *pGR = GeoGroupReader::createGeoGroupReader(hFile);
     if (pGR != NULL) {
         GeoAttributes geoatt;
         iResult = pGR->readAttributes(&geoatt);
         if (iResult == 0) {
             if (geoatt.m_iMaxNeighbors == (uint)m_pCG->m_iConnectivity) {
                 if (geoatt.m_iNumCells == (uint)m_pCG->m_iNumCells) {
                     if (!bUpdate) {
                         m_pGeo = new Geography(geoatt.m_iNumCells, geoatt.m_iMaxNeighbors, geoatt.m_dRadius);
                     } else {
                         m_pGeo = m_pCG->m_pGeography;
                     }
                     iResult = pGR->readData(m_pGeo);
                     if (iResult == 0) {
                         if (!bUpdate) {
                             m_pCG->setGeography(m_pGeo);
                         }
                         printf("[setGeo] GeoReader readData succeeded - Geo: %p, CG: %p!\n", m_pGeo, m_pCG);
                         LOG_STATUS("[setGeo] GeoReader readData succeeded : %p!\n", m_pGeo);
                     } else {
                         printf("[setGeo] Couldn't read data\n");
                         LOG_ERROR("[setGeo] Couldn't read data\n");
                     }
                 } else {
                     iResult = -2;
                     printf("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iNumCells, geoatt.m_iNumCells);
                     LOG_ERROR("[setGeo] Cell number mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iNumCells, geoatt.m_iNumCells);
                 }
             } else {
                 iResult = -3;
                 printf("[setGeo] Connectivity mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iConnectivity, geoatt.m_iMaxNeighbors);
                 LOG_ERROR("[setGeo] Connectivity mismatch: CG(%d) Geo(%d)\n", m_pCG->m_iConnectivity, geoatt.m_iMaxNeighbors);
             }
             
         } else {
             printf("[setGeo] Couldn't read attributes\n");
             LOG_ERROR("[setGeo] Couldn't read attributes\n");
         }
         
         delete pGR;
     } else {
         printf("[setGeo] Couldn't create GeoGroupReader: did not find group [%s]\n", GEOGROUP_NAME);
         if (bRequired) {
             LOG_ERROR("[setGeo] Couldn't create GeoGroupReader: did not find group [%s]\n", GEOGROUP_NAME);
         }
     }

    if (m_bCalcGeoAngles && (iResult == 0)) {
        m_pGeo->calcAngles(m_pCG);
    }

    return iResult;
}


//----------------------------------------------------------------------------
// setGeoFromDefFile
//  [try the old way (def file)] we don't do def files for geo anymore
//
int SimParams::setGeoFromDefFile(const char *pFile) {
    int iResult = -1;

    return iResult;
}


//----------------------------------------------------------------------------
// setClimate
//  if File is QDF file, use ClimateGroupReader to create Climate
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
    
    ClimateGroupReader *pCR = ClimateGroupReader::createClimateGroupReader(hFile);
    if (pCR != NULL) {
        ClimateAttributes climatt;
        iResult = pCR->readAttributes(&climatt);
        if (iResult == 0) {
            if (climatt.m_iNumCells == m_pCG->m_iNumCells) {

                if (!bUpdate) {
                    m_pCli = new Climate(climatt.m_iNumCells, climatt.m_iNumSeasons, m_pGeo);
                } else {
                    m_pCli = m_pCG->m_pClimate;
                }
                
                iResult = pCR->readData(m_pCli);
                if (iResult == 0) {
                    if (!bUpdate) {
                        m_pCG->setClimate(m_pCli);
                    }
                    printf("[setClimate] ClimateReader readData succeeded : %p!\n", m_pCli);
                    LOG_STATUS("[setClimate] ClimateReader readData succeeded : %p!\n", m_pCli);
                } else {
                    printf("[setClimate] Couldn't read climate data\n");
                    LOG_ERROR("[setClimate] Couldn't read climate data\n");
                    delete m_pCli;
                }
               
            } else {
                iResult = -2;
                printf("[setClimate] Cell number mismatch: CG(%d) Cli(%d)\n", m_pCG->m_iNumCells, climatt.m_iNumCells);
                LOG_ERROR("[setClimate] Cell number mismatch: CG(%d) Cli(%d)\n", m_pCG->m_iNumCells, climatt.m_iNumCells);
            }
            
        } else {
            printf("[setClimate] Couldn't read attributes\n");
            LOG_ERROR("[setClimate] Couldn't read attributes\n");
        }
        
        
        delete pCR;
    } else {
        printf("[setClimate] Couldn't create ClimateGroupReader: did not find group [%s]\n", CLIGROUP_NAME);
        if (bRequired) {
            LOG_ERROR("[setClimate] Couldn't create ClimateGroupReader: did not find group [%s]\n", CLIGROUP_NAME);
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
//  if File is QDF file, use VegGroupReader to create Vegetation
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
    
    VegGroupReader *pVR = VegGroupReader::createVegGroupReader(hFile);
    if (pVR != NULL) {
        VegAttributes vegatt;
        iResult = pVR->readAttributes(&vegatt);
        if (iResult == 0) {
            //@@            printf("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", vegatt.m_iNumVegSpc, vegatt.m_iNumCells);
            //@@            LOG_STATUS("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", iNumVegSpc, iNumCells);

            if (vegatt.m_iNumCells == m_pCG->m_iNumCells) {
                if (!bUpdate) {
                    m_pVeg = new Vegetation(vegatt.m_iNumCells, vegatt.m_iNumVegSpc, m_pCG->m_pGeography, m_pCG->m_pClimate);
                } else {
                    m_pVeg = m_pCG->m_pVegetation;
                }
                iResult = pVR->readData(m_pVeg);
                if (iResult == 0) {
                    if (!bUpdate) {
                        m_pCG->setVegetation(m_pVeg);
                    }
                    printf("[setVeg] VegReader readData succeeded!\n");
                    LOG_STATUS("[setVeg] VegReader readData succeeded!\n");
                } else {
                    printf("[setVeg] Couldn't read data\n");
                    LOG_ERROR("[setVeg] Couldn't read data\n");
                }
            } else {
                iResult = -2;
                printf("[setVeg] Cell number mismatch: CG(%d) Veg(%d)\n", m_pCG->m_iNumCells, vegatt.m_iNumCells);
                LOG_ERROR("[setVeg] Cell number mismatch: CG(%d) Veg(%d)\n", m_pCG->m_iNumCells, vegatt.m_iNumCells);
            }
        } else {
            printf("[setVeg] Couldn't read attributes\n");
            LOG_ERROR("[setVeg] Couldn't read attributes\n");
        }


        delete pVR;
    } else {
        if (bRequired) {
            printf("[setVeg] Couldn't create VegGroupReader: did not find group [%s]\n", VEGGROUP_NAME);
            LOG_ERROR("[setVeg] Couldn't create VegGroupReader: did not find group [%s]\n", VEGGROUP_NAME);
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
// setNav
//  if File is QDF file, use NavGroupReader to create Navigation
//  otherwise fail
//
int SimParams::setNav(const char *pFile) {
    int iResult = -1;
    
    char sExistingFile[MAX_PATH];
    if (exists(pFile, sExistingFile)) {
        hid_t hFile = qdf_openFile(pFile);
        if (hFile > 0) {
            iResult = setNav(hFile, true);
        }
	qdf_closeFile(hFile);
    } else {
        // err doesn't exist
    }
    return iResult;
}



//----------------------------------------------------------------------------
// setNav
//  use QDF file
//
int SimParams::setNav(hid_t hFile, bool bUpdate) {
    int iResult = -1;
    
    NavGroupReader *pNR = NavGroupReader::createNavGroupReader(hFile);
    if (pNR != NULL) {
        NavAttributes navatt;
        iResult = pNR->readAttributes(&navatt);
        if (iResult == 0) {
            //@@            printf("[setNav] NavReader attributes read (numports: %d, numdests:%d, numdists:%d, sampledist:%f)\n", navatt.m_iNumPorts, navatt.m_iNumDests, navatt.m_iNumDists, navatt.m_dSampleDist);
            //@@            LOG_STATUS("[setVeg] VegReader attributes read (numspc: %d, numcells:%d)\n", iNumVegSpc, iNumCells);
            
            if (!bUpdate) {
                m_pNav = new Navigation();
            } else {
                m_pNav = m_pCG->m_pNavigation;
            }

            
            iResult = pNR->readData(m_pNav);
            if (iResult == 0) {
                if (!bUpdate) {
                    m_pCG->setNavigation(m_pNav);
                }
                printf("[setNav] NavGroupReader readData succeeded!\n");
                LOG_STATUS("[setNav] NavGroupReader readData succeeded!\n");
            } else {
                printf("[setNav] Couldn't read data\n");
                LOG_ERROR("[setNav] Couldn't read data\n");
            }
            
        } else {
            printf("[setNav] Couldn't read attributes\n");
            LOG_ERROR("[setNav] Couldn't read attributes\n");
        }
        
        
        delete pNR;
    } else {
        printf("[setNav] Couldn't create NavGroupReader: did not find group [%s]\n", NAVGROUP_NAME);
        LOG_WARNING("[setNav] Couldn't create NavGroupReader: did not find group [%s]\n", NAVGROUP_NAME);
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
    //    LOG_ERROR("[setPops] Looking at [%s]\n", pFile);
    
    // create poplooper and popfactory  if they don't exist
    // 
    if (m_pPopLooper == NULL) {
        m_pPopLooper = new PopLooper();
    }
    if (m_pPopFac == NULL) {
        m_pPopFac = new PopulationFactory(m_pCG, m_pPopLooper, m_iLayerSize, m_apIDG, m_aulState, m_aiSeeds);
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
                iResult = setPops(hFile, NULL, true);
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
//  if pPopName is NULL: read all  populations in the QDF file,
//  otherwise read population named pPopName
//
int SimParams::setPops(hid_t hFile, const char *pPopName, bool bRequired) {
    int iResult = -1;
    PopReader *pPR = PopReader::create(hFile);
    if (pPR != NULL) {
        if (m_pPopFac != NULL) {
            const popinfolist &pil = pPR->getPopList();
            if (pil.size() > 0) {
                for (uint i = 0; i < pil.size(); i++) {
                    
                    if ((pPopName == NULL) || (strcmp(pPopName, pil[i].m_sClassName) == 0)) {
                        PopBase *pPop = m_pPopFac->createPopulation(pil[i].m_sClassName);
                        if (pPop != NULL) {
                            //@@                        LOG_STATUS("[setPops] have pop\n");
                            
                            iResult = pPR->read(pPop, pil[i].m_sSpeciesName, m_pCG->m_iNumCells, m_bResume);
                            if (iResult == 0) {
                                
                                //@@                            LOG_STATUS("[setPops] have read pop\n");
                                m_pPopLooper->addPop(pPop);
                                printf("[setPops] successfully added Population: %s(%d): %ld agents\n", pPop->getSpeciesName(), pPop->getSpeciesID(), pPop->getNumAgentsTotal());
                                LOG_STATUS("[setPops] successfully added Population: %s(%d): %ld agents\n", pPop->getSpeciesName(), pPop->getSpeciesID(), pPop->getNumAgentsTotal());
                                if (!m_bResume) {
                                    if (m_aiSeeds[0] > 0) {
                                        printf("Randomizing(1) with %u\n", m_aiSeeds[0]);
                                        pPop->randomize(m_aiSeeds[0]);
                                    } else {
                                        printf("No seeds???\n");
                                    }
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
                    } else {
                        printf("[setPops] No population name given\n");
                        LOG_ERROR("[setPops] No population name given\n");
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
                    if (m_aiSeeds[0] > 0) {
                        printf("Randomizing(1) with %u\n", m_aiSeeds[0]);
                        pPop->randomize(m_aiSeeds[0]);
                    } else {
                        printf("No seeds???\n");        
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
    
    MoveStatGroupReader *pMSR = MoveStatGroupReader::createMoveStatGroupReader(hFile);
    if (pMSR != NULL) {
        MoveStatAttributes statatt;

        iResult = pMSR->readAttributes(&statatt);
        if (iResult == 0) {
            if (statatt.m_iNumCells == m_pCG->m_iNumCells) {
                MoveStats *pMStat = new MoveStats(statatt.m_iNumCells);
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
                printf("[setStats] Cell number mismatch: CG(%d) Stat(%d)\n", m_pCG->m_iNumCells, statatt.m_iNumCells);
                LOG_ERROR("[setStats] Cell number mismatch: CG(%d) Stat(%d)\n", m_pCG->m_iNumCells, statatt.m_iNumCells);
            }
        } else {
            printf("[setStats] Couldn't read attributes\n");
            LOG_ERROR("C[setStats] ouldn't read attributes\n");
        }
        delete pMSR;
    } else {
        printf("[setStats] Couldn't create MoveStatGroupReader: did not find group [%s]\n", MSTATGROUP_NAME);
        LOG_WARNING("[setStats] Couldn't create MoveStatGroupReader: did not find group [%s]\n", MSTATGROUP_NAME);
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// createQDFTimeStamp
//  creates a string with a trigger time from a qdf file.
//  Currently geo events (EVENT_ID_GEO), climate events (EVENT_ID_CLIMATE) and
//  environment events (EVENT_ID_ENV) are supported.
//  We expect the EventParameter to be a string of the form
//    "qdf:geofile.qdf"
//    "qdf:climatefile.qdf"
//    "type1+type2:envfile.qdf"
//  I.e., we expect only one ':' which must be followed by a qdf file name
int SimParams::createQDFTimeStamp(int iEventType, char *pEventParams, char *pTimeStamp){
    int iResult = 0;
    
    // open qdf file    
    char sExistingFile[MAX_PATH];
    // later, other qdf files (climate etc) may also have to use this
    if (((iEventType = EVENT_ID_GEO) && strncmp(pEventParams, EVENT_PARAM_GEO_QDF, strlen(EVENT_PARAM_GEO_QDF)) == 0) ||
        ((iEventType = EVENT_ID_CLIMATE) && strncmp(pEventParams, EVENT_PARAM_CLIMATE_QDF, strlen(EVENT_PARAM_CLIMATE_QDF)) == 0) ||
        ((iEventType = EVENT_ID_ENV) && strncmp(pEventParams, EVENT_PARAM_CLIMATE_QDF, strlen(EVENT_PARAM_CLIMATE_QDF)) == 0)) {

        if (!isnan(m_fStartTime)) {
            // move to beginning of file name (+1: jump over ':')
            char  *pQDFFile = pEventParams + strlen(EVENT_PARAM_GEO_QDF)+1;
            if (exists(pQDFFile, sExistingFile)) {
                float fTimeStamp = fNaN;
                hid_t hFile = qdf_openFile(sExistingFile);
                if (hFile > 0) {
                    // read time attribute t
                    char sTime[256];
                    iResult = qdf_extractSAttribute(hFile,  ROOT_TIME_NAME, 255, sTime);
                    if (iResult == 0) {

                        if (strToNum(sTime, &fTimeStamp)) {
                            iResult = 0;
                        } else {
                            LOG_ERROR("[createQDFTimeStamp] time stamp is not a number [%s]\n", sTime);
                            printf("[createQDFTimeStamp] time stamp is not a number [%s]\n", sTime);
                            iResult = -1;
                        }
                    } else {
                        LOG_ERROR("[createQDFTimeStamp] no time stamp found in QDF file [%s]\n", pQDFFile);
                        printf("[createQDFTimeStamp] no time stamp found in QDF file [%s]\n", pQDFFile);
                        iResult = -1;
                    }
                    qdf_closeFile(hFile);
                } else {
                    LOG_ERROR("[createQDFTimeStamp] couldn't open [%s] as a QDF file\n", pQDFFile);
                    printf("[createQDFTimeStamp] couldn't open [%s] as a QDF file\n", pQDFFile);
                    iResult = -1;
                }

                if (iResult == 0) {
                    // create step value
                    sprintf(pTimeStamp, "[%d]", (int)(fTimeStamp - m_fStartTime));
                }
                
            } else {
                LOG_ERROR("[createQDFTimeStamp] [%s] does not exist\n", pQDFFile);
                printf("[createQDFTimeStamp] [%s] does not exist\n", pQDFFile);
                iResult = -1;
            }
        } else {
            LOG_ERROR("[createQDFTimeStamp] Can't use qdf timestamp if no initial time stamp is given (grid file)\n");
            printf("[createQDFTimeStamp] Can't use qdf timestamp if no initial time stamp is given (grid file)\n");
            iResult = -1;
        }
    } else {
        LOG_ERROR("[createQDFTimeStamp] Expected '@' in definition for [%s]\n", pEventParams);
        printf("[createQDFTimeStamp] Expected '@' in definition for [%s]\n", pEventParams);
        iResult = -1;
    }
    

    return iResult;
}


//----------------------------------------------------------------------------
// readEventsFromString
//  
//
int SimParams::readEventFromString(char *pEventString) {
    int iResult = 0;

    char *pEventParams = strchr(pEventString, '|');
 
    char *pTriggerTimes = strchr(pEventString, '@');
    if (pTriggerTimes != NULL) {
        *pTriggerTimes = '\0';
        pTriggerTimes++;
    } else {
        printf("[readEventFromString] Bad event description: expected '@' after [%s]\n", pEventString);
        LOG_ERROR("[readEventFromString] Bad event description: expected '@' after [%s]\n", pEventString);
        iResult = -1;
    }
    
    if (pEventParams != NULL) {
        *pEventParams = '\0';
        pEventParams++;
    } else {
        printf("[readEventFromString] Bad event description: expected '|' in [%s]\n", pEventString);
        LOG_ERROR("[readEventFromString] Bad event description: expected '|' in [%s]\n", pEventString);
        iResult = -1;
    }
    
    if (iResult == 0) {
        if (strcasecmp(pEventString, EVENT_TYPE_FILE) == 0) {
            iResult = readEventsFromFile(pEventParams);
        } else {
            m_vEvents.push_back(new eventdata(pEventString, pEventParams, pTriggerTimes));
        }
    }

    
    return iResult;
}


//----------------------------------------------------------------------------
// readEventsFromFile
//  
//
int SimParams::readEventsFromFile(char *pEventFile) {
    int iResult = 0;

    LineReader *pLR = LineReader_std::createInstance(pEventFile, "rt");
    if (pLR != NULL) {

        char *pLine = pLR->getNextLine(GNL_IGNORE_ALL);
        while ((iResult == 0) && (pLine != NULL) && !pLR->isEoF()) {
            iResult = readEventFromString(trim(pLine));
            
            pLine = pLR->getNextLine(GNL_IGNORE_ALL);
        }

        delete pLR;
    } else {
        printf("[readEventsFromFile] Couldn't open file [%s]\n", pEventFile);
        LOG_ERROR("[readEventsFromFile] Couldn't open file [%s]\n", pEventFile);
        iResult = -1;
    }
    
    return iResult;
}


//----------------------------------------------------------------------------
// processEvents
//  
//
int SimParams::processEvents(float fCurTime) {
    int iResult = 0;
    
    std::vector<eventdata*>::const_iterator it;
    for (it = m_vEvents.begin(); it != m_vEvents.end(); ++it) {
        
            // find event type
        int iEventType = EVENT_ID_NONE;
        char *pEventType = (*it)->pEventType;
        if (strcasecmp(pEventType, EVENT_TYPE_WRITE) == 0) {
            iEventType = EVENT_ID_WRITE;
        } else if (strcasecmp(pEventType, EVENT_TYPE_ENV) == 0) {
            iEventType = EVENT_ID_ENV;
        } else if (strcasecmp(pEventType, EVENT_TYPE_ARR) == 0) {
            iEventType = EVENT_ID_ARR;
        } else if (strcasecmp(pEventType, EVENT_TYPE_POP) == 0) {
            iEventType = EVENT_ID_POP;
        } else if (strcasecmp(pEventType, EVENT_TYPE_CHECK) == 0) {
            iEventType = EVENT_ID_CHECK;
        } else if (strcasecmp(pEventType, EVENT_TYPE_COMM) == 0) {
            iEventType = EVENT_ID_COMM;
        } else if (strcasecmp(pEventType, EVENT_TYPE_DUMP) == 0) {
            iEventType = EVENT_ID_DUMP;
        } else if (strcasecmp(pEventType, EVENT_TYPE_USER) == 0) {
            iEventType = EVENT_ID_USER;
        } else {
            printf("[processEvents] Unknown event type: [%s]\n", pEventType);
            LOG_ERROR("[processEvents] Unknown event type: [%s]\n", pEventType);
            iResult = -1;
        }

        // now create the eventdata and the trigger
        if (iResult == 0) {
            // pEventParams points to event data

            EventData *pED = new EventData(iEventType, (*it)->pEventParams);
            if (pED != NULL) {
                const char *pEventTimes = (*it)->pEventTimes;
                // add last iteration to triggers for write events
                // 64 bytes should be enought to hold "+[<number>]"
                char *pNewInts = new char[strlen(pEventTimes)+65];
                
                if (iEventType == EVENT_ID_WRITE) {
                    // force a write at the last time
                    sprintf(pNewInts, "%s+[0]+[%d]", pEventTimes, m_iNumIters);
                } else {
                    sprintf(pNewInts, "%s", pEventTimes);
                }
                
                printf("Setting triggers [%s]\n", pNewInts);
                Triggers *pT = Triggers::createTriggers(pNewInts, m_iNumIters);
                if (pT != NULL) {
                    m_pEM->loadEventItem(pED, pT, fCurTime);
                    
                } else {
                    printf("[processEvents] Bad trigger definition: [%s]\n", pEventTimes);
                    LOG_ERROR("[processEvents] Bad trigger definition: [%s]\n", pEventTimes);
                    iResult = -1;
                }
                delete[] pNewInts;
            } else {
                printf("[processEvents] Bad Event Data [%s]\n", (*it)->pEventParams);
                LOG_ERROR("[processEvents] Bad Event Data [%s]\n", (*it)->pEventParams);
                iResult = -1;
            }
            
        }
    }

    if (iResult == 0) {
        
        printf("[processEvents] Flat event list\n");
        LOG_STATUS("[processEvents] Flat event list\n");
        
        for (it = m_vEvents.begin(); it != m_vEvents.end(); ++it) {
            printf("  %s|%s@%s\n", (*it)->pEventType, (*it)->pEventParams, (*it)->pEventTimes);
            LOG_STATUS("  %s|%s@%s\n", (*it)->pEventType, (*it)->pEventParams, (*it)->pEventTimes);
        }
    }

    return iResult;
}



//----------------------------------------------------------------------------
// addEventTriggers
//  write output
//
int SimParams::addEventTriggers(char *pEventDescription, float fCurTime){
    int iResult = 0;
    // vEvents: vector of std::string
    // split events string and loop through events
    //   if event is 'file'
    //      readEvents(arg, vEvents)
    //   else
    //      vEvents.push_back(event string)

    m_vEvents.clear();

    if (*pEventDescription != '\0') {
        char *pCtx1;
        char *pEvent = strtok_r(pEventDescription, ",", &pCtx1);
        while ((iResult == 0) && (pEvent != NULL)) {
            printf("event: [%s]\n", pEvent);
            iResult = readEventFromString(pEvent);

            pEvent = strtok_r(NULL, ",", &pCtx1);

        }

        if (iResult == 0) {
            iResult = processEvents(fCurTime);
        }


    }

    if (iResult == 0) {
        m_pEM->start();
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
//  readSeed
//    creates a 16 array of long from an arbitrary string
//
int SimParams::readSeed(const char *pFileName, std::vector<uint32_t> &vulState) {
    int iResult = -1;

    // later: check if it is a file; read first line
    char sSeedFile[MAX_PATH];
    if (exists(pFileName, sSeedFile)) {
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
                        //@@printf("checking [%s]\n", p);
                        iResult = stringToSeed(p, vulState);
                        if (iResult == 0) {
                            p = pLR->getNextLine();
                        }
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
        LOG_ERROR("File [%s] does not exist\n", pFileName);
        iResult = -1;
    }
    return iResult;
}




//-----------------------------------------------------------------------------
//  setSeed
// 
int SimParams::setSeed(const char *pSeed) {
    int iResult = -1;

    if (!m_bResume) {
        std::vector<uint32_t> vulState;
    
        if (strcasecmp(pSeed, SEED_RANDOM) == 0) {
            // create random seeds
            LOG_STATUS("[setSeed]Creating random seeds\n");
            time_t t = time(NULL);
            iResult = randomSeed((int)t, vulState);
            if ((iResult == 0) && (vulState.size() >= STATE_SIZE)) {
                LOG_STATUS("[setSeed] created random seed sequence with rand()\n");
            }
            
        } else if (strncasecmp(pSeed, SEED_SEQ, strlen(SEED_SEQ)) == 0) {
            pSeed += strlen(SEED_SEQ);
            
            // let's see if it is a sequence
            char sSeed[MAX_PATH];
            strcpy(sSeed, pSeed);
            LOG_STATUS("[setSeed] trying to read seed from string");
            iResult = stringToSeed(sSeed, vulState);
            if ((iResult == 0) && (vulState.size() >= STATE_SIZE)) {
                LOG_STATUS("[setSeed] read seed sequence from string\n");
            }
            
        } else if (strncasecmp(pSeed, SEED_FILE, strlen(SEED_FILE)) == 0) {
            pSeed += strlen(SEED_FILE);
            
            // maybe it is a file?
            LOG_STATUS("[setSeed] trying to read seed from file");
            iResult = readSeed(pSeed, vulState);
            if ((iResult == 0) && (vulState.size() >= STATE_SIZE)) {
                LOG_STATUS("[setSeed] read seed sequence from file\n");
            }
            
        } else if (strncasecmp(pSeed, SEED_PHRASE, strlen(SEED_PHRASE)) == 0) {
            pSeed += strlen(SEED_PHRASE);
            
            // ok - let's use it as pass phrase
            LOG_STATUS("Use [%s] as phrase to create seed\n", pSeed);
            iResult = phraseToSeed(pSeed, vulState);
            if ((iResult == 0) && (vulState.size() >= STATE_SIZE)) {
                LOG_STATUS("[setSeed] used passphrase [%s] to create seed sequence\n", pSeed);
            }
        } else {
            printf("Unknown seed type [%s]\n", pSeed);
            LOG_ERROR("Unknown seed type [%s]\n", pSeed);
        }


        if (iResult == 0) {
            // line or file have a vecor of uints
            if (vulState.size() >= STATE_SIZE) {
                iResult = 0;
                if (vulState.size() > STATE_SIZE) {
                    LOG_WARNING("Got %zd hex numbers; only using first %d of them\n", vulState.size(), STATE_SIZE);
                }
                for (uint i = 0; i < STATE_SIZE; i++) {
                    m_aulState[i] = vulState[i];
                }
            } else {
                LOG_ERROR("Require %d hexnumbers but only got %zd\n", STATE_SIZE, vulState.size());
                iResult = -1;
            }
        }
    } else {
        // don't change random states if we are resuming
        iResult = 0;
    }

    return iResult;
}



//-----------------------------------------------------------------------------
//  setShuffles
// 
int SimParams::setShuffles(char *pShuffles) {
    int iResult = 0;
    memset(m_aiSeeds, 0, NUM_SEEDS*sizeof(uint));
    bool bOK = true;
    int i = 0;
    char *pi = strtok(pShuffles, ",");
    uint *po = m_aiSeeds;
    while (bOK && (pi != 0) && (i < NUM_SEEDS)) {
        if (strToNum(pi, po)) {
            po++;
            pi = strtok(NULL, ",");
        } else {
            bOK = false;
            iResult = -1;
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
            } else {
                // we have reached the end of the file - nop error
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
        printf("      %sevent-type%s       ::= \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\"\n", BOLD, OFF, 
               BOLD, EVENT_TYPE_WRITE, OFF, 
               BOLD, EVENT_TYPE_ENV, OFF, 
               BOLD, EVENT_TYPE_ARR, OFF, 
               BOLD, EVENT_TYPE_POP, OFF, 
               BOLD, EVENT_TYPE_FILE, OFF,
               BOLD, EVENT_TYPE_CHECK, OFF,
               BOLD, EVENT_TYPE_COMM, OFF,
               BOLD, EVENT_TYPE_USER, OFF);
        printf("    <event-times> defines at which times the event should be triggered.\n"); 
        printf("      %sevent-times%s      ::= <full-trigger> [ \"%s+%s\" <full-trigger>]*\n", 
               BOLD, OFF, BOLD, OFF);
        printf("      %sfull-trigger%s     ::= <normal-trigger> | <point-trigger> | <final-trigger>\n", BOLD, OFF);
        printf("      %snormal-trigger%s   ::= [<trigger-interval>] <step-size>\n", BOLD, OFF);
        printf("      %strigger-interval%s ::= \"%s[%s\"[<minval>] \"%s:%s\" [<maxval>] \"%s]%s\"\n",  
               BOLD, OFF, BOLD, OFF,  BOLD, OFF, BOLD, OFF);
        printf("      %sfinal-trigger%s    ::= %s\"final\"%s\n", BOLD, OFF, BOLD, OFF);
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
        printf("\n");
        printf("    The event parameters differ for the event types:\n");

        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_WRITE, OFF);
        printf("      %sevent-params%s ::= <type> [\"+\" <type>]*\n", BOLD, OFF);
        printf("      %stype%s         ::= \"%s%s%s\" | \"%s%s%s\"   | \"%s%s%s \" |\n", BOLD, OFF, 
               BOLD, EVENT_PARAM_WRITE_GRID, OFF, 
               BOLD, EVENT_PARAM_WRITE_GEO, OFF, 
               BOLD, EVENT_PARAM_WRITE_CLIMATE, OFF);
        printf("                         \"%s%s%s\"  | \"%s%s%s\" |  \"%s%s%s\" | \"%s%s:%s\"<speciesname>[\"#\"]\n", 
               BOLD, EVENT_PARAM_WRITE_VEG, OFF, 
               BOLD, EVENT_PARAM_WRITE_STATS, OFF, 
               BOLD, EVENT_PARAM_WRITE_NAV, OFF, 
               BOLD, EVENT_PARAM_WRITE_POP, OFF);
        printf("    Write the specified data sets to file.\n");
        printf("    In case of \"%s%s%s\", appending a '#' to the populaton name prevents writing of additional data (e.g. genome).\n", BOLD, EVENT_PARAM_WRITE_POP, OFF);
        printf("\n");

        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_ENV, OFF); 
        printf("      %sevent-params%s   ::= <type> [\"+\" <type>]* \":\" <qdf-file>\n", BOLD, OFF);
        printf("      %stype%s           ::= \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\"\n", BOLD, OFF,             
               BOLD, EVENT_PARAM_NAME_GEO, OFF, 
               BOLD, EVENT_PARAM_NAME_CLIMATE, OFF, 
               BOLD, EVENT_PARAM_NAME_VEG, OFF,
               BOLD, EVENT_PARAM_NAME_NAV, OFF),
        printf("    Read the specified data sets from file.\n");
        printf("\n");

        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_ARR, OFF); 
        printf("      %sevent-params%s   ::= <type> \":\" <arrayname> \":\" <qdf-file>\n", BOLD, OFF);
        printf("      %stype%s           ::= \"%s%s%s\" | \"%s%s%s\" | \"%s%s%s\"\n", BOLD, OFF,             
               BOLD, EVENT_PARAM_NAME_GEO, OFF, 
               BOLD, EVENT_PARAM_NAME_CLIMATE, OFF, 
               BOLD, EVENT_PARAM_NAME_VEG, OFF),
        printf("    Read the specified arrays from file.\n");
        printf("\n");

        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_POP, OFF); 
        printf("      %sevent-params%s   ::= <speciesname> [\"+\" <speciesname>]* \":\" <qdf-file>\n", BOLD, OFF);
        printf("    Read the specified populations from file.\n");
        printf("\n");

        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_FILE, OFF); 
        printf("      %sevent-params%s   ::= <filename>\n", BOLD, OFF);
        printf("      The file must contain lines of the form\n");
        printf("      %sline%s            ::= <event-type> \"%s|%s\" <event-params> \"%s@%s\" <event-times>\n", 
               BOLD, OFF, BOLD, OFF, BOLD, OFF);
        printf("    Add the events listed in the file to the event manager.\n");
        printf("\n");

        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_COMM, OFF);
        printf("        %sevent-params%s   ::= <cmd-file>  | <command>\n", BOLD, OFF);
        printf("        %scommand%s        ::= <iter_cmd> | <del_action_cmd> | <event>\n", BOLD, OFF);
        printf("        %siter_cmd%s       ::= \"%s%s%s\" <num_iters>\n", BOLD, OFF, BOLD, CMD_SET_ITERS, OFF);
        printf("        %sdel_action_cmd%s ::= \"%s%s%s\" <population>:<action_name>\n", BOLD, OFF, BOLD, CMD_REMOVE_ACTION, OFF);
        printf("        %sevent%s          : any event description; see definition above\n", BOLD, OFF);
        printf("      The format of <cmd-file>\n");
        printf("        %scmd-file%s       ::= <command-line>*\n", BOLD, OFF);
        printf("        %scommand-line%s   ::= <command> <NL>\n", BOLD, OFF);
        printf("      If the <cmd-file> has changed since the last triggered time the contents will be executed:\n");
        printf("        iter_cmd:        set new iteration limit\n");
        printf("        del_action_cmd:  remove action from prio list\n");
        printf("        event:           add event to event manager's list\n");
        printf("\n");

        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_CHECK, OFF); 
        printf("      %sevent-params%s   ::= <what> [\"+\" <what]*\n", BOLD, OFF);
        printf("      %swhat%s           ::= \"%s%s%s\"\n",  BOLD, OFF, BOLD, EVENT_PARAM_CHECK_LISTS, OFF);
        printf("    (Debugging only) \"lists\": check linked lists at specified times\n");
        printf("\n");

        printf("    for <event-type> == \"%s%s%s\":\n", BOLD, EVENT_TYPE_USER, OFF); 
        printf("      %sevent-params%s   ::= <eventid>:<stringdata>\n", BOLD, OFF);
        printf("      %seventid%s        : an event id in [%d, %d]\n",  BOLD, OFF, EVENT_ID_USR_MIN, EVENT_ID_USR_MAX);
        printf("      %sstringdata%s     : a string\n",  BOLD, OFF);
        printf("\n");

        printf("    Full example:\n");
        printf("      %s--events='env|geo+climate:map120.qdf@[120],write|pop:GrassLover|grid@5+[20:30]1,comm:REMOVE ACTION ConfinedMove@[3000]'%s\n", BOLD, OFF); 
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
        printf("    %sseedtype%s   ::= \"%s%s%s\" | \"%s%s%s\"<sequence> | \"%s%s%s\"<seed-file> | \"%s%s%s\"<arbitrary>\n", BOLD, OFF,
               BOLD, SEED_RANDOM, OFF, 
               BOLD, SEED_SEQ, OFF,
               BOLD, SEED_FILE, OFF,
               BOLD, SEED_PHRASE, OFF);
        printf("    %ssequence%s   ::= (<hexnumber> \"%s,%s\")*\n", BOLD, OFF, BOLD, OFF);
        printf("    %sarbitrary%s  : arbitrary string (enclose in quotes if it contains spaces)\n", BOLD, OFF);
        printf("    %sseed-file%s  : name of a seed file\n", BOLD, OFF);
        printf("    If seedType is \"%srandom%s\", the seeds are filled with random numbers\n", BOLD, OFF);
        printf("    based on the current time\n");
        printf("    A sequence must be consist of %d comma-separated 8-digit hexnumbers.\n", STATE_SIZE);
        printf("    If sequence is given, it is passed directly to the random nuber generators.\n");
        printf("    If an arbitrary string is provided, it is turned into a seed sequence using\n");
        printf("    a hash function (SHA-512)\n");
        printf("    If a seed file is specified it must conform to the following format.\n");
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
        printf("    Data provided this way will be passed to the parameter of the population's setParam(const char *pParam) method.\n");
        bFound = true;
    } 

    //-- topic "zip-output"
    if (bAll || (strcmp(pTopic, "zip-output") == 0)) {
        printHeaderLine(iL, "zip-output");
        printf("  %s--zip-output%s    zip all output files\n", BOLDBLUE, OFF);
        bFound = true;
    }

    //-- topic "resume"
    if (bAll || (strcmp(pTopic, "resume") == 0)) {
        printHeaderLine(iL, "resume");
        printf("  %s--resume%s    resume simulation from previously dumped env and pop files (created by a \"dump\" event)\n", BOLDBLUE, OFF);
        printf("    Use the full environment dump as initial grid, and all dumped population qdfs for '--pops='\n");
        printf("    To have totally exact resume, the number of threads must be the same as in the dumped simulation\n");
        bFound = true;
    }

    //-- topic "dump-on-interrupt"
    if (bAll || (strcmp(pTopic,  "dump-on-interrupt") == 0)) {
        printHeaderLine(iL,  "dump-on-interrupt");
        printf("  %s--dump-on-interrupt%s  set handler for SIG_INT (Ctrl-C): write dump and exit gracefullly\n", BOLDBLUE, OFF);
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
    case EVENT_ID_ENV:
        p = EVENT_TYPE_ENV;
        break;
    case EVENT_ID_COMM:
        p = EVENT_TYPE_COMM;
        break;
    case EVENT_ID_DUMP:
        p = EVENT_TYPE_DUMP;
        break;
    case EVENT_ID_ARR:
        p = EVENT_TYPE_ARR;
        break;
    }
    return p + sEventString;
}
