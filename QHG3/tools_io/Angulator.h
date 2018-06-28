#ifndef __ANGULATOR_H__
#define __ANGULATOR_H__

#include <hdf5.h>

class Vec3D;
class SCellGrid;

class Angulator {
public:
    static Angulator *createInstance(const char *pFile);
    static Angulator *createInstance(hid_t hFile);
    virtual ~Angulator();

    int doAngles();
    int check(const char *pFile);
    static double calcAngle(Vec3D *pvNorth, Vec3D *pvVert, Vec3D *pvDir);
    static void getNorth(double dTheta, double dPhi, Vec3D *pvNorth);
    static void polar2Cart(double dTheta, double dPhi, Vec3D *pVec);

protected:    
    Angulator(hid_t hFile);
    int calcAngles(double *pdAngles);
    int calcDirs(double *pdDirs);
    int openGroups();
    void closeGroups();
    int prepareArrays(bool bAngles);
    void destroyArrays();
    hid_t createCellDataType();
    int readCellData();
    int saveAngles();
    int saveDirs();

    hid_t m_hFile;
    hid_t m_hGridGroup;
    hid_t m_hGeoGroup;

    int     m_iNumCells;
    double *m_pdLons;
    double *m_pdLats;
    double *m_pdAngles;
    double *m_pdAngles2;
    double *m_pdDirs;
    double *m_pdDirs2;
    SCell  *m_aCells;
};

#endif
