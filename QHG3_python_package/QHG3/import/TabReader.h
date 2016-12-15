/******************************************************************************
| TabReader
| abstract base class for the reading & processing of textfiles with structure
|   <longitude> <latitude> <item>*
| where it is assumed that successive longitude and latitude values always 
| differ by the same resepctive amount.
| TabReader only uses the lines whose longitude and latitude values are
| inside the boundaries given in the constructor.
| The method 
|  action(float fLon, float fLat, char *pLine)
| must be defined in derived classes.
\*****************************************************************************/
#ifndef __TABREADER_H__
#define __TABREADER_H__

#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "strutils.h"

const char DEF_SEP[]=" \t\n";

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DataExtractor
//   abstract base class
//
class DataExtractor {
public:
  virtual ~DataExtractor(){};
    //------------------------------------------------------------------------------
    // extractItem
    //   extract an item from sLine
    //
    virtual float extractItem(char *pLine, int iIndex=0)=0;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FieldExtractor
//   extract an item by field number
//
class FieldExtractor : public DataExtractor {
private:
    int   m_iField;
    char *m_pSep;
    
public:
    //------------------------------------------------------------------------------
    // constructor
    //   specify field number
    //
    FieldExtractor(int iField, const char *pSep=NULL) 
        : m_iField(iField) {
        if (pSep == NULL) {
            m_pSep = strdup(DEF_SEP);
        } else {
            m_pSep = strdup(pSep);
        }
    }

    //----------------------------------------------------------------------
    // destructor
    //
    virtual ~FieldExtractor() {
        if (m_pSep != NULL) {
            free(m_pSep);
        }
    }


    //------------------------------------------------------------------------------
    // extractItem
    //   extract the field specified by m_iField
    //
    float extractItem(char *pLine, int iIndex=0) {
        float fVal = fNaN;
        int iF = 0;
        char *pp;
        char *p = strtok_r(pLine, m_pSep, &pp);
        while ((p != NULL) && (iF < m_iField)) {
            p = strtok_r(NULL, m_pSep, &pp);
            iF++;
        }
        if (p != NULL) {
            fVal = strtof(trim(p), &pp); 
            if (*pp != '\0') {
                fVal = fNaN;
            }
        }
        return fVal;
    }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RangeExtractor
//   extract an item by range
//
class RangeExtractor : public DataExtractor {
private:
    unsigned int m_iColStart;
    unsigned int m_iColEnd;

public:	
    virtual ~RangeExtractor() {};
    //------------------------------------------------------------------------------
    // constructor
    //   specify Range
    //
    RangeExtractor(int iColStart, int iColEnd) {
        m_iColStart = iColStart;
        m_iColEnd   = iColEnd;
        if (m_iColStart < 0) {
            m_iColStart = 0;
        }
        if (m_iColEnd <= m_iColStart) {
            m_iColEnd = m_iColStart+1;
        }
    }
	
    //------------------------------------------------------------------------------
    // extractItem
    //   extract the field specified by m_iColStart, m_iColEnd
    //
    float extractItem(char *pLine, int iIndex=0) {
        float fVal = fNaN;
        if ((m_iColStart < strlen(pLine)) && (m_iColEnd <= strlen(pLine))) {
            char *q = pLine + m_iColEnd;
            *q = '\0';
            q = pLine+m_iColStart;


            char *pp;
            fVal = strtof(q, &pp); 
            if (*pp != '\0') {
                fVal = fNaN;
            }
        }        
        return fVal;
    }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MultiRangeExtractor
//   extract an item by range
//
class MultiRangeExtractor : public DataExtractor {
private:
    unsigned int m_iColStart;
    unsigned int m_iDataSize;
    unsigned int m_iNumItems;

public:	
    virtual ~MultiRangeExtractor() {};
    //------------------------------------------------------------------------------
    // constructor
    //   specify Range
    //
    MultiRangeExtractor(int iColStart, int iDataSize, int iNumItems) {
        m_iColStart = iColStart;
        m_iDataSize = iDataSize;
        m_iNumItems = iNumItems;

        if (m_iDataSize == 0) {
            m_iDataSize = 1;
        }
        if (m_iNumItems == 0) {
            m_iNumItems = 1;
        }
    }
	
    //------------------------------------------------------------------------------
    // extractItem
    //   extract the field specified by m_iColStart, m_iColEnd
    //
    float extractItem(char *pLine, int iIndex=0) {
        float fVal = fNaN;

        if ((m_iColStart < strlen(pLine)) && ( m_iColStart+(iIndex+1)*m_iDataSize <= strlen(pLine))) {
            
            char *q = pLine + m_iColStart+(iIndex+1)*m_iDataSize;
            char c = *q;
            *q = '\0';
            char *p = pLine+m_iColStart+iIndex*m_iDataSize;;

            
            char *pp;
            fVal = strtof(p, &pp); 
            if (*pp != '\0') {
                fVal = fNaN;
            }
            *q = c;
        }
        return fVal;
    }
};


// abstract base class for reading table files
class TabReader {
protected:
    FILE *m_fIn;
    float m_fDLon;
    float m_fDLat;
    DataExtractor *m_pde;
    // range from which to extract
    float m_fRangeLonMin;
    float m_fRangeLonMax;
    float m_fRangeLatMin;
    float m_fRangeLatMax;
	
public:
    TabReader(char *pFileName, 
              float fDLon, float fRangeLonMin, float fRangeLonMax, 
              float fDLat, float fRangeLatMin, float fRangeLatMax);
    virtual ~TabReader();

    void setLocationRange(float fLonMin, float fLonMax, float fLatMin, float fLatMax);
    bool extractField(int iField);
    bool extractCSVField(int iField);
    bool extractColumnRange(int iColStart, int iColEnd);
    bool extractMultiRange(int iColStart, int iDataSize, int iNumItems);
    bool processLine(char *pLine);

    float getLonMin() { return m_fRangeLonMin;};
    float getLonMax() { return m_fRangeLonMax;};
    float getDLon() { return m_fDLon; };
    float getLatMin() { return m_fRangeLatMin;};
    float getLatMax() { return m_fRangeLatMax;};
    float getDLat() { return m_fDLat; };
protected:
    virtual bool action(float fLon, float fLat, char *pLine) = 0;
    virtual bool extractLocation(char *pLine, float *pfLon, float *pfLat) = 0;

    bool locationCheck(float fLon, float fLat);

    bool extractData();
    

};


#endif
