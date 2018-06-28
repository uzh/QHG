#ifndef __ICOHEADER_H__
#define __ICOHEADER_H__

#include "BufReader.h"
#include "BufWriter.h"

#include "icoutil.h"

// old 
#define KEY_START_ICO  "ICO"
#define KEY_START_OCT  "OCT"
#define KEY_START_TET  "TET"
// new 
#define KEY_START_ICO3 "ICO3"
#define KEY_START_OCT3 "OCT3"
#define KEY_START_TET3 "TET3"

#define KEY_ID    "ID:"
#define KEY_LEVEL "LEVEL:"
#define KEY_BOX   "BOX:"
#define KEY_END   "HEADER_END"

class IcoHeader {
public:
    IcoHeader(int iPolyType, gridtype lID, int iSubLevel, tbox &tBox);
    IcoHeader(int iPolyType, int iSubLevel, tbox &tBox);
    IcoHeader();

    int read(BufReader *pBR);
    int write(BufWriter *pBW);

    gridtype getPolyType() { return m_iPolyType;};
    gridtype getID() { return m_lID;};
    int  getSubLevel() { return m_iSubLevel;};
    void getBox(tbox &tBox);
protected:
    static gridtype createID();
    int m_iPolyType;
    gridtype m_lID;
    int  m_iSubLevel;
    tbox m_tBox;
};

#endif

