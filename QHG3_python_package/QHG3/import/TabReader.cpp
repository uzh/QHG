#include "utils.h"
#include "TabReader.h"

//------------------------------------------------------------------------------
// constructor
//   sFileName - name of data file
//   fLonMin   - smallest longitude value
//   fLonMin   - largest  longitude value
//   fDLon     - longitude step size
//   fLatMin   - smallest latitude value
//   fLatMin   - largest  latitude value
//   fDLat     - latitude step size
TabReader::TabReader(char *pFileName, 
                     float fDLon, float fRangeLonMin, float fRangeLonMax, 
                     float fDLat, float fRangeLatMin, float fRangeLatMax) {
    m_fDLon       = fDLon;
    m_fDLat       = fDLat;
    // open file
    
    m_fIn = fopen(pFileName, "rt");
    if (m_fIn == NULL) {
        printf("Couldn't open file [%s]\n", pFileName);
    }
    m_pde=NULL;
    
    // default range: all
    m_fRangeLonMin = fRangeLonMin;
    m_fRangeLonMax = fRangeLonMax;
    m_fRangeLatMin = fRangeLatMin;
    m_fRangeLatMax = fRangeLatMax;
}

TabReader::~TabReader() {
    if (m_pde != NULL) {
        delete m_pde;
    }
}
	
//------------------------------------------------------------------------------
// setLocationRange
//   set range
//
void TabReader::setLocationRange(float fLonMin, float fLonMax, float fLatMin, float fLatMax) {
    m_fRangeLonMin = fLonMin;
    m_fRangeLonMax = fLonMax;
    m_fRangeLatMin = fLatMin;
    m_fRangeLatMax = fLatMax;
}
	
//------------------------------------------------------------------------------
// locationCheck
//   return true if coordinates within range
//
bool TabReader::locationCheck(float fLon, float fLat) {
    
    return ((m_fRangeLonMin <= fLon) && (fLon <= m_fRangeLonMax) &&
            (m_fRangeLatMin <= fLat) && (fLat <= m_fRangeLatMax));
}
	

//------------------------------------------------------------------------------
// extractField
//   extract data by field number
//
bool TabReader::extractField(int iField) {
    // create appropriate DataExtracor
    m_pde = new FieldExtractor(iField);
    return extractData();
}

//------------------------------------------------------------------------------
// extractCSVField
//   extract data by field number in a comma-separated file
//
bool TabReader::extractCSVField(int iField) {
    // create appropriate DataExtracor
    m_pde = new FieldExtractor(iField, ", \t\n");
    return extractData();
}

//------------------------------------------------------------------------------
// extractColumnRange
//   extract data by range
//
bool TabReader::extractColumnRange(int iColStart, int iColEnd) {
    m_pde = new RangeExtractor(iColStart, iColEnd);
    return extractData();
}
	
//------------------------------------------------------------------------------
// extractColumnRange
//   extract data by range
//
bool TabReader::extractMultiRange(int iColStart, int iDataSize, int iNumItems) {
    m_pde = new MultiRangeExtractor(iColStart, iDataSize, iNumItems);
    return extractData();
}
	
//------------------------------------------------------------------------------
// extractData
//   extract data using data extractor
//
bool TabReader::extractData() {
    bool bOK = false;
    char sLine[MAX_LINE];
    printf("Line ");
    if (m_pde != NULL) {
        if (m_fIn != NULL) {
            bOK = true;
            int iL = 0;
            char *p = sLine;
            // loop through lines
            while (bOK && (p != NULL)) {
                p = fgets(sLine, 1024, m_fIn);
                if (p != NULL) {
                    bOK = processLine(sLine);
                }
                iL++;
                if (iL%100 == 0) {
                    printf("%6d\b\b\b\b\b\b", iL);
                }
            }
            printf("\nprocessed %d lines\n", iL);
        } else {
            printf("File not open");
        }
    } else {
        printf("Data Extractor not defined. Call extractField() or extractColumnRange()");
    } 
    return bOK;
}

//------------------------------------------------------------------------------
// processLine
//   get longitude and latitude, pass string to DataExtractor to get value
//
bool TabReader::processLine(char *pLine) {
    bool bOK = false;
    char sLine[MAX_LINE];
    strcpy(sLine, pLine);
    
    float fLon;
    float fLat;
    bOK = extractLocation(pLine, &fLon, &fLat);
    if (bOK) {
        if (locationCheck(fLon, fLat)) {
            bOK = action(fLon, fLat, sLine);
        } else {
            bOK = true;
        }
    } else {
        printf("Bad Location:%s\n", pLine);
    }
    return bOK;
}
	
	


