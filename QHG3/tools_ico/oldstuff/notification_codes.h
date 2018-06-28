#ifndef __NOTIFICATION_CODES_H__
#define __NOTIFICATION_CODES_H__

#include "types.h"
#include "icoutil.h"

class Surface;
class IQSurface_OGL;

static const int  NOTIFY_MESSAGE        = 1;
static const int  NOTIFY_WARNING        = 2;
static const int  NOTIFY_ERROR          = 3;

static const int  NOTIFY_LU_CHANGE      = 100;
static const int  NOTIFY_GP_SET         = 101;
static const int  NOTIFY_DATA_LOADED    = 102;
static const int  NOTIFY_ALL_SET        = 103;


static const int  NOTIFY_MODEL_REPAINT  = 200;
static const int  NOTIFY_MODEL_LOADED   = 201;
static const int  NOTIFY_MODEL_FAILED   = 202;
static const int  NOTIFY_ICO_LOADED     = 203;
static const int  NOTIFY_ICO_FAILED     = 204;

static const int  NOTIFY_NEW_GRID       = 1001;
static const int  NOTIFY_NEW_DATA       = 1002;
static const int  NOTIFY_CLEAR_DATA     = 1003;
static const int  NOTIFY_LOAD_GRID      = 1004;
static const int  NOTIFY_LOAD_DATA      = 1005;
static const int  NOTIFY_DISPLAY_MODE   = 1006;
static const int  NOTIFY_INVALIDATE     = 1007;
static const int  NOTIFY_SET_PRESEL     = 1008;
static const int  NOTIFY_NEW_GRIDDATA   = 1009;
static const int  NOTIFY_ADD_DATA       = 1010;

static const int  NOTIFY_SET_USE_ALT    = 1020;
static const int  NOTIFY_SET_ALT_FACTOR = 1021;
static const int  NOTIFY_SET_USE_LIGHT  = 1022;
static const int  NOTIFY_USE_ALT_SET    = 1023;
static const int  NOTIFY_USE_LIGHT_SET  = 1024;

static const int  NOTIFY_SET_SELECTED   = 1030;
static const int  NOTIFY_CLEAR_MARKERS  = 1031;


typedef struct tAltData {
    bool bUseAlt;
    float fAltFactor;
} AltData;

typedef struct SurfInfo {
    Surface       *_pSurf;
    IQSurface_OGL *_pOGL;
    bool           _bFlat;
    SurfInfo(Surface *pSurf, IQSurface_OGL *pOGL, bool bFlat) : _pSurf(pSurf), _pOGL(pOGL), _bFlat(bFlat){};
    
} tSurfInfo;

typedef struct tSelectData {
    int x;
    int y;
    gridtype lNodeID;
    double dLon;
    double dLat;
    double dVal;
} SelectData;

#endif
