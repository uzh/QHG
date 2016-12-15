#ifndef __SURFACEHANDLER_H__
#define __SURFACEHANDLER_H__

class RegionSplitter;

class SurfaceHandler {
public:
    virtual int saveSurface(const char *pFileNameSurface, 
                            const char *pFileNameIGN, 
                            const char *pFileNameQDF, 
                            RegionSplitter *pRS)=0;
    virtual int loadSurface(const char *pFileNameSurface)=0;
    virtual void activateSurface()=0;
    virtual void displaySurface()=0;
};


#endif



