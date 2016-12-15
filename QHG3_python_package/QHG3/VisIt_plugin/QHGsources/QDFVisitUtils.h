#ifndef __QDFUTILS_H__
#define __QDFUTILS_H__

#include <hdf5.h>

// this macro is rewuired to calculate offsets of fields in a derived struct
#define qoffsetof(S,M) (ulong(&(S.M))   - ulong(&(S)))

#define POPGROUP_NAME   "Populations"
#define GEOGROUP_NAME   "Geography"
#define CLIGROUP_NAME   "Climate"
#define VEGGROUP_NAME   "Vegetation"
#define GRIDGROUP_NAME  "Grid"
#define MSTATGROUP_NAME "MoveStatistics"

#define AGENT_DATASET_NAME "AgentDataSet"
#define CELL_DATASET_NAME  "CellDataSet"
#define ROOT_ATTR_NAME "QHG"
#define ROOT_TIME_NAME "Time"
    

#define GRID_STYPE_ICO  "ICO"
#define GRID_STYPE_HEX  "HEX"
#define GRID_STYPE_RECT "RECT"

#define GRID_ATTR_NUM_CELLS   "NumCells"
#define GRID_ATTR_TYPE        "GridType"
#define GRID_ATTR_FORMAT      "GridFormat"
#define GRID_ATTR_PERIODIC    "Periodic"

#define GRID_DS_CELL_ID       "CellID"
#define GRID_DS_NUM_NEIGH     "NumNeighbors"
#define GRID_DS_NEIGHBORS     "Neighbors"
 
#define GEO_ATTR_NUM_CELLS    "NumCells"
#define GEO_ATTR_MAX_NEIGH    "MaxNeigh"
#define GEO_ATTR_RADIUS       "Radius"
#define GEO_ATTR_SEALEVEL     "SeaLevel"

#define GEO_DS_LONGITUDE      "Longitude"
#define GEO_DS_LATITUDE       "Latitude"
#define GEO_DS_ALTITUDE       "Altitude"
#define GEO_DS_AREA           "Area"
#define GEO_DS_DISTANCES      "Distances"
#define GEO_DS_ICE_COVER      "IceCover"
#define GEO_DS_WATER          "Water"

#define CLI_ATTR_NUM_CELLS    "NumCells"
#define CLI_ATTR_NUM_SEASONS  "NumSeasons"
#define CLI_ATTR_DYNAMIC      "Dynamic"

#define CLI_DS_ACTUAL_TEMPS   "ActualTemps"
#define CLI_DS_ACTUAL_RAINS   "ActualRains"
#define CLI_DS_ANN_MEAN_TEMP  "AnnualMeanTemp"
#define CLI_DS_ANN_TOT_RAIN   "AnnualRainfall"
#define CLI_DS_SEAS_TEMP_DIFF "SeasonalTempDiff" 
#define CLI_DS_SEAS_RAIN_RAT  "SeasonalRainRatios"
#define CLI_DS_CUR_SEASON     "CurSeason"

#define VEG_ATTR_NUM_CELLS    "NumCells"
#define VEG_ATTR_NUM_SPECIES  "NumSpecies"
#define VEG_ATTR_DYNAMIC      "Dynamic"

#define VEG_DS_MASS           "Mass"
#define VEG_DS_NPP            "NPP"


#define MSTAT_ATTR_NUM_CELLS   "NumCells"
#define MSTAT_DS_HOPS          "Hops"
#define MSTAT_DS_DIST          "Dist"
#define MSTAT_DS_TIME          "Time"

#define SPOP_ATTR_CLASS_ID     "ClassID"
#define SPOP_ATTR_CLASS_NAME   "ClassName"
#define SPOP_ATTR_SPECIES_ID   "SpeciesID"
#define SPOP_ATTR_SPECIES_NAME "SpeciesName"
#define SPOP_ATTR_SENS_DIST    "SensingDistance"
#define SPOP_ATTR_NUM_CELL     "NumCells"
#define SPOP_ATTR_PRIO_INFO    "PrioInfo"


hid_t qdf_openFile(const char *pFileName);
void  qdf_closeFile(hid_t hFile);

hid_t qdf_openGroup(hid_t hGroup, const char *pGroupName, bool bForceCheck=true);
void  qdf_closeGroup(hid_t hGroup);

void  qdf_closeDataSet(hid_t hDataSet);
void  qdf_closeDataSpace(hid_t hDataSpace);
void  qdf_closeDataType(hid_t hDataType);
void  qdf_closeAttribute(hid_t hDataSpace);
 
int qdf_extractSAttribute(hid_t hLoc, const char *pName, uint iNum, char   *sValue); 

int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, char   *cValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, int    *iValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, long   *lValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, float  *fValue); 
int qdf_extractAttribute(hid_t hLoc, const char *pName, uint iNum, double *dValue); 


int qdf_readArray(hid_t hGroup, const char *pName, int iNum, double *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNum, float *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNum, int *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNum, char *pData);

int qdf_readArray(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData);

int qdf_readArrays(hid_t hGroup, const char *pName, int iNumArr, int iSize, double **pData);


#endif
