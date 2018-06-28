#include <stdio.h>
#include <string.h>

#include "types.h"
#include "strutils.h"
#include "LifeTile.h"

static int s_Dirs[NUM_DIR][2] = {
    // east
    { 1,  0},
    // north east
    { 1,  1},
    // north
    { 0,  1},
    // north west
    {-1,  1},
    // west
    {-1,  0},
    // south west
    {-1, -1},
    // south
    { 0, -1},
    // south east
    { 1, -1},
};

    
//----------------------------------------------------------------------------
// createInstance
//   iID    tile ID
//   iW     tile width
//   iH     tile height
//   iHalo  halo size
//
LifeTile *LifeTile::createInstance(int iID, int iW, int iH, int iHalo) {
    LifeTile *pLT = new LifeTile();
    int iResult = pLT->init(iID, iW, iH, iHalo);
    if (iResult != 0) {
        delete pLT;
        pLT = NULL;
    }
    return pLT;
}


//----------------------------------------------------------------------------
// destructor
//
LifeTile::~LifeTile() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < m_iH+2*m_iHalo; j++) {
            delete[] m_ppField[i][j];
        }
        delete[] m_ppField[i];
    }
    delete[] m_ppField;
}


//----------------------------------------------------------------------------
// getSendSize
//   return the size of the data for the specified direction
//
int LifeTile::getSendSize(int iDir) {
    int iSendSize = 0;
    switch (iDir) {
        // east west
    case DIR_E:
    case DIR_W:
        iSendSize = m_iHalo*m_iH;
        break;
        // north, south
    case DIR_N:
    case DIR_S:
        iSendSize = m_iHalo*m_iW;
        break;
    default:
        iSendSize = m_iHalo*m_iHalo;
    }
    return iSendSize;
}

//----------------------------------------------------------------------------
// showInterior
//  show contents of both arrays (debug)
//
void LifeTile::showInterior(bool bShowHalo) {
    int iD = bShowHalo?0:m_iHalo;
    for (int ic= 0; ic < 2; ic++) {
        printf("+------------------%s------------------------------------+\n", (ic== m_iCur)?"ccc":"---");
        for (int i = iD; i < m_iH+2*m_iHalo-iD; i++) {
            printf("i%02d: ",i);
            for (int j = iD; j < m_iW+2*m_iHalo-iD; j++) {
                printf("%d ", m_ppField[ic][i][j]);
            }
            printf("\n");
        }
    }
     
    printf("+---------------------------------------------------------+\n");

}

//----------------------------------------------------------------------------
// setInitialPattern
//  put data passed in pData to the current array
//
int LifeTile::setInitialPattern(uchar *pData) {
    int iResult = 0;
    for (int i = m_iHalo; i < m_iH+m_iHalo; i++) {
        pData = getMem(m_ppField[m_iCur][i]+m_iHalo, pData, m_iW*sizeof(uchar));
    }
    
    /*
    printf("[%d] LifeTile (%dx%d) got pattern with halo\n", m_iID, m_iW, m_iH);
    showInterior(true);
    */
    return iResult;
}


//----------------------------------------------------------------------------
// getFinalPattern
//   fill buffer pData with contents of current array
//
int LifeTile::getFinalPattern(uchar *pData) {
    int iResult = 0;
    /*
    printf("[%d] LifeTile has final\n", m_iID);
    showInterior(false);
    */
    for (int i = m_iHalo; i < m_iH+m_iHalo; i++) {
        pData = putMem(pData, m_ppField[m_iCur][i]+m_iHalo, m_iW*sizeof(uchar));
    }
    return iResult;
}
 

//----------------------------------------------------------------------------
// setSendData
//
int LifeTile::setSendData(int iDir, uchar *pData) {
    int iResult = 0;
    switch (iDir) {
        // east west
    case DIR_E:
        for (int h = m_iHalo; h < m_iH+m_iHalo; h++) {
            pData = putMem(pData, &(m_ppField[m_iCur][h][m_iW]), m_iHalo*sizeof(uchar));
        }
        break;

    case DIR_NE:
        for (int h = 0; h < m_iHalo; h++) {
            pData = putMem(pData, &(m_ppField[m_iCur][h+m_iHalo][m_iW]), m_iHalo*sizeof(char));
        }
        break;

    case DIR_N:
        for (int h = 0; h < m_iHalo; h++) {
            pData  = putMem(pData, &(m_ppField[m_iCur][h+m_iHalo][m_iHalo]), m_iW*sizeof(uchar));
        }
        break;

    case DIR_NW:
        for (int h = 0; h < m_iHalo; h++) {
            pData = putMem(pData, &(m_ppField[m_iCur][h+m_iHalo][m_iHalo]), m_iHalo*sizeof(char));
        }
        break;

    case DIR_W:
        for (int h = m_iHalo; h < m_iH+m_iHalo; h++) {
            pData = putMem(pData, &(m_ppField[m_iCur][h][m_iHalo]), m_iHalo*sizeof(uchar));
        }
        break;

    case DIR_SW:
        for (int h = 0; h < m_iHalo; h++) {
            pData = putMem(pData, &(m_ppField[m_iCur][m_iH+h][m_iHalo]), m_iHalo*sizeof(char));
        }
        break;
        
    case DIR_S:
        for (int h = 0; h < m_iHalo; h++) {
            pData = putMem(pData, &(m_ppField[m_iCur][m_iH+h][m_iHalo]), m_iW*sizeof(uchar));
        }
        break;
        
    case DIR_SE:
        for (int h = 0; h < m_iHalo; h++) {
            pData = putMem(pData, &(m_ppField[m_iCur][m_iH+h][m_iW]), m_iHalo*sizeof(char));
        }
        break;
        
    }
    return iResult;
}


//----------------------------------------------------------------------------
// getRecvData
//
int LifeTile::getRecvData(int iDir, const uchar *pData) {
    int iResult = 0;

    switch(iDir) {
    case DIR_E:
        for (int h = m_iHalo; h < m_iH+m_iHalo; h++) {
            pData = getMem(&(m_ppField[m_iCur][h][m_iW+m_iHalo]), pData, m_iHalo*sizeof(uchar));
        }
        break;
    case DIR_NE:
        for (int h = 0; h < m_iHalo; h++) {
            pData = getMem(&(m_ppField[m_iCur][h][m_iW+m_iHalo]), pData, m_iHalo*sizeof(char));
        }
        break;
    case DIR_N:
        for (int h = 0; h < m_iHalo; h++) {
            pData  = getMem(&(m_ppField[m_iCur][h][m_iHalo]), pData, m_iW*sizeof(uchar));
        }
        break;

    case DIR_NW:
        for (int h = 0; h < m_iHalo; h++) {
            pData = getMem(&(m_ppField[m_iCur][h][0]), pData, m_iHalo*sizeof(char));
        }
        break;

    case DIR_W:
        for (int h = m_iHalo; h < m_iH+m_iHalo; h++) {
            pData = getMem(&(m_ppField[m_iCur][h][0]), pData, m_iHalo*sizeof(uchar));
        }
        break;

    case DIR_SW:
        for (int h = m_iHalo; h < 2*m_iHalo; h++) {
            pData = getMem(&(m_ppField[m_iCur][m_iH+h][0]), pData, m_iHalo*sizeof(char));
        }
        break;

        
    case DIR_S:
        for (int h = m_iHalo; h < 2*m_iHalo; h++) {
            pData = getMem(&(m_ppField[m_iCur][m_iH+h][m_iHalo]), pData, m_iW*sizeof(uchar));
        }
        break;
        
    case DIR_SE:
        for (int h = m_iHalo; h < 2*m_iHalo; h++) {
            pData = getMem(&(m_ppField[m_iCur][m_iH+h][m_iHalo+m_iW]), pData, m_iHalo*sizeof(char));
        }
        break;
        
    }

    return iResult;
}


//----------------------------------------------------------------------------
// constructor
//
LifeTile::LifeTile() 
    : m_iID(-1),
      m_iW(-1),
      m_iH(-1),
      m_iHalo(-1),
      m_iCur(0),
      m_ppField(NULL) {

}


//----------------------------------------------------------------------------
// init
//
int LifeTile::init(int iID, int iW, int iH, int iHalo) {
    int iResult = -1;
    m_iID   = iID;
    m_iW    = iW;
    m_iH    = iH;
    m_iHalo = iHalo;
    m_iCur  = 0;

    m_ppField = new uchar**[2];
    for (int c = 0; c < 2; c++) {
        m_ppField[c] = new uchar*[m_iH+ 2*m_iHalo];
        for (int i = 0; i < m_iH+ 2*m_iHalo; i++) {
            m_ppField[c][i] = new uchar[m_iW+2*m_iHalo];
            memset(m_ppField[c][i], 0, (m_iW+2*m_iHalo)*sizeof(uchar));
        }
    }
    /*
    printf("[%d] LifeTile initialized\n", m_iID);
    showInterior(true);
    */

    iResult = 0;
    
    return iResult;
}


//----------------------------------------------------------------------------
// doStep
//
int LifeTile::doStep() {
    int iResult = -1;
    //    printf("[%d] before step\n", m_iID);
    //    showInterior(true);
    // loop inside cells
    for (int i = m_iHalo; i < m_iH+m_iHalo; i++) {
        for (int j = m_iHalo; j < m_iW+m_iHalo; j++) {
            //   count neighbors in field m_iCur
            int iC = 0;
            for (int k = 0; k < NUM_DIR; k++) {
                iC += m_ppField[m_iCur][i+s_Dirs[k][1]][j+s_Dirs[k][0]];
            }

            if ((iC < 2) || (iC > 3)) {
                // death
                m_ppField[1-m_iCur][i][j] = 0;
            } else if (iC == 3) {
                // birth
                m_ppField[1-m_iCur][i][j] = 1;
            } else {
                // no change
                m_ppField[1-m_iCur][i][j] = m_ppField[m_iCur][i][j]; // nothing changes
            }
        }
    }    
    m_iCur = 1 - m_iCur;
    return iResult;
}


//----------------------------------------------------------------------------
// writeCurrent
//
int LifeTile::writeCurrent(const char *pTemplate) {
    int iResult = -1;

    char sFile[256];
    sprintf(sFile, pTemplate, m_iID);
    printf("Wrtiting to [%s]\n", sFile);
    FILE *fOut = fopen(sFile, "wb");
    if (fOut != NULL) {
        fwrite(&m_iW, sizeof(int), 1, fOut);
        fwrite(&m_iH, sizeof(int), 1, fOut);
        
        iResult = 0;
        for (int i = m_iHalo; (iResult == 0) && (i < m_iH+m_iHalo); i++) {
            int iWritten = fwrite(m_ppField[m_iCur][i]+m_iHalo, sizeof(uchar), m_iW, fOut);
            if (iWritten != m_iW) {
                iResult = -1;
                printf("Couldn't write line\n");
            }
        }
        
        fclose(fOut);
        if (iResult != 0) {
            remove(sFile);
        }
    } else {
        printf("[%d] Couldn't open [%s] for writing\n", m_iID, sFile);
    }
    return iResult;
}
