#ifndef __SHPHEADER_H__
#define __SHPHEADER_H__

#include <stdio.h>


// the size of a shape buffer

#define SHP_HEADER_SIZE  100
#define SHP_MAGIC        0x0000270a
#define SHP_UNUSED_BYTES 20

class shpHeader {
public:
    shpHeader(FILE *fIn);

    int read();
    void display(const char *pCaption);

    int  getFileLenWords() const { return m_iFileLenWords;};
    long getFileLen() const { return m_iFileLen;};
    int  getVersion() const { return m_iVersion; };
    int  getShapeType() const { return m_iShapeType;};

    double getXMin() const { return m_dXMin; };
    double getXMax() const { return m_dXMax; };
    double getYMin() const { return m_dYMin; };
    double getYMax() const { return m_dYMax; };
    double getZMin() const { return m_dZMin; };
    double getZMax() const { return m_dZMax; };
    double getMMin() const { return m_dMMin; };
    double getMMax() const { return m_dMMax; };

protected:
    FILE *m_fIn;

    int  m_iFileLenWords;
    int  m_iFileLen;
    int  m_iVersion;
    int  m_iShapeType;

    mbr  m_mbr;

    double m_dXMin;
    double m_dXMax;
    double m_dYMin;
    double m_dYMax;
    double m_dZMin;
    double m_dZMax;
    double m_dMMin;
    double m_dMMax;
};

#endif

