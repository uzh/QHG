#include "FreeLookUp.h"
#include "utils.h"


//virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

//----------------------------------------------------------------------------
// constructor
//
FreeLookUp::FreeLookUp(double dMinLevel, double dMaxLevel, 
               double_levels &diLevels, 
               double_intensities &vdiR, 
               double_intensities &vdiG, 
               double_intensities &vdiB, 
               double_intensities &vdiA) 
    : LookUp(dMinLevel, dMaxLevel) {

    defaultColors();

    int iResult = init(diLevels, vdiR, vdiG, vdiB, vdiA);
    if (iResult != 0) {
        defaultColors();
    }

}

//----------------------------------------------------------------------------
// init
//
int FreeLookUp::init(double_levels &diLevels, 
                     double_intensities &vdiR, 
                     double_intensities &vdiG, 
                     double_intensities &vdiB, 
                     double_intensities &vdiA) {
    
    int iResult = 0;
    /* specials prefilled with default values
    for (int i =0; i < NUM_SPECIALS; i++) {
        for (int j =0; j < NUM_COLS; j++) {
            m_adSpecial[i][j] = dNaN;
        }
    }
    */
    // add min and max explicitly
    diLevels["min"] = m_dMinLevel;
    diLevels["max"] = m_dMaxLevel;

    iResult = processIntensities(vdiR, COL_R, diLevels);

    if (iResult == 0) {
        iResult = processIntensities(vdiG, COL_G, diLevels);

        if (iResult == 0) {
            iResult = processIntensities(vdiB, COL_B, diLevels);

            if (iResult == 0) {
                iResult = processIntensities(vdiA, COL_A, diLevels);

                if (iResult == 0) {
                    //         if no submin, set submin colors to min colors
                    //         if no supmax, set supmax colors to max colors
                }
            }
        }
    }
    return iResult;
}


//----------------------------------------------------------------------------
// constructor
//
FreeLookUp::FreeLookUp(double dMinLevel, double dMaxLevel, 
                       double_levels diLevels, 
                       double_intensities *pvadiAll) 
    : LookUp(dMinLevel, dMaxLevel) {
    
    defaultColors();
    int iResult = init(diLevels, 
                       pvadiAll[COL_R],
                       pvadiAll[COL_G],
                       pvadiAll[COL_B],
                       pvadiAll[COL_A]);

    if (iResult != 0) {
        defaultColors();
    }
    
}

//----------------------------------------------------------------------------
// defaultColors
//
void FreeLookUp::defaultColors() {
    m_mLineSegDefs[COL_R][m_dMinLevel] = lineseg(0.5, 0);
    m_mLineSegDefs[COL_G][m_dMinLevel] = lineseg(0.5, 0);
    m_mLineSegDefs[COL_B][m_dMinLevel] = lineseg(0.5, 0);
    m_mLineSegDefs[COL_A][m_dMinLevel] = lineseg(0.5, 0);
    m_mLineSegDefs[COL_R][m_dMaxLevel] = lineseg(0.5, 0);
    m_mLineSegDefs[COL_G][m_dMaxLevel] = lineseg(0.5, 0);
    m_mLineSegDefs[COL_B][m_dMaxLevel] = lineseg(0.5, 0);
    m_mLineSegDefs[COL_A][m_dMaxLevel] = lineseg(0.5, 0);

    m_adSpecial[SPECIAL_NEGINF][COL_R] = 1.0;
    m_adSpecial[SPECIAL_NEGINF][COL_G] = 0.0;
    m_adSpecial[SPECIAL_NEGINF][COL_B] = 0.0;
    m_adSpecial[SPECIAL_NEGINF][COL_A] = 0.5;

    m_adSpecial[SPECIAL_POSINF][COL_R] = 0.0;
    m_adSpecial[SPECIAL_POSINF][COL_G] = 1.0;
    m_adSpecial[SPECIAL_POSINF][COL_B] = 0.0;
    m_adSpecial[SPECIAL_POSINF][COL_A] = 0.5;

    m_adSpecial[SPECIAL_NANVAL][COL_R] = 0.0;
    m_adSpecial[SPECIAL_NANVAL][COL_G] = 0.0;
    m_adSpecial[SPECIAL_NANVAL][COL_B] = 1.0;
    m_adSpecial[SPECIAL_NANVAL][COL_A] = 0.5;

    m_adSpecial[SPECIAL_SUBMIN][COL_R] = 1.0;
    m_adSpecial[SPECIAL_SUBMIN][COL_G] = 1.0;
    m_adSpecial[SPECIAL_SUBMIN][COL_B] = 0.0;
    m_adSpecial[SPECIAL_SUBMIN][COL_A] = 0.5;

    m_adSpecial[SPECIAL_SUPMAX][COL_R] = 1.0;
    m_adSpecial[SPECIAL_SUPMAX][COL_G] = 0.0;
    m_adSpecial[SPECIAL_SUPMAX][COL_B] = 1.0;
    m_adSpecial[SPECIAL_SUPMAX][COL_A] = 0.5;

}

//----------------------------------------------------------------------------
// getType
//
int FreeLookUp::getType(ci_string sName, double_levels &diLevels) {
    int iType = -1;
    
    for (unsigned int i = 0; (i < NUM_SPECIALS) && (iType < 0); i++) {
        if (sName == asSpecialNames[i]) {
            iType = i+1;
        }
    }

    if (iType < 0) {
         for (unsigned int i = 0; (i < NUM_LEVELS) && (iType < 0); i++) {
            if (sName == asLevelNames[i]) {
                iType = 0;
            }
        }
        
    }

    if (iType < 0) {
        double_levels::const_iterator it;
        for (it = diLevels.begin(); (it != diLevels.end()) && (iType < 0); it++) {
            if (it->first == sName) {
                iType = 0;
            }
        }
    }



    return iType;
}

//----------------------------------------------------------------------------
// testAndSetSpecial
//
int FreeLookUp::testAndSetSpecial(double_intensity diPoints, int iCol, double_levels diLevels, double *pdVal, double *pdLev) {
    int iResult = 0;

    ci_string sCurName = diPoints.first;
    int iType = getType(sCurName, diLevels);
    if (iType >= 0) {
        // this will give a correct value because
        // sCurname is a key 
        *pdVal  = diPoints.second;
        if (iType > 0) {
            m_adSpecial[iType-1][iCol] = *pdVal;
            
        } else {
            *pdLev  = diLevels[sCurName];
            // itype = 0
            m_vLevels[iCol].insert(*pdLev);
        }
        iResult = iType;
    }
    return iResult;
}

 
//----------------------------------------------------------------------------
// processIntensities
//
int FreeLookUp::processIntensities(double_intensities &vdi, int iCol, double_levels diLevels) {
    int iResult = 0;
    unsigned int i = 0;
    if (vdi.size() > 0) {
        bool bGoOn = true;
        
        double      dCurVal=0;
        double      dCurLev=0;
        while (bGoOn && (iResult == 0) && (i < vdi.size())) {
            int iType = testAndSetSpecial(vdi[i], iCol, diLevels, &dCurVal, &dCurLev);

            if (iType < 0) {
                iResult = -1;
            } else if (iType == 0){
                bGoOn = false;
            }
            i++;
        }
        
        bGoOn = true;
        double dNextVal=0;
        double dNextLev=0;
        while (bGoOn && (iResult == 0) && (i < vdi.size())) {
            bool bSpecial = true;
            while (bSpecial && (i < vdi.size()) && (iResult == 0)) {
                int iType =  testAndSetSpecial(vdi[i], iCol, diLevels, &dNextVal, &dNextLev);
                if (iType < 0) {
                    iResult = -1;
                } else if (iType == 0) {
                    // exit to calc line data
                    bSpecial = false;
                }
                i++;
            }
            if (!bSpecial) {
                // calc line data
                double dA = (dNextVal - dCurVal)/(dNextLev - dCurLev);
                m_mLineSegDefs[iCol][dCurLev] = lineseg(dCurVal, dA);
                dCurVal = dNextVal;
                dCurLev = dNextLev;


            }
        }

        // 
    } else {
        printf("*** Not enough intensity values\n");
    } 

    return iResult;
}

//----------------------------------------------------------------------------
// getColor
//
void FreeLookUp::getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha) {
    if (isnan(dValue)) {
        //        printf("%f is nan\n", dValue);
        dRed   = m_adSpecial[SPECIAL_NANVAL][COL_R];
        dGreen = m_adSpecial[SPECIAL_NANVAL][COL_G];
        dBlue  = m_adSpecial[SPECIAL_NANVAL][COL_B];
        dAlpha = m_adSpecial[SPECIAL_NANVAL][COL_A];
    } else if (isinf(dValue)) {
        if (dValue < 0) {
            //            printf("%f is neginf\n", dValue);
            dRed   = m_adSpecial[SPECIAL_NEGINF][COL_R];
            dGreen = m_adSpecial[SPECIAL_NEGINF][COL_G];
            dBlue  = m_adSpecial[SPECIAL_NEGINF][COL_B];
            dAlpha = m_adSpecial[SPECIAL_NEGINF][COL_A];
        } else {
            //            printf("%f is posinf\n", dValue);
            dRed   = m_adSpecial[SPECIAL_POSINF][COL_R];
            dGreen = m_adSpecial[SPECIAL_POSINF][COL_G];
            dBlue  = m_adSpecial[SPECIAL_POSINF][COL_B];
            dAlpha = m_adSpecial[SPECIAL_POSINF][COL_A];
        }
    } else {
        if (dValue < m_dMinLevel) {
            //            printf("%f is submin\n", dValue);
            dRed   = m_adSpecial[SPECIAL_SUBMIN][COL_R];
            dGreen = m_adSpecial[SPECIAL_SUBMIN][COL_G];
            dBlue  = m_adSpecial[SPECIAL_SUBMIN][COL_B];
            dAlpha = m_adSpecial[SPECIAL_SUBMIN][COL_A];
        } else if (dValue >= m_dMaxLevel) {
            //            printf("%f is supmax\n", dValue);
            dRed   = m_adSpecial[SPECIAL_SUPMAX][COL_R];
            dGreen = m_adSpecial[SPECIAL_SUPMAX][COL_G];
            dBlue  = m_adSpecial[SPECIAL_SUPMAX][COL_B];
            dAlpha = m_adSpecial[SPECIAL_SUPMAX][COL_A];
        } else {
            // find closest levels to V in red, blue, green,amd alpha seg defs
            double dLR = findLevel(dValue, COL_R);
            double dLG = findLevel(dValue, COL_G);
            double dLB = findLevel(dValue, COL_B);
            double dLA = findLevel(dValue, COL_A);

            // calc values for r, g, b, a with their corrresponding linesegs
            dRed    = interpol(dValue, dLR, COL_R);
            dGreen  = interpol(dValue, dLG, COL_G);
            dBlue   = interpol(dValue, dLB, COL_B);
            dAlpha  = interpol(dValue, dLA, COL_A);
        }

    }
}

double FreeLookUp::findLevel(double dValue, int iCol) {
    double dLevel = -1;
    double dPrev = -1;
    std::set<double>::const_iterator it;
    for (it = m_vLevels[iCol].begin(); (dLevel < 0) && (it != m_vLevels[iCol].end()); it++) {
        if (*it > dValue+0.001) {
            dLevel = dPrev;
        }
        dPrev = *it;
    }
    return dLevel;
}

double FreeLookUp::interpol(double dValue, double dLevel, int iCol) {
    double dResult=-1;
    
    std::map<double, lineseg>::const_iterator it = m_mLineSegDefs[iCol].find(dLevel);
    if (it != m_mLineSegDefs[iCol].end()) {
        lineseg ls = it->second;
        dResult = ls.first+ls.second*(dValue-dLevel);
    } else {
        printf("No lineseg found for level %f in col %d\n", dLevel, iCol);
    }
    return dResult;
    
}
