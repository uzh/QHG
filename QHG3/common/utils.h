/*============================================================================
| utils
| 
|  Various constantsMethods to handle and manipulate genomes consisting of 2 nucleotides
|  represented as 1-bit items packed in arrays of unsiged longs:
|  - creation and conversion to/from string
|  - mutation amd recombination
|  
|  Author: Jody Weissmann
\===========================================================================*/ 
#ifndef __UTILS_H__
#define __UTILS_H__
 

#include <math.h>
#include <stdio.h>
#include <endian.h>
#include <bits/endian.h>

#include "types.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef NUL
#define NUL '\0'
#endif


#define DEG2RAD(x) ((x)*M_PI/180.0)
#define RAD2DEG(x) ((x)*180.0/M_PI)

#define STDERR stderr
//#define STDERR stdout
#define STDOUT stdout

/// string lengths
// real path_max is 4096?
const int MAX_PATH = 2048;
const int MAX_LINE = 1024;
const int SHORT_INPUT = 64;
const int LONG_INPUT = 256;

/// some constants
const double S    = sqrt(3.0);
const double SINV = 1/S;

/// time periods
const float DAY  = 1.0f;
const float WEEK = 7.0f;
const float YEAR = 365.256363f;

const float MONTH = YEAR/12;
const float QUART = YEAR/4;

const float SECS_PER_DAY = 86400;

const double RADIUS_EARTH     = 6371300.0;
const double RADIUS_EARTH_KM  = RADIUS_EARTH/1000;
const double RADIUS_EARTH2 = RADIUS_EARTH * RADIUS_EARTH;

const int REP_ASEXUAL    =  0;
const int REP_SEXUAL     =  1;
const char STR_ASEXUAL[] = "asexual";
const char STR_SEXUAL[]  = "sexual";



const unsigned int ZONE_NONE = 0;
const unsigned int ZONE_CORE = 1;
const unsigned int ZONE_EDGE = 2;
const unsigned int ZONE_HALO = 3;

/*
// for these values, include bits/endian.h and check __BYTE_ORDER (__LITTLE_ENDIAN, __BIG_ENDIAN in  endian.h)
const long  NaNL = 0x7fbfffff;
const float fNaN = float(*((float *)(&NaNL)));

const int aNaN[2] = {0x7fffffff, 0xffffffff};
const double dNaN = double(*((double *)(aNaN)));
*/

///
/// definition of infinities and NaN
///
typedef union { unsigned char __c[4]; float __f; } __jhuge_valf_t;
typedef union { unsigned char __c[8]; double __d; } __jhuge_val_t;
#if __BYTE_ORDER == __BIG_ENDIAN
const __jhuge_valf_t __huge_valfp = {  0x7f, 0x80, 0, 0};
const __jhuge_valf_t __huge_valfn = {  0xff, 0x80, 0, 0};
const __jhuge_valf_t __nan_valf   = {  0x7f, 0xbf, 0xff, 0xff};

const __jhuge_val_t  __huge_valp  = {  0x7f, 0xf0, 0, 0, 0, 0, 0, 0 };
const __jhuge_val_t  __huge_valn  = {  0xff, 0xf0, 0, 0, 0, 0, 0, 0 };
const __jhuge_val_t  __nan_val    = {  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#elif __BYTE_ORDER == __LITTLE_ENDIAN
const __jhuge_valf_t __huge_valfp = {{  0, 0, 0x80, 0x7f}};
const __jhuge_valf_t __huge_valfn = {{  0, 0, 0x80, 0xff}};
const __jhuge_valf_t __nan_valf   = {{  0xff, 0xff, 0xbf, 0x7f}};

const __jhuge_val_t  __huge_valp  = {{  0, 0, 0, 0, 0, 0, 0xf0, 0x7f }};
const __jhuge_val_t  __huge_valn  = {{  0, 0, 0, 0, 0, 0, 0xf0, 0xff }};
const __jhuge_val_t  __nan_val    = {{  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f }};
#else
// no endian info - assume big
const __jhuge_valf_t __huge_valfp = {{  0x7f, 0x80, 0, 0}};
const __jhuge_valf_t __huge_valfn = {{  0xff, 0x80, 0, 0}};
const __jhuge_valf_t __nan_valf   = {{  0x7f, 0xbf, 0xff, 0xff}};

const __jhuge_val_t  __huge_valp  = {{  0x7f, 0xf0, 0, 0, 0, 0, 0, 0 }};
const __jhuge_val_t  __huge_valn  = {{  0xff, 0xf0, 0, 0, 0, 0, 0, 0 }};
const __jhuge_val_t  __nan_val    = {{  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }};
#endif

#ifdef INFINITY
const float fPosInf = (float) INFINITY;
const float fNegInf = -(float) INFINITY;
#else
const float fPosInf = __huge_valfp;
const float fNegInf = __huge_valfn;
#endif

#ifdef HUGE_VAL
const double dPosInf = (double) HUGE_VAL;
const double dNegInf = -(double) HUGE_VAL;
#else
const double dPosInf = __huge_valp.__d;
const double dNegInf = __huge_valn.__d;
#endif

extern const float fNaN;
extern const double dNaN;
//const float  fNaN = __nan_valf.__f;
//const double dNaN = __nan_val.__d;


#ifdef JDEBUG
#define dprintf(...) {printf("%s[%d]::%s(): ", __FILE__, __LINE__, __func__); printf(__VA_ARGS__); fflush(stdout);}
#else
#define dprintf(...)
#endif

template<class T> class Point {
public:
    Point(T dX, T dY) : m_dX(dX), m_dY(dY) {};
    T m_dX;
    T m_dY;

    void Hex2Cart() {
        T dX = (T) (S*(m_dX + m_dY/2));
        T dY = (T) (3*m_dY/2);
        m_dX = dX;
        m_dY = dY;
    };

    void Cart2Hex() {
        T dX = (T) (m_dX*SINV - m_dY/3);
        T dY = (T) (2*m_dY/3);
        m_dX = dX;
        m_dY = dY;
    }


};


/// verbosity values
const unsigned int VERB_ALL  = 16;
const unsigned int VERB_INFO =  8;
const unsigned int VERB_NONE = 0;

/*
/// auxiliary functions for managing 2D grids of doubles
double **createGrid(int iW, int iH, bool bInit=false);
double **createGrid(int iW, int iH, double dBGVal);
void     destroyGrid(double **ppGrid, int iW, int iH);
void     clearGrid(double **ppGrid, int iW, int iH);
void     nanGrid(double **ppGrid, int iW, int iH);
void     setGrid(double **ppGrid, int iW, int iH, double dVal);
*/

char *niceNum(char *pNum, double dNum);

/// arithmetic auxiliaries
#define jsignum(x) (((x)>0)?1:(((x)<0)?-1:0))
#define jmax(x,y) (((x)>(y))?(x):(y))
#define jmin(x,y) (((x)<(y))?(x):(y))
#endif
