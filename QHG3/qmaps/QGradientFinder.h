#ifndef __QGRADIENTFINDER_H__
#define __QGRADIENTFINDER_H__

#include <vector>

#include "Vec3D.h"

class ValReader;

class QGradientFinder {
public:
    static QGradientFinder *createQGradientFinder(char *pQMAPFile);
    ~QGradientFinder();
    
    int calcFlow(unsigned int iX, unsigned int iY, double dMin, std::vector<Vec3D> &vPath);

    ValReader *getVR() { return m_pVR;};
    void setVerbosity(bool bNewVerbosity) { m_bVerbose = bNewVerbosity;};
protected: 
    QGradientFinder();
    int loadQMap(char *pQMAPFile);
    int findGradient(unsigned int iX, unsigned int iY, std::vector<int> &vDirs, double &dMag);
    bool close(unsigned int iX1, unsigned int iY1,unsigned int iX2, unsigned int iY2);
    int getPossibleDirsN(unsigned int iX, unsigned int iY, int aaiNeighN[][2], unsigned int iNumNeigh, std::vector<Vec3D> &vPath, std::vector<int> &vDirs);
    //    void collectPoint(unsigned int &iX0, unsigned int &iY0, std::vector<Vec3D> &vPath, std::vector<int> &vDirs);
    void collectPoint(unsigned int &iX0, unsigned int &iY0, Vec3D &vA, std::vector<int> &vDirs);
    ValReader     *m_pVR;
    unsigned int   m_iW;
    unsigned int   m_iH;
    bool           m_bVerbose;
};
#endif

