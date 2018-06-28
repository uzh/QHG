/** ***************************************************************************\
*   \file   PNGImage.h
*   \author jody weissmann
*   \brief  Header file for class PNGImage
*
*   PNGImage represents a PNG image
*   Its methods allow reading from and writing to file, as well as 
*   direct manipulation of the buffer (using the GetRow()) 
*** ***************************************************************************/
#ifndef __PNGIMAGE_H__
#define __PNGIMAGE_H__

#include "png.h"
#include "types.h"



class PNGImage {

public:
    PNGImage(int iWidth, int iHeight, int iBitDepth=8, int iColorType=PNG_COLOR_TYPE_RGB_ALPHA);
    PNGImage();
    virtual ~PNGImage();
    
    bool createPNGFromData(double ***aadData, const char *pOutputFile); 
    bool createPNGFromData(uchar  **aaucData, const char *pOutputFile); 
    bool isInitialized() { return m_bInitialized; };
    void setPalette(uchar **pPaletteEntries, int iNumPaletteEntries, bool bUseAlpha=false); 
    void setText(const char **ppKeys, const char **ppValues, int iNumEntries);
    bool writePNGFile(const char *pOutputFile);
    bool readPNGFile(const char *pInputFile);
    
    int getWidth() { return m_iWidth;};
    int getHeight() { return m_iHeight;};
    int getColorType() { return m_iColorType;};
    int getBitDepth() { return m_iBitDepth;};
    int getByteWidth() { return m_iByteWidth;};
    int getNumChannels();
    uchar *getRow(int iIndex) { return m_pucRows[iIndex];};
    void setRow(int iIndex, uchar *ucRow);
    void setRow(int iIndex, int *piRow);
    int getText(char ***ppKeys, char ***ppValues);

    void setVerbose(bool bVerbose) { m_bVerbose=bVerbose;};
protected:
    void createRows(int iRowWidth);
    void deleteRows(int iHeight);

    bool testSettings();
    bool fillData(double ***aadData);   
    bool fillData16(double ***aadData);   
    void fillPalette();
    bool initializeWritePNG(FILE *fOut);
    
    void cleanWriteStuff();
    void cleanReadStuff();

    int  m_iWidth;
    int  m_iHeight;
    int  m_iBitDepth;
    int  m_iColorType;
    int  m_iPixSize;
    bool m_bHasAlpha;
    int  m_iNumChannels;
    int  m_iByteWidth;
    png_structp  m_pPNGWrite;
    png_infop    m_pPNGWriteInfo;
    png_structp  m_pPNGRead;
    png_infop    m_pPNGReadInfo;
    
    uchar *m_pTrans;
    uchar **m_pPaletteEntries;
    int  m_iNumPaletteEntries;
    uchar **m_pucRows;

    int    m_iNumEntries;
    png_text *m_apWriteTexts;
    int    m_iNumRead;
    png_textp m_pReadTexts;
    char  **m_apKeys;
    char  **m_apValues;

    bool m_bInitialized;
    bool m_bVerbose;
};


 



#endif
