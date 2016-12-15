#include <stdlib.h>
#include <string.h>
#include "png.h"
#include "PNGImage.h"

const int HEADER_SIZE = 8;

//-----------------------------------------------------------------------------
// constructor
//
PNGImage::PNGImage(int iWidth, int iHeight, int iBitDepth/*=8*/, int iColorType/*=PNG_COLOR_RGB_ALPHA*/)
    :   m_iWidth(iWidth),
        m_iHeight(iHeight),
        m_iBitDepth(iBitDepth),
        m_iColorType(iColorType),
        m_iPixSize(0),
        m_bHasAlpha(false),
        m_iNumChannels(0),
        m_iByteWidth(0),
        m_pPNGWrite(NULL),
        m_pPNGWriteInfo(NULL),
        m_pPNGRead(NULL),
        m_pPNGReadInfo(NULL),
        m_pTrans(NULL),
        m_pPaletteEntries(NULL),
        m_iNumPaletteEntries(0),
        m_pucRows(NULL),
        m_iNumEntries(0),
        m_apWriteTexts(NULL),
        m_apKeys(NULL),
        m_apValues(NULL),
        m_bInitialized(false),
        m_bVerbose(false) {
    
    m_bInitialized = testSettings();
    
    if (m_bInitialized) {
        createRows(m_iByteWidth*m_iWidth);
        //        printf("[PNGImage] row size: %d\n", m_iByteWidth*m_iWidth);
    } else {
        printf("[PNGImage] Init problem\n");
    }  
};

//-----------------------------------------------------------------------------
// constructor
//
PNGImage::PNGImage()
:   m_iWidth(-1),
    m_iHeight(-1),
    m_iBitDepth(-1),
    m_iColorType(-1),
    m_iPixSize(0),
    m_bHasAlpha(false),
    m_iNumChannels(0),
    m_iByteWidth(0),
    m_pPNGWrite(NULL),
    m_pPNGWriteInfo(NULL),
    m_pPNGRead(NULL),
    m_pPNGReadInfo(NULL),
    m_pTrans(NULL),
    m_pPaletteEntries(NULL),
    m_iNumPaletteEntries(0),
    m_pucRows(NULL),
    m_iNumEntries(0),
    m_apWriteTexts(NULL),
    m_apKeys(NULL),
    m_apValues(NULL),
    m_bInitialized(false),
    m_bVerbose(false) {
}


//-----------------------------------------------------------------------------
// destructor
//
PNGImage::~PNGImage() {
    
    deleteRows(m_iHeight);

    if (m_pTrans != NULL) {
        delete[] m_pTrans;
    }
    
    cleanReadStuff();
    cleanWriteStuff();

    if (m_apWriteTexts != NULL) {
        delete[] m_apWriteTexts;
    }
}

//-----------------------------------------------------------------------------
// cleanWriteStuff
//
void PNGImage::cleanWriteStuff() {
    if (m_pPNGWrite != NULL) {
        //        printf("writeinfo: %p \n", m_pPNGWriteInfo);
        if (m_pPNGWriteInfo != NULL) {
            png_destroy_info_struct(m_pPNGWrite, &m_pPNGWriteInfo);
        }
        png_destroy_write_struct(&m_pPNGWrite, &m_pPNGWriteInfo);
    }   
    m_pPNGWrite = NULL;
    m_pPNGWriteInfo = NULL;
}


//-----------------------------------------------------------------------------
// cleanReadStuff
//
void PNGImage::cleanReadStuff() {
    if (m_pPNGRead != NULL) {
        png_destroy_read_struct(&m_pPNGRead, &m_pPNGReadInfo, NULL);
    }   
    m_pPNGRead = NULL;

    if (m_apKeys != NULL) {
        for (int i = 0; i < m_iNumRead; ++i) {
            if (m_apKeys[i] != NULL) {
                free(m_apKeys[i]);
            }
        }
        delete[] m_apKeys;
    }
    m_apKeys = NULL;

    if (m_apValues != NULL) {
        for (int i = 0; i < m_iNumRead; ++i) {
            if (m_apValues[i] != NULL) {
                free(m_apValues[i]);
            }
        }
        delete[] m_apValues;
    }
    m_apValues = NULL;

}

//-----------------------------------------------------------------------------
// createRows
//
void PNGImage::createRows(int iRowWidth) {
    if (m_pucRows != NULL) {
        deleteRows(m_iHeight);
    }
    
    m_pucRows = new uchar *[m_iHeight];
    if (m_pucRows != NULL) {
        m_bInitialized = true;
        for (int iRow = 0; m_bInitialized && (iRow < m_iHeight); iRow++) {
	        m_pucRows[iRow] = new uchar[iRowWidth];
            if (m_pucRows[iRow] == NULL) {
                m_bInitialized = false;
            }
        }
    } 
}

//-----------------------------------------------------------------------------
// deleteRows
//
void PNGImage::deleteRows(int iHeight) {
    if (m_pucRows != NULL) {
        for (int iRow = 0; iRow < iHeight; iRow++) {
            if (m_pucRows[iRow] != NULL) {
                delete[] m_pucRows[iRow];
            }
        }
        delete[] m_pucRows;
    }
    m_pucRows = NULL;
}

//-----------------------------------------------------------------------------
// testSettings
//
bool PNGImage::testSettings() {
    m_bInitialized = false;
    m_bHasAlpha = false;

    if (m_bVerbose) {
        printf("iColorType: %d, bitDepth:%d\n", m_iColorType, m_iBitDepth);
    }

    if (m_iColorType == PNG_COLOR_TYPE_GRAY) {
        m_bInitialized = (m_iBitDepth ==  1) ||
                         (m_iBitDepth ==  2) ||
                         (m_iBitDepth ==  4) ||    
                         (m_iBitDepth ==  8) ||
                         (m_iBitDepth == 16);
        m_iNumChannels = 1;       
    } else if (m_iColorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
        m_bInitialized = (m_iBitDepth ==  8) ||
                         (m_iBitDepth == 16);
        m_bHasAlpha = true;
        m_iNumChannels = 2;       
    } else if (m_iColorType == PNG_COLOR_TYPE_PALETTE) {
        m_bInitialized = (m_iBitDepth ==  1) ||
                         (m_iBitDepth ==  2) ||
                         (m_iBitDepth ==  4) ||    
                         (m_iBitDepth ==  8);
        // currently, no palettes
        // later probably to be preferred
        //m_bInitialized = false;    
        m_iNumChannels = 1;    
    } else if (m_iColorType == PNG_COLOR_TYPE_RGB) {
        m_bInitialized = (m_iBitDepth ==  8) ||
                         (m_iBitDepth == 16);
        m_iNumChannels = 3;
    } else if (m_iColorType == PNG_COLOR_TYPE_RGB_ALPHA) {
        m_bInitialized = (m_iBitDepth ==  8) ||
                         (m_iBitDepth == 16);
        m_iNumChannels = 4;    
    }
                   
    if (m_bInitialized) {
        int iNumRowBits = m_iNumChannels*m_iBitDepth;
        
        m_iByteWidth = ((iNumRowBits%8==0)?0:1) + iNumRowBits/8;
        m_iPixSize = 1 << m_iBitDepth;

        if (m_bVerbose) {
            printf("BitDepth    : %d\n", m_iBitDepth);
            printf("NumChannels : %d\n", m_iNumChannels);
            printf("NumRowBits  : %d\n", iNumRowBits);
            printf("ByteWidth   : %d\n", m_iByteWidth);
            printf("PixSize     : %d\n", m_iPixSize);
        }
    }

    return m_bInitialized;
}    

//-----------------------------------------------------------------------------
// getNumChannels
//
int PNGImage::getNumChannels() {
    if (m_iNumChannels == 0) {
        switch(m_iColorType) {
        case PNG_COLOR_TYPE_GRAY:
            m_iNumChannels = 1;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            m_iNumChannels = 2;
            break;
        case PNG_COLOR_TYPE_PALETTE:
            m_iNumChannels = 1;
            break;
        case PNG_COLOR_TYPE_RGB:
            m_iNumChannels = 3;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            m_iNumChannels = 4;
            break;
        }
    }
    return m_iNumChannels;
}

//-----------------------------------------------------------------------------
// setPalette
//
void PNGImage::setPalette(uchar * *pPaletteEntries, int iNumPaletteEntries, bool bUseAlpha) {
    m_pPaletteEntries = pPaletteEntries;
    m_iNumPaletteEntries = iNumPaletteEntries;
    m_bHasAlpha = bUseAlpha;

}    
    
//-----------------------------------------------------------------------------
// setText
//
void PNGImage::setText(const char **ppKeys, const char **ppValues, int iNumEntries) {
    m_iNumEntries = iNumEntries;
    if (m_apWriteTexts != NULL) {
        delete[] m_apWriteTexts;
    }
    if (m_iNumEntries > 0) {
        m_apWriteTexts = new png_text[m_iNumEntries];
        for (int i = 0; i < m_iNumEntries; ++i) {
            m_apWriteTexts[i].key  = (char *) ppKeys[i];
            m_apWriteTexts[i].text = (char *) ppValues[i];
            m_apWriteTexts[i].compression=PNG_TEXT_COMPRESSION_NONE;
        }
    }
}    

    
//-----------------------------------------------------------------------------
// getText
//  allocates and filles char * arrays.
//  user must delete[] these arrays after use
//
int PNGImage::getText(char ***pppKeys, char ***pppValues) {
    
    if (m_iNumRead > 0) {
        if (m_apKeys == NULL) {
        
            m_apKeys   = new char *[m_iNumRead];
            m_apValues = new char *[m_iNumRead];
            for (int i = 0; i < m_iNumRead; ++i) {
                m_apKeys[i]   = strdup(m_pReadTexts[i].key);
                m_apValues[i] = strdup(m_pReadTexts[i].text);
            }
        }
        *pppKeys = m_apKeys;
        *pppValues = m_apValues;

    }

    return m_iNumRead;
}
    
//-----------------------------------------------------------------------------
// createPNGFromData
//
// aadData:
//   first index  : y-coord
//   second index : x-coord
//   third index  : channel (gray:0, gray+alpha:0-1, rgb:0-2, rgba:0-3, pal:0 
// values should be >= 0 and < 1
bool PNGImage::createPNGFromData(double ***aadData, const char *pOutputFile) {
    
    bool bOK = false;
    if (m_bInitialized) {
        if (m_iBitDepth < 16) {
            bOK = fillData(aadData);
        } else {
            bOK = fillData16(aadData);
        }
        if (bOK) {
            bOK = writePNGFile(pOutputFile);
        }   
    }
    return bOK;
}
   
bool PNGImage::createPNGFromData(uchar **aaucData, const char *pOutputFile) {
    bool bOK = false;
    if (m_bInitialized) {
        for (int iRow = 0; iRow < m_iHeight; iRow++) {
            uchar * p = m_pucRows[iRow];
            memcpy(p, aaucData[iRow],m_iWidth*m_iByteWidth);
        }
        bOK = true;
    }
    if (bOK) {
        bOK = writePNGFile(pOutputFile);
    }   
    return bOK;
}

//-----------------------------------------------------------------------------
// writePNGFile
//
bool PNGImage::writePNGFile(const char *pOutputFile) {
    bool bOK = true;

    FILE *fOut = fopen(pOutputFile, "wb");
    if (fOut != NULL) {
        bOK = initializeWritePNG(fOut);
        if (bOK) {

	    if (setjmp(png_jmpbuf(m_pPNGWrite))) {
		        printf("[write_png_file] Error during writing bytes");
                bOK = false;
            } else {
                png_write_info(m_pPNGWrite, m_pPNGWriteInfo);
        //                    printf("Wrote preliminary image data.\n");

                // write name, number of agents, step, selector, date

                if (m_apWriteTexts != NULL) {
                    png_set_text(m_pPNGWrite, m_pPNGWriteInfo, m_apWriteTexts, m_iNumEntries);
                }

                if (m_pPNGWriteInfo == NULL) {
                    printf("Dummy PNGWriteINfo is NULL\n");
                }
                if (m_pucRows == NULL) {
                    printf("Dummy pucRows is NULL\n");
                }
                png_write_image(m_pPNGWrite, m_pucRows);

                png_write_end(m_pPNGWrite, m_pPNGWriteInfo);
        //                    printf("Called end to data.\n");
            }
        } else {
            printf("[PNGImage] error during InitializeWritePNG\n");
            bOK = false;
        }
        fclose(fOut);
    } else {
        printf("[PNGImage] couldn't open %s for writeng\n", pOutputFile);
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// readPNGFile
//
bool PNGImage::readPNGFile(const char *pInputFile) {
    bool bOK = true;
	uchar sHeader[HEADER_SIZE];	// 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(pInputFile, "rb");
	if (fp != NULL) {
	    fread(sHeader, 1, HEADER_SIZE , fp);
	    if (png_sig_cmp(sHeader, 0, HEADER_SIZE) == 0) {


	        /* initialize stuff */
                cleanReadStuff();
	        m_pPNGRead = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	        if (m_pPNGRead != NULL) {

	            m_pPNGReadInfo = png_create_info_struct(m_pPNGRead);
	            if (m_pPNGReadInfo != NULL) {

	                if (setjmp(png_jmpbuf(m_pPNGRead))) {
                            printf("[read_png_file 2] Error during init_io");
                            bOK = false;
                        } else {

	                    png_init_io(m_pPNGRead, fp);
	                    png_set_sig_bytes(m_pPNGRead, HEADER_SIZE );

	                    png_read_info(m_pPNGRead, m_pPNGReadInfo);

                            m_iWidth  = png_get_image_width(m_pPNGRead, m_pPNGReadInfo); 
                            m_iHeight = png_get_image_height(m_pPNGRead, m_pPNGReadInfo); 
                            m_iColorType = png_get_color_type(m_pPNGRead, m_pPNGReadInfo);
                            m_iBitDepth = png_get_bit_depth(m_pPNGRead, m_pPNGReadInfo);
                            /* deprecated
                            m_iWidth     = m_pPNGReadInfo->width;
	                    m_iHeight    = m_pPNGReadInfo->height;
	                    m_iColorType = m_pPNGReadInfo->color_type;
	                    m_iBitDepth  = m_pPNGReadInfo->bit_depth;
                            */
                            printf("Rowbytes:%u\n",(uint) png_get_rowbytes(m_pPNGRead, m_pPNGReadInfo)); 
                            //	                    number_of_passes2 = png_set_interlace_handling(png_ptr2);
	                    png_read_update_info(m_pPNGRead, m_pPNGReadInfo);

                            /* read file */
	                    if (setjmp(png_jmpbuf(m_pPNGRead))) {
                                printf("[read_png_file] Error during read_image");
                            } else {
                                uint iRowBytes = png_get_rowbytes(m_pPNGRead, m_pPNGReadInfo);
                                if (m_bVerbose) {
                                    printf("[PNGImage::Read] w:%d, h:%d, ct:%d. BD:%d, row size: %u\n", m_iWidth, m_iHeight, m_iColorType, m_iBitDepth, iRowBytes);
                                }
                                createRows(iRowBytes);

	                        png_read_image(m_pPNGRead, m_pucRows);
                                
                                png_read_end(m_pPNGRead, m_pPNGReadInfo);

                                m_iNumRead = png_get_text(m_pPNGRead, m_pPNGReadInfo, &m_pReadTexts, &m_iNumRead);
                                
                                if (m_bVerbose) {
                                    printf("Found %d chunks\n", m_iNumRead);
                                    for (int i = 0; i < m_iNumRead; ++i) {
                                        printf("%s -> %s\n", m_pReadTexts[i].key, m_pReadTexts[i].text);
                                    }
                                }
                            }
                        }
                    } else {
                        printf("[read_png_file] png_create_info_struct failed");
                        bOK = false;
                    }
                } else {
                    printf("[read_png_file] png_create_read_struct failed");
                    bOK = false;
                }
            } else {
                printf("[read_png_file] File %s is not recognized as a PNG file", pInputFile);
            bOK = false;
        }        
        fclose(fp);
    } else {
		printf("[read_png_file] File %s could not be opened for reading", pInputFile);
        bOK = false;
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// setRow
//
void PNGImage::setRow(int iIndex, uchar * ucRow) {
    memcpy(m_pucRows[iIndex], ucRow, m_iWidth*m_iByteWidth);
}
        
//-----------------------------------------------------------------------------
// setRow
//
void PNGImage::setRow(int iIndex, int *piRow) {
    memcpy(m_pucRows[iIndex], piRow, m_iWidth*m_iByteWidth);
}
        
//-----------------------------------------------------------------------------
// fillData
//
bool PNGImage::fillData(double ***aadData) {
    for (int iRow = 0; iRow < m_iHeight; iRow++) {
        uchar * p = m_pucRows[iRow];
        uchar iVal = 0; 
        int iCount = 0;
        for (int iCol = 0; iCol < m_iWidth; iCol++) {

            for (int iCh = 0; iCh < m_iNumChannels; iCh++) {
                iVal <<= m_iBitDepth;
                double dVal = aadData[iRow][iCol][iCh];
                if (dVal >= 1) {
                    dVal = 0.9999999;
                }
                iVal += (uchar)(dVal*m_iPixSize);
                iCount+=m_iBitDepth;
                if (iCount == 8) {
                    *p = iVal;
                    p++;
//                    printf("%02x ", iVal);
                    iVal = 0;
                    iCount = 0;
                }
            }
//            printf("|");
        }
        if (iCount != 0) {
           *p = iVal;
//            printf("%02x", iVal);
            iVal = 0;
            iCount = 0;
        }
//        printf("\n");
    }
    return true;
}

//-----------------------------------------------------------------------------
// fillData16
//
bool PNGImage::fillData16(double ***aadData) {
    for (int iRow = 0; iRow < m_iHeight; iRow++) {
        uchar * p = m_pucRows[iRow];
        for (int iCol = 0; iCol < m_iWidth; iCol++) {
            for (int iCh = 0; iCh < m_iNumChannels; iCh++) {
                int iVal = (int)(aadData[iRow][iCol][iCh]*m_iPixSize);
                *p = (uchar)(iVal >> 8);
                p++;
//                printf("%02x ", iVal >>8);
                *p = (uchar)(iVal & 0xff);
                p++;
//                printf("%02x ", iVal & 0xff);
            }
//            printf("|");
        }
//        printf("\n");
    }
    return true;
}

//-----------------------------------------------------------------------------
// initializeWritePNG
//
bool PNGImage::initializeWritePNG(FILE *fOut) {
    bool bOK = false;
    /* Initialize image variables */

    cleanWriteStuff();

    m_pPNGWrite = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
                                          (png_voidp)NULL,
			                      		  (png_error_ptr)NULL, 
                                          (png_error_ptr)NULL);
    if (m_pPNGWrite != NULL) {
        m_pPNGWriteInfo = png_create_info_struct(m_pPNGWrite);
        //        printf("writeinfo: %p \n", m_pPNGWriteInfo);
        if (m_pPNGWriteInfo != NULL) {
            //            if (!setjmp(m_pPNGWrite->jmpbuf)) {
            if (!setjmp(png_jmpbuf(m_pPNGWrite))) {

                png_init_io(m_pPNGWrite, fOut);
                
                png_set_IHDR(m_pPNGWrite, m_pPNGWriteInfo, 
                             m_iWidth, m_iHeight, 
                             m_iBitDepth, m_iColorType,
		                     PNG_INTERLACE_NONE,
		                     PNG_COMPRESSION_TYPE_DEFAULT, 
                             PNG_FILTER_TYPE_DEFAULT);
                if (m_pPaletteEntries != NULL) {
                    fillPalette();
                }
                
                bOK = true;
            } else {
                // write error
                png_destroy_write_struct(&m_pPNGWrite, &m_pPNGWriteInfo);
                bOK = false;
                printf("write error");
            }
        } else {
        	png_destroy_write_struct(&m_pPNGWrite,(png_infopp)NULL);
            printf("couldn't allocate info structure\n");
        }
    } else {
	  printf("couldn't allocate write structure\n");
    } 

    return bOK;
}


//-----------------------------------------------------------------------------
// fillPalette
//
void PNGImage::fillPalette() {
    //printf("hasalpha : %d\n",m_bHasAlpha);    
    if (m_bHasAlpha) {
        m_pTrans = new uchar[m_iNumPaletteEntries];
    }
    
    png_colorp pPalette = (png_colorp)png_malloc(m_pPNGWrite, m_iNumPaletteEntries* sizeof (png_color));
    for (int i = 0; i < m_iNumPaletteEntries; i++) {
        pPalette[i].red   = m_pPaletteEntries[i][0];
        pPalette[i].green = m_pPaletteEntries[i][1];
        pPalette[i].blue  = m_pPaletteEntries[i][2];
        if (m_bHasAlpha) {
            m_pTrans[i]      = m_pPaletteEntries[i][3];
        }
    }
    png_set_PLTE(m_pPNGWrite, m_pPNGWriteInfo, pPalette, m_iNumPaletteEntries);
    if (m_bHasAlpha) {
        png_set_tRNS(m_pPNGWrite, m_pPNGWriteInfo, m_pTrans, m_iNumPaletteEntries, NULL);    
    }
}
