#ifndef __ANALYSISUTILS_H__
#define __ANALYSISUTILS_H__

#include "types.h"

typedef struct locspec {
    const char *pLocFile;
    double dDistance;
    uint iNum;

    locspec(const char *pFile, double dDist=0, uint iN=0) 
        : pLocFile(pFile),dDistance(dDist),iNum(iN) {};
} locspec;

typedef struct locitem {
    double dLon;
    double dLat;
    double dDist;
    int iNum;
    int iPadding;
} locitem;

// map: location name => locitem
typedef std::map<std::string, locitem>               loc_data;

// vector of string
typedef std::vector<std::string>                     stringvec; 
// map: cell id => coord pair
typedef std::map<int, std::pair<double, double> >    arrpos_coords;

// (lon, lat)
typedef  std::pair<double,double> coords;
// (loc, time)
typedef std::pair<std::string,double> loctime;
// (loc,time) => idlist
typedef std::map<loctime, std::vector<idtype> > tnamed_ids;
// id => (lon,lat)
typedef std::map<idtype, coords > id_locs;
// id => node id
typedef std::map<idtype,int> id_node;
//locname => (lon,lat)
typedef std::map<std::string, coords > named_locs;

int fillLocData(const locspec *pLocSpec, loc_data &mLocData, stringvec *pvNames=NULL);



#endif
