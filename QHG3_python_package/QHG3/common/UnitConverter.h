// This class creates units conversions to be used by QHG3
// the time step of the simulation is always 1 in sim units

#ifndef __UNITCONVERTER_H__
#define __UNITCONVERTER_H__

#include <map>
#include <string>
#include <hdf5.h>

#include "LineReader.h"

#define KEY_TIME_UNIT "TIME"
#define KEY_LENGTH_UNIT "LENGTH"
#define KEY_MASS_UNIT "MASS"
#define KEY_ENERGY_UNIT "ENERGY"

#define KEY_NOUNIT  "#"

#define KEY_SECOND  "s"
#define KEY_HOUR    "h"
#define KEY_DAY     "d"
#define KEY_YEAR    "y"
#define KEY_KYEAR   "ky"

#define KEY_METER   "m"
#define KEY_KMETER  "km"

#define KEY_GRAM    "g"
#define KEY_KGRAM   "kg"
#define KEY_TON     "t"

#define KEY_JOULE   "J"
#define KEY_CAL     "cal"
#define KEY_KCAL    "kcal"
#define KEY_KGDM    "kgDM" // dry plant matter
#define KEY_KGC     "kgC"  // carbon plant matter

#define KEY_CELSIUS "C"

#define KEY_MMRAIN  "mm"

#define S_IN_H 3600.0
#define H_IN_D 24.0
#define D_IN_Y 365.242
#define Y_IN_KY 1000.0

#define M_IN_KM 1000.0

#define G_IN_KG 1000.0
#define KG_IN_T 1000.0

#define J_IN_CAL 4.184
#define CAL_IN_KCAL 1000.0
#define C_IN_DM 0.45
#define J_IN_DM 1.85e6

// general conversion factors, universal.
// < from unit, < to unit, conv factor > >
// e.g.:  < m... , < km... , 0.001... > >
typedef std::map< std::string, std::map<std::string, double> > factorsMap;

// the units of this particular simulation.
// < physical quantity, < howmuch, unit > > 
// e.g.:  < TIME, < 7.0, d > >
typedef std::map< std::string, std::pair<float, std::string> > unitsMap;

class UnitConverter {
    
 public:   
    UnitConverter(const char *sDefFile);
    ~UnitConverter();
    
    int readDef();
    static void convert(char *sUnits, int iNum, double *pData);
    static void convert(char *sUnits, int iNum, float *pData);
    

    // these two functions are needed to initialize static members in here

    static factorsMap initFactorsMap();
    static unitsMap initUnitsMap();

 protected:
    void initFactors();
    static double getCompositeFactor(char *sUnits); 
    static double getSingleFactor(char *sUnit);

    static factorsMap m_mFactors;
    static unitsMap m_mSimUnits;

    LineReader *m_pLR;
};

#endif

