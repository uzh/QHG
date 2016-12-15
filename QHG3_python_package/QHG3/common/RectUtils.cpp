#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "RectUtils.h"

#ifndef KEY_HALO
  #define KEY_HALO "HALO"
#endif
#ifndef KEY_RECT
#define KEY_RECT "RECT"
#endif

//-----------------------------------------------------------------------------
// clearRect
//
void clearRect(rect &r1) {
    r1.top=0;
    r1.bottom=0;
    r1.left=0;
    r1.right=0;
}

//-----------------------------------------------------------------------------
// equalRects
//
bool equalRects(const rect &r1, const rect &r2) {
    return ((r1.left == r2.left) &&
            (r1.right == r2.right) &&
            (r1.bottom == r2.bottom) && 
            (r1.top == r2.top));
}

//-----------------------------------------------------------------------------
// mergeRects
//
void mergeRects(rect &r1, const rect &r2) {
    if (r2.left < r1.left) {
        r1.left = r2.left;
    }
    if (r2.bottom < r1.bottom) {
        r1.bottom = r2.bottom;
    }
    if (r2.right > r1.right) {
        r1.right = r2.right;
    }
    if (r2.top > r1.top) {
        r1.top = r2.top;
    }
}

//-----------------------------------------------------------------------------
// shiftRect 
//  shift the rectangle by x, y
//
void shiftRect(rect &r, int iX, int iY) {
    r.left    += iX;
    r.right   += iX;
    r.bottom  += iY;
    r.top     += iY;
}

//-----------------------------------------------------------------------------
// growRectSymm
//  expand the rectangle symmetrically in all directions (->Halo)
//
void growRectSymm(rect &r, int iDW, int iDH) {
    r.left    -= iDW;
    r.right   += iDW;
    r.bottom  -= iDH;
    r.top     += iDH; 
}


//-----------------------------------------------------------------------------
// contains
//  does the rectangle contain the point?
//
bool contains(const rect &r, const point &p) {
    return (p.first >= r.left) && (p.first <= r.right) &&
        (p.second >= r.bottom) && (p.second <= r.top);
}

//-----------------------------------------------------------------------------
// calcArea
//
int calcArea(const rect &r) {
    return (r.right - r.left + 1)*(r.top-r.bottom + 1);
}

//-----------------------------------------------------------------------------
// shareBorders
//  do the two rects share borders?
//
bool shareBorders(rect &r1, rect &r2) {
return  (((r1.bottom == r2.bottom) && (r1.top == r2.top) &&
          ((r1.left == (r2.right+1)) || ((r1.right+1) == r2.left))) ||
         ((r1.left == r2.left) && (r1.right == r2.right) &&
          ((r1.bottom == (r2.top+1)) || ((r1.top+1) == r2.bottom))));
}




//-----------------------------------------------------------------------------
// contains
//  is the rectangle contained in the rectlist?
//
bool contains(const rectlist  &vR, rect &r) {
    bool bSearching = true;
    unsigned int iN = vR.size();
    for (unsigned int i = 0; bSearching && (i < iN); i++) {
        rect r0 = vR[i];
        if ((r0.left = r.left) && (r0.right == r.right) && 
            (r0.bottom == r.bottom) && (r0.top == r.top)) {
            bSearching = false;
        }
    }
    return !bSearching;
}

//-----------------------------------------------------------------------------
// halointersect
//   calculate the intersection of r1 with r2 plus halo as r3,
//   return true if intersection not empty
//   if the rectangles do not overlap, the result is the overlap of r2's halo
//   with r1
//
bool halointersect(rect &r1, rect &r2, int iHalo, rect &r3) {
    bool bIntersect = false;
    if ((r2.left-iHalo <= r1.right) && (r2.bottom-iHalo <= r1.top) &&
        (r2.right+iHalo >= r1.left) && (r2.top+iHalo >= r1.bottom)) {
        bIntersect = true;
        r3.left   = MAX(r1.left,   r2.left-iHalo);
        r3.right  = MIN(r1.right,  r2.right+iHalo);
        r3.bottom = MAX(r1.bottom, r2.bottom-iHalo);
        r3.top    = MIN(r1.top,    r2.top+iHalo);
    }
    return bIntersect;
}

//-----------------------------------------------------------------------------
// toString
//   write a string representation of the rectangle into s
//
void toString(const rect &r, char *s) {
    sprintf(s, "(%d,%d,%d,%d)",r.first.first, 
            r.first.second, 
            r.second.first, 
            r.second.second); 
}

//-----------------------------------------------------------------------------
// parseRect
//   parse the string for a rect
//
int parseRect(char *pLine, rect &r) {
    int iResult = -1;
    char *pCtx;
    char *p = strtok_r(pLine, " \t,;(){}", &pCtx);
    int i = 0;
    int a[4];
    char *pEnd;
    while ((p != NULL) && (i < 4)) {
        int c = strtol(p, &pEnd, 10);
        if (*pEnd == '\0') {
            a[i++] = c;
            p = strtok_r(NULL, " \t,;(){}", &pCtx);
        } else {
            iResult = -1;
            p = NULL;
        }
    }
    if (i == 4) {
        r.left   = a[0];
        r.right  = a[2];
        r.bottom = a[1];
        r.top    = a[3];
        iResult = 0;
    }
    return iResult;       
}

//-----------------------------------------------------------------------------
// write
//  writes a tlf file of the rects
//
int writeRects(const char *pFile, const rectlist  &vR, int iHalo) {
    int iResult=-1;

    FILE *fOut = fopen(pFile, "wt");
    if (fOut != NULL) {
        fprintf(fOut, "%s %d\n", KEY_HALO, iHalo);
        for (unsigned int i = 0; i < vR.size(); i++) {
            char s[80];
            *s = '\0';
            rect r = vR[i];
            
            // write it
            toString(r, s);
            fprintf(fOut, "%s %s\n", KEY_RECT, s);
        }
        fclose(fOut);
    } else {
        printf("Couldn't open [%s]\n", pFile);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// addToBuffer
//   serialize the rectangle
//
unsigned char *addRectToBuffer(const rect &r, unsigned char *pBuf) {
    unsigned char *p = pBuf;
    memcpy(p, &(r.left),   sizeof(size)); p+= sizeof(size);
    memcpy(p, &(r.bottom), sizeof(size)); p+= sizeof(size);
    memcpy(p, &(r.right),  sizeof(size)); p+= sizeof(size);
    memcpy(p, &(r.top),    sizeof(size)); p+= sizeof(size);
    return p;
}

//-----------------------------------------------------------------------------
// getFromBuffer
//   deserialize the rectangle
//
unsigned char *getRectFromBuffer(rect &r, unsigned char *pBuf) {
    unsigned char *p = pBuf;
    memcpy(&(r.left),   p, sizeof(size)); p+= sizeof(size);
    memcpy(&(r.bottom), p, sizeof(size)); p+= sizeof(size);
    memcpy(&(r.right),  p, sizeof(size)); p+= sizeof(size);
    memcpy(&(r.top),    p, sizeof(size)); p+= sizeof(size);

    return p;
}
