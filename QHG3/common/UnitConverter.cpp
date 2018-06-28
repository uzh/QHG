#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <string>

#include "UnitConverter.h"
#include "MessLogger.h"


// initialization of static members before everything else

factorsMap UnitConverter::initFactorsMap() {
    factorsMap amap;
    std::map<std::string,double> bmap;
    bmap[""] = 0;
    amap[""] = bmap;
    return amap;
};
          
unitsMap UnitConverter::initUnitsMap() {
    unitsMap amap;
    amap[""] = std::pair<float, std::string>(0,"");
    return amap;
};

factorsMap UnitConverter::m_mFactors = UnitConverter::initFactorsMap();
unitsMap UnitConverter::m_mSimUnits = UnitConverter::initUnitsMap();



//-----------------------------------------------------------------------------
// constructor

UnitConverter::UnitConverter(const char *sDefFile) {

    m_pLR = LineReader_std::createInstance(sDefFile, "rt");

    if (m_pLR != NULL) {
        initFactors();
        readDef();
    } else {
        LOG_ERROR("[UnitConverter] units file %s not found\n",sDefFile);
    }
}


//-----------------------------------------------------------------------------
// destructor

UnitConverter::~UnitConverter() {
    
    if (m_pLR != NULL) {
        delete m_pLR;
    }

}


//-----------------------------------------------------------------------------
// convert()
void UnitConverter::convert(char *sUnits, int iNum, double *pData) {
    
    double dF = getCompositeFactor(sUnits);
    
    for (int i = 0; i < iNum; i++) {
        *pData++ *= dF;
    }

    return;
}

//-----------------------------------------------------------------------------
// convert()
void UnitConverter::convert(char *sUnits, int iNum, float *pData) {
    
    double dF = getCompositeFactor(sUnits);
    
    for (int i = 0; i < iNum; i++) {
        *pData++ *= (double)dF;
    }

    return;
}


//-----------------------------------------------------------------------------
// getCompositeFactor()
double UnitConverter::getCompositeFactor(char *sUnits) {
    
    double dF = 1.0;
    
    // example of units: "g m^-2 *s^1.5"

    char *sInput = new char[64];
    strcpy(sInput, sUnits);

    char *sUnit = new char[8];
    sUnit = strtok(sInput," *");
    while(sUnit != NULL) {
        dF *= getSingleFactor(sUnit);
        sUnit = strtok(NULL," *");
    }
    
    return dF;
}


//-----------------------------------------------------------------------------
// getSingleFactor()
double UnitConverter::getSingleFactor(char *sUnit) {

    double dF = 0.0;
    
    char *sInput = new char[8];
    strcpy(sInput, sUnit);

    char *sC = strtok(sInput,"^");
    
    // let's find if we have the unit in our conversion map

    factorsMap::iterator it = m_mFactors.begin();
    while (strcmp(sC, it->first.c_str()) != 0 && it != m_mFactors.end()) {
        it++;
    }
    if (it == m_mFactors.end()) {
        LOG_ERROR("[UnitConverter] could not find conversion for unit %s\n",sInput);
    } else {

        std::map<std::string, double> fromThisUnit = it->second;
        std::pair<float, std::string> finalUnit;

        // which kind of unit are we converting?

        if (strcmp(sC, KEY_SECOND) == 0 ||
            strcmp(sC, KEY_HOUR) == 0 ||
            strcmp(sC, KEY_DAY) == 0 ||
            strcmp(sC, KEY_YEAR) == 0 ||
            strcmp(sC, KEY_KYEAR) == 0) {
            
            finalUnit = m_mSimUnits[KEY_TIME_UNIT];

        } else if (strcmp(sC, KEY_METER) == 0 ||
                   strcmp(sC, KEY_KMETER) == 0) {

            finalUnit = m_mSimUnits[KEY_LENGTH_UNIT];

        } else if (strcmp(sC, KEY_GRAM) == 0 ||
                   strcmp(sC, KEY_KGRAM) == 0 ||
                   strcmp(sC, KEY_TON) == 0) {

            finalUnit = m_mSimUnits[KEY_MASS_UNIT];
            
        } else if (strcmp(sC, KEY_JOULE) == 0 ||
                   strcmp(sC, KEY_CAL) == 0 ||
                   strcmp(sC, KEY_KCAL) == 0 ||
                   strcmp(sC, KEY_KGDM) == 0 ||
                   strcmp(sC, KEY_KGC) == 0) {

            finalUnit = m_mSimUnits[KEY_ENERGY_UNIT];
            
        }
        
        // the basic conversion factor is the unit conversion from
        // the data to the sim units, divided by the number in front of the sim unit
        // e.g.: if the sim time unit is 7.0 days and we convert from years, we have:
        // 1 year = 365.242 days / (7.0 days / time unit) = 52.177 time units

        dF = fromThisUnit[finalUnit.second] / finalUnit.first;
    }
    
    // now let's take care of exponents in the unit 
    // by elevating to power the conversion factor we just found

    sC = strtok(NULL,"^");
    if (sC != NULL) {
        float fExponent = (float)atof(sC);
        dF = pow(dF,fExponent);
    }

    return dF;
}


//-----------------------------------------------------------------------------
// readDef()
int UnitConverter::readDef() {
    int iResult = 0;

    while (!m_pLR->isEoF()) {
        char *pLine = m_pLR->getNextLine(GNL_IGNORE_ALL);
        if (pLine != NULL) {

            char *sPhysQuantity = strtok(pLine, " \t:;");

            if (strcmp(sPhysQuantity,KEY_TIME_UNIT) != 0 &&
                strcmp(sPhysQuantity,KEY_LENGTH_UNIT) != 0 &&
                strcmp(sPhysQuantity,KEY_MASS_UNIT) != 0 &&
                strcmp(sPhysQuantity,KEY_ENERGY_UNIT) != 0) {

                LOG_ERROR("[UnitConverter::readDef] ERROR parsing units definition file\n");
                LOG_ERROR("[UnitConverter::readDef] Unknown physical quantity: %s\n",sPhysQuantity);
                iResult = -1;
                
        } else {
                float dHowMuch = (float) atof(strtok(NULL, " \t:;"));
                char *pUnit = strtok(NULL, " \t:;");
                if (pUnit != NULL) {
                    m_mSimUnits[sPhysQuantity] = std::pair< float, std::string >(dHowMuch,pUnit);
                } else {
                    LOG_ERROR("[UnitConverter::readDef] ERROR parsing units definition file\n");
                    iResult = -1;
                }
            }
        }
    }
    
    if (iResult == 0) {
        LOG_STATUS("Simulation Units:\n");
        for (std::map< std::string, std::pair< float, std::string> >::iterator it = m_mSimUnits.begin(); 
             it != m_mSimUnits.end(); 
             ++it) {
            LOG_STATUS("%s : %f %s\n", it->first.c_str(), it->second.first, it->second.second.c_str());
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// initFactors()

void UnitConverter::initFactors() {
    
    std::map<std::string, double> mConv;


    ////////////////////////////////////
    // T I M E 
    
    // seconds
    mConv[KEY_SECOND] = 1.0;
    mConv[KEY_HOUR] = 1.0 / S_IN_H;
    mConv[KEY_DAY] = 1.0 / (S_IN_H * H_IN_D);
    mConv[KEY_YEAR] = 1.0 / (S_IN_H * H_IN_D * D_IN_Y);
    mConv[KEY_KYEAR] = 1.0 / (S_IN_H * H_IN_D * D_IN_Y * Y_IN_KY);
    m_mFactors[KEY_SECOND] = mConv;
    mConv.clear();

    // hours
    mConv[KEY_SECOND] = S_IN_H;
    mConv[KEY_HOUR] = 1.0;
    mConv[KEY_DAY] = 1.0 / H_IN_D;
    mConv[KEY_YEAR] = 1.0 / (H_IN_D * D_IN_Y);
    mConv[KEY_KYEAR] = 1.0 / (H_IN_D * D_IN_Y * Y_IN_KY);
    m_mFactors[KEY_HOUR] = mConv;
    mConv.clear();

    // days
    mConv[KEY_SECOND] = S_IN_H * H_IN_D;
    mConv[KEY_HOUR] = H_IN_D;
    mConv[KEY_DAY] = 1.0;
    mConv[KEY_YEAR] = 1.0 / D_IN_Y;
    mConv[KEY_KYEAR] = 1.0 / (D_IN_Y * Y_IN_KY);
    m_mFactors[KEY_DAY] = mConv;
    mConv.clear();

    // years
    mConv[KEY_SECOND] = S_IN_H * H_IN_D * D_IN_Y;
    mConv[KEY_HOUR] = H_IN_D * D_IN_Y;
    mConv[KEY_DAY] = D_IN_Y;
    mConv[KEY_YEAR] = 1.0;
    mConv[KEY_KYEAR] = 1.0 / Y_IN_KY;
    m_mFactors[KEY_YEAR] = mConv;
    mConv.clear();

    // years
    mConv[KEY_SECOND] = S_IN_H * H_IN_D * D_IN_Y * Y_IN_KY;
    mConv[KEY_HOUR] = H_IN_D * D_IN_Y * Y_IN_KY;
    mConv[KEY_DAY] = D_IN_Y * Y_IN_KY;
    mConv[KEY_YEAR] = Y_IN_KY;
    mConv[KEY_KYEAR] = 1.0;
    m_mFactors[KEY_KYEAR] = mConv;
    mConv.clear();


    ////////////////////////////////////
    // L E N G T H 
    
    // meters
    mConv[KEY_METER] = 1.0;
    mConv[KEY_KMETER] = 1.0 / M_IN_KM;
    m_mFactors[KEY_METER] = mConv;
    mConv.clear();

    // kilometers
    mConv[KEY_METER] = M_IN_KM;
    mConv[KEY_KMETER] = 1.0;
    m_mFactors[KEY_KMETER] = mConv;
    mConv.clear();

    
    ////////////////////////////////////
    // M A S S 

    // grams
    mConv[KEY_GRAM] = 1.0;
    mConv[KEY_KGRAM] = 1.0 / G_IN_KG;
    mConv[KEY_TON] = 1.0 / (G_IN_KG * KG_IN_T);
    m_mFactors[KEY_GRAM] = mConv;
    mConv.clear();

    // kilograms
    mConv[KEY_GRAM] = G_IN_KG;
    mConv[KEY_KGRAM] = 1.0;
    mConv[KEY_TON] = 1.0 / KG_IN_T;
    m_mFactors[KEY_KGRAM] = mConv;
    mConv.clear();
    
    // tons
    mConv[KEY_GRAM] = G_IN_KG * KG_IN_T;
    mConv[KEY_KGRAM] = KG_IN_T;
    mConv[KEY_TON] = 1.0;
    m_mFactors[KEY_TON] = mConv;
    mConv.clear();
    

    ////////////////////////////////////
    // E N E R G Y 

    // Joules
    mConv[KEY_JOULE] = 1.0;
    mConv[KEY_CAL] = 1.0 / J_IN_CAL;
    mConv[KEY_KCAL] = 1.0 / (J_IN_CAL * CAL_IN_KCAL);
    mConv[KEY_KGDM] = 1.0 / J_IN_DM;
    mConv[KEY_KGC] = C_IN_DM / J_IN_DM;
    m_mFactors[KEY_JOULE] = mConv;
    mConv.clear();

    // calories 
    mConv[KEY_JOULE] = J_IN_CAL;
    mConv[KEY_CAL] = 1.0;
    mConv[KEY_KCAL] = 1.0 / (CAL_IN_KCAL);
    mConv[KEY_KGDM] = mConv[KEY_JOULE] / J_IN_DM;
    mConv[KEY_KGC] = mConv[KEY_JOULE] * C_IN_DM / J_IN_DM;
    m_mFactors[KEY_CAL] = mConv;
    mConv.clear();

    // kilocalories 
    mConv[KEY_JOULE] = J_IN_CAL * CAL_IN_KCAL;
    mConv[KEY_CAL] = CAL_IN_KCAL;
    mConv[KEY_KCAL] = 1.0;
    mConv[KEY_KGDM] = mConv[KEY_JOULE] / J_IN_DM;
    mConv[KEY_KGC] = mConv[KEY_JOULE] * C_IN_DM / J_IN_DM;
    m_mFactors[KEY_KCAL] = mConv;
    mConv.clear();

    // dry matter
    mConv[KEY_JOULE] = J_IN_DM;
    mConv[KEY_CAL] = J_IN_DM / (J_IN_CAL);
    mConv[KEY_KCAL] = J_IN_DM / (J_IN_CAL * CAL_IN_KCAL);
    mConv[KEY_KGDM] = 1.0;
    mConv[KEY_KGC] = C_IN_DM;
    m_mFactors[KEY_KGDM] = mConv;
    mConv.clear();

    // carbon
    mConv[KEY_JOULE] = J_IN_DM / C_IN_DM;
    mConv[KEY_CAL] = J_IN_DM / (C_IN_DM * J_IN_CAL);
    mConv[KEY_KCAL] = J_IN_DM / (C_IN_DM * J_IN_CAL * CAL_IN_KCAL);
    mConv[KEY_KGDM] = 1.0 / C_IN_DM;
    mConv[KEY_KGC] = 1.0;
    m_mFactors[KEY_KGC] = mConv;
    mConv.clear();

    
    //////////////////////////////////
    // T E M P E R A T U R E 
    mConv[KEY_CELSIUS] = 1.0;
    m_mFactors[KEY_CELSIUS] = mConv;
    mConv.clear();


    //////////////////////////////////
    // R A I N F A L L 
    mConv[KEY_MMRAIN] = 1.0;
    m_mFactors[KEY_MMRAIN] = mConv;
    mConv.clear();

    
}

