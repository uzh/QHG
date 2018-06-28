#ifndef __NPPCAPACITY_H__
#define __NPPCAPACITY_H__

#include "Observer.h"
#include "Action.h"

class WELL512;
class NPPCalc;

#define ATTR_NPPCAPACITY_NAME "NPPCapacity"
#define ATTR_NPPCAPACITY_MAXCAPACITY_NAME     "NPPCap_max_capacity"
#define ATTR_NPPCAPACITY_VEGSELECTION_NAME    "NPPCap_veg_selection"
#define ATTR_NPPCAPACITY_WATERFACTOR_NAME     "NPPCap_water_factor"
#define ATTR_NPPCAPACITY_COASTALFACTOR_NAME   "NPPCap_coastal_factor"
#define ATTR_NPPCAPACITY_COASTAL_MIN_LAT_NAME "NPPCap_coastal_min_latitude"
#define ATTR_NPPCAPACITY_COASTAL_MAX_LAT_NAME "NPPCap_coastal_max_latitude"
#define ATTR_NPPCAPACITY_NPPMIN_NAME          "NPPCap_NPP_min"
#define ATTR_NPPCAPACITY_NPPMAX_NAME          "NPPCap_NPP_max"
#define ATTR_NPPCAPACITY_KMIN_NAME            "NPPCap_K_min"

#define NPPCAPACITY_SEL_GRASS 1
#define NPPCAPACITY_SEL_BUSH  2
#define NPPCAPACITY_SEL_TREE  4

// it is not very nice to derive NPPCapacity from Action (it isn't really one),
// but we want to profit from the automatic parameter loading.

template<typename T>
class NPPCapacity : public Action<T>, Observer {
public:
    NPPCapacity(SPopulation<T> *pPop, SCellGrid *pCG, WELL512 **apWELL, double *adCapacities, int iStride);
    virtual ~NPPCapacity();

    virtual int preLoop();

   // get action parameters from QDF 
    virtual int extractParamsQDF(hid_t hSpeciesGroup);

    // write action parameters to QDF
    virtual int writeParamsQDF(hid_t hSpeciesGroup);

    // read action parameters from ascii POP file
    virtual int tryReadParamLine(char *pLine);

    void notify(Observable *pObs, int iEvent, const void *pData);
    void recalculate();

    void showAttributes();
protected:
    WELL512 **m_apWELL;
    double   *m_adCapacities;
    int       m_iStride;
    bool      m_bNeedUpdate;
    
    double   m_dMaxCapacity;
    double   m_adSelection[3];
    double   m_dWaterFactor;
    double   m_dCoastalFactor;
    double   m_dCoastalMinLatitude;
    double   m_dCoastalMaxLatitude;
    double   m_dNPPMin;
    double   m_dNPPMax;
    double   m_dKMin;

    NPPCalc *m_pNPPCalc;

};    
#endif
