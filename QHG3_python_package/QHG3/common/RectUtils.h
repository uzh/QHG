/******************************************************************************
| RectUtils contains some useful functions for rectangles
|
\*****************************************************************************/
#ifndef __RECTUTILS_H__
#define __RECTUTILS_H__

#include <vector>
#include <map>

#include "types.h"

#define top    second.second
#define bottom first.second
#define left   first.first
#define right  second.first

#define MAX(a,b) (((a) >= (b))?(a):(b))
#define MIN(a,b) (((a) <= (b))?(a):(b))


typedef std::vector<int> vecint;
typedef std::map<int, vecint> mapvecint;

typedef std::pair<size,size> point;
typedef std::pair<point,point> rect;

typedef std::vector<rect> rectlist;  
typedef std::vector<point> pointlist;  

typedef std::pair<point, int> dataitem;
typedef std::vector<dataitem> dataitems;

void clearRect(rect &r1);
bool equalRects(const rect &r1, const rect &r2);
void mergeRects(rect &r1, const rect &r2);
void shiftRect(rect &r, int iX, int iY);
void growRectSymm(rect &r, int iDW, int iDH);
bool contains(const rect &r, const point &p);
int  calcArea(const rect &r);
bool shareBorders(rect &r1, rect &r2);
void toString(const rect &r, char *s);

bool contains(const rectlist  &vR, rect &r);

bool halointersect(rect &r1, rect &r2, int iHalo, rect &r3);
int  parseRect(char *pLine, rect &r);

int writeRects(const char *pFile, const rectlist  &vR, int iHalo);

unsigned char *addRectToBuffer(const rect &r, unsigned char *pBuf);
unsigned char *getRectFromBuffer(rect &r, unsigned char *pBuf);

#endif
