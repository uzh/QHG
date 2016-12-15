#include <math.h>
#include "TabDataReader.h"
#include "utils.h"
#include "types.h"



static double s_fLonMin = 1000000;

TabDataReader::TabDataReader(char *pFileName, 
                             float fDataLonMin, float fDLon, float fRangeLonMin, float fRangeLonMax,
                             float fDataLatMin, float fDLat, float fRangeLatMin, float fRangeLatMax,
                             int iNumVals) 
    :   TabReader(pFileName, fDLon, fRangeLonMin, fRangeLonMax,
                  fDLat, fRangeLatMin, fRangeLatMax),
        m_iNumVals(iNumVals)  {



    m_fGridPhaseLon = getPhase(fDataLonMin, fDLon);
    m_fGridPhaseLat = getPhase(fDataLatMin, fDLat);
		
    m_fGridLonMin = getNextGridPointLon(m_fRangeLonMin);
    float fMaxLon = getNextGridPointLon(m_fRangeLonMax) - fDLon;
    m_fGridLatMin = getNextGridPointLat(m_fRangeLatMin);
    float fMaxLat = getNextGridPointLat(m_fRangeLatMax) - fDLat;
		
    // calculate array size
    m_iNLon = (int)(1+(fMaxLon-m_fGridLonMin)/m_fDLon);
    m_iNLat = (int)(1+(fMaxLat-m_fGridLatMin)/m_fDLat);
    // allocate array
    /// printf("ArraySize: %dx%dx%d\n", m_iNumVals, m_iNLat, m_iNLon);
    m_afData = new float**[m_iNumVals];
    for (int h = 0; h < m_iNumVals; h++) {
        m_afData[h] = new float*[m_iNLat];
        for (int i = 0; i < m_iNLat; ++i) {
            m_afData[h][i] = new float[m_iNLon];
            for (int j = 0; j < m_iNLon; ++j) {
                m_afData[h][i][j] = fNegInf;
            }
        }
    }
}

TabDataReader::~TabDataReader() {
    if (m_afData != NULL) {
        for (int h = 0; h < m_iNumVals; ++h) {
            if (m_afData[h] != NULL) {
                for (int i = 0; i < m_iNLat; ++i) {
                    if (m_afData[h ][i] != NULL) {
                        delete[] m_afData[h][i];
                    }
                }
                delete[] m_afData[h];
            }
        }
        delete[] m_afData;
    }
}

float TabDataReader::getPhase(float fMin, float fD) {
    return (float)(fMin - floorf(fMin/fD)*fD);
}
	
float TabDataReader::getNextGridPointLon(float fM) {
    return (float) (m_fGridPhaseLon + ceilf((fM-m_fGridPhaseLon)/m_fDLon)*m_fDLon);
}
		
float TabDataReader::getNextGridPointLat(float fM) {
    return (float) (m_fGridPhaseLat + ceilf((fM-m_fGridPhaseLat)/m_fDLat)*m_fDLat);
}

float TabDataReader::X2Lon(float iX) {
    return m_fGridLonMin+m_fDLon*iX;
}
float TabDataReader::Y2Lat(float iY) {
    return m_fGridLatMin+m_fDLat*iY;
}
//------------------------------------------------------------------------------
// action
//   claculate indexes for fLon&fLat, extract value & save
//
bool TabDataReader::action(float fLon, float fLat, char *pLine) {
		
    bool bOK = true;
    int iLon = calcLonIndex(fLon);
    int iLat = calcLatIndex(fLat);

    if ((0 <= iLon) && (iLon <= m_iNLon) && (0 <= iLat) && (iLat <= m_iNLat)) {
        for (int iIndex = 0; iIndex < m_iNumVals; iIndex++) {
            m_afData[iIndex][iLat][iLon] = m_pde->extractItem(pLine, iIndex);
        }
    } else {
        printf("Bad index : [%d][%d]\n", iLat, iLon);
        bOK = false;
    }
    return bOK;
}

		
//------------------------------------------------------------------------------
// getValue
//   get value for specified longitude and latitude
//
 float TabDataReader::getValue(float fLon, float fLat, int iIndex) {
    int iLon = calcLonIndex(fLon);
    int iLat = calcLatIndex(fLat);
    float fValue = fNaN;

    if ((0 <= iLon)   && (iLon   < m_iNLon) && 
        (0 <= iLat)   && (iLat   < m_iNLat) &&
        (0 <= iIndex) && (iIndex < m_iNumVals)) {
        fValue = m_afData[iIndex][iLat][iLon];
    } else {
        printf("Bad index : [%d][%d][%d] %f\n", iIndex, iLat, iLon, fLon);
        if (fLon < s_fLonMin) {
            s_fLonMin = fLon;
            printf("         LonMin:%f\n", s_fLonMin);
        }
    }

    
    return fValue;
}


//------------------------------------------------------------------------------
// calcLonIndex
//   calc array index for longitude
//
int TabDataReader::calcLonIndex(float fLon) {
    return (int)((fLon-m_fGridLonMin)/m_fDLon);
}
	
//------------------------------------------------------------------------------
// calcLon
//   calc longitude for array index
//
float TabDataReader::calcLon(int iLonIndex) {
    return iLonIndex*m_fDLon+m_fGridLonMin;
}
	
//------------------------------------------------------------------------------
// calcLatIndex
//   calc array index for latitude
//
int TabDataReader::calcLatIndex(float fLat) {
    return (int)((fLat-m_fGridLatMin)/m_fDLat);
}

//------------------------------------------------------------------------------
// calcLat
//   calc latitude for array index
//
float TabDataReader::calcLat(int iLatIndex) {
    return iLatIndex*m_fDLat+m_fGridLatMin;
}

	
//------------------------------------------------------------------------------
// getValueBiLin
//   bilinear interpolation from grid points surrounding (fLon, fLat)
//
 float TabDataReader::getValueBiLin(float fLon, float fLat, int iIndex) {
    //printf("doing Lon %f\n", fLon);
    float fLonL = (float) ((int)((fLon)*4))/4.0f;
    float fLonU = (float) ((int)((fLon+0.5f)*4))/4.0f;
    
    //printf("doing Lat %f", fLat);
    float fLatL = (float) ((int)((fLat)*4))/4.0f;
    float fLatU = (float) ((int)((fLat+0.5f)*4))/4.0f;
		
    int iLonL = calcLonIndex(fLonL);
    int iLatL = calcLatIndex(fLatL);
    int iLonU = calcLonIndex(fLonU);
    int iLatU = calcLatIndex(fLatU);
    
		
    float fLonL2 = calcLon(iLonL);
    float fLatL2 = calcLat(iLatL);
    /*
      float fLonU2 = calcLon(iLonU);
      float fLatU2 = calcLat(iLatU);
      println("lower1: %f->%d->%f\n", fLonL, iLonL, fLonL2);				
      println("lower2: %f->%d->%f\n", fLatL, iLatL, fLatL2);				
      println("upper1: %f->%d->%f\n", fLonU, iLonU, fLonU2);				
      println("upper2: %f->%d->%f\n", fLatU, iLatU, fLatU2);				
    */
    float dX = (fLon-fLonL2)/m_fDLon;
    float dY = (fLat-fLatL2)/m_fDLat;
    //printf("dX: %f, dY: %f\n", dX, dY);
    float fVLL = m_afData[iIndex][iLatL][iLonL];
    float fVUL = m_afData[iIndex][iLatU][iLonL];
    float fVLU = m_afData[iIndex][iLatL][iLonU];
    float fVUU = m_afData[iIndex][iLatU][iLonU];
    float fV =  fNaN;
    if (isnan(fVLL) || isnan(fVUL) || isnan(fVLU) || isnan(fVUU) ||
        isinf(fVLL) || isinf(fVUL) || isinf(fVLU) || isinf(fVUU)) {
        fV = getValue(fLon, fLat, iIndex);
    } else {
    /*
      printf("LL: %f\n", fVLL);				
      printf("UL: %f\n", fVUL);				
      printf("LU: %f\n", fVLU);				
      printf("UU: %f\n", fVUU);
    */				
        float fVL = fVLL + dX*(fVUL - fVLL); 
        float fVU = fVLU + dX*(fVUU - fVLU);
        fV = fVL + dY*(fVU-fVL);
    }
    //printf("V: %f\n", fV);	
    return fV;
}

/*
	//------------------------------------------------------------------------------
	// main
	//   testing methods
	//
	public static void main(String asArgs[]) {
//			CliReader cr = new CliData("air_temp.clim", 15, -179.75f, 179.75f, 0.5f, -89.75f, 89.75f, 0.5f);
		CliData cd = new CliData("precip.clim", -179.75f, 0.5f, -15.0f, 40.0f, -89.75f, 0.5f, 30.0f, 60.0f);
		boolean bOK = cd.extractColumnRange(100,107);
		if (bOK) {
			//cr.getValueBiLin(-0.375f, 0.4f);
			/ *
			cr.xxx(-0.25f);
			cr.xxx(0.375f);
			cr.xxx(0.25f);
			cr.xxx(7.0f);
		* /
			float fLon = 152;
			float fLat = -4;
			float fA = cd.getValue(fLon, fLat);
			System.out.println(" "+fA);
			float fB = cd.getValueBiLin(fLon, fLat);
			System.out.println(" "+fB);
			
			/ *
			float f1 = cd.getValue(91.3f,23.75f);
			System.out.println(" "+f1);
			float f2 = cd.getValueBiLin(91.3f,23.75f);
			System.out.println(" "+f2);
			float f3 = cd.getValueBiLin(91.4f,23.75f);
			System.out.println(" "+f3);
			float f4 = cd.getValueBiLin(91.5f,23.75f);
			System.out.println(" "+f4);
			float f5 = cd.getValueBiLin(91.6f,23.75f);
			System.out.println(" "+f5);
			float f6 = cd.getValueBiLin(91.75f,23.75f);
			System.out.println(" "+f6);
			float fa = cd.getValue(91.75f,24.1f);
			System.out.println(" "+fa);
			* /
		}
	}
	
*/
