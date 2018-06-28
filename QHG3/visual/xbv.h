#ifndef __XBV_H__
#define __XBV_H__

#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/alignment.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/adjustment.h>

#include <gtkmm/scrolledwindow.h>

#include <vector>
#include <map>

#include "LookUpFactory.h"
#include "GeoInfo.h"
#include "xbvd.h"
#include "xbvod.h"
#include "xbvsd.h"

#include "Vec3D.h"

class ValReader;

class LookUp;
class LookUpFactory;


class LookUpVals {
public:
    int iType;
    int iNumParams;
    std::vector<double> vParamVals;
    LookUpVals() :iType(LOOKUP_UNDEF) {};
};



char *niceNum(char *pNum, double dNum);

class xbv : public Gtk::Window {
public:
    xbv();
    virtual ~xbv();
    
    int setImage(const char *pFileName);
    void createImage();

    int setReader(const char *pFileName, const char *pRange=NULL);
    int setValReader(ValReader *pVR, LookUp *pLookUp=NULL);
    int setRawReader(const char *pFileName, int iType, 
                     const char *pLonMin, const char *pLonMax, const char *pDLon, 
                     const char *pLatMin, const char *pLatMax, const char *pDLat);
    int setLookUp(LookUp *pLU);
    LookUp *setLookUp(int iType, double *pdParams);

    ValReader *getValReader() const {return m_pVR;};
    LookUp *getLookUp() const {return m_pLU;};
    LookUpVals getLookUpVals() const {return m_luvCur;};
    char *getDir() { return m_sDir; };


    void setOverlayData(int iShape, double dSize, double dValue);
    void getOverlayData(int &iShape, double &dSize, double &dValue);

    void getProjInfo(ProjType *ppt, ProjGrid *ppg){};
    void setProjInfo(ProjType *ppt, ProjGrid *ppg){};

    void getConvInfo(char *pFileName, int &iType, 
                     double &dLonMin, double &dLonMax, double &dDeltaLon,
                     double &dLatMin, double &dLatMax, double &dDeltaLat){};
    void setConvInfo(const char *pFileName, int iType, 
                     double dLonMin, double dLonMax, double dDeltaLon,
                     double dLatMin, double dLatMax, double dDeltaLat){};

    void doCommands(char *pCommands);
    bool createPNG(char *pName=NULL);
    bool createRAW(char *pName=NULL);
    int setLookUp(const char *pLookUpName, const char *pParams);

    int loadArcs(char *pArcs);
    int drawArcs();
    int loadVectors(char *pVectors);
    int drawVectors();
    int getRange(double &dMin, double &dMax);
protected:
    void drawVectors(Cairo::RefPtr<Cairo::Context> &cr, double dS);
    int  saveVectors(std::string sFileName, bool bWithBackground);
    bool createPNGFromQMAP(char *pName);

    void helpDialog();
    guint8 *createImageFromQMAPOVR();
    guint8 *createImageFromQMAP();
    guint8 *createImageFromPNG();

    //Signal handlers:
    void showZoom();
    void getCenter();
    void setCenter();
    bool showValue(unsigned int iX, unsigned int iY);
    void  setOverlayVal(unsigned int iX, unsigned int iY, double dValue);

    void createOverlay();
    void saveOverlay();
    void destroyOverlay();

    virtual bool doMotion(GdkEventMotion *e);
    virtual bool press_action(GdkEventKey *e);
    virtual bool doClick(GdkEventButton *e);
    virtual bool doRelease(GdkEventButton *e);
    virtual bool expose_action(GdkEventExpose *e);
    void setLU(int iLUType);
    void showInteractiveCommands();

    Gtk::Image m_imgMain; 

    //Child widgets:
    Gtk::Label m_lblCoords;
    Gtk::Label m_lblLon;
    Gtk::Label m_lblLat;
    Gtk::Label m_lblValName;
    Gtk::Label m_lblValue;
    Gtk::Label m_lblZoom;
    Gtk::HBox m_HBox0;

    Gtk::HBox m_HBox1;
    Gtk::HBox m_HBox2;
    Gtk::HBox m_HBox3;
    Gtk::VBox m_VBox;
    Gtk::Adjustment m_adjh;
    Gtk::Adjustment m_adjv;
    Gtk::ScrolledWindow m_scrW;
    Gtk::Alignment m_ali;


    ValReader  *m_pVR;
    LookUp    *m_pLU;
    LookUpVals m_luvCur;
   
    bool       m_bFlipped;

    char m_sPrevFile[1024];
    char m_sDir[1024];
    int m_iVRType;
    int m_iZoom;
    double m_dZoom;
    bool m_bMagnify;
    bool m_bDying;
 
    bool m_bGeoView;
    double m_CX;
    double m_CY;
    bool m_bCross;

    double  m_dCurSpacing;
    guint8 *m_pImgData;

    xbvd   m_xbvlDialog;
    bool   m_bOverlay;
    double m_dOAlpha;
    double m_dOValue;
    double m_dOSize;
    double **m_aadOverlay;
    unsigned int m_iOW;
    unsigned int m_iOH;
    xbvod m_xbvoDialog;

    ProjType m_ptLocal;
    ProjGrid m_pgLocalY;
    xbvsd m_xbvsDialog;

    std::vector<std::pair<Vec3D, Vec3D> > m_vVecs;
    
    std::vector<std::vector<std::pair<double, double> > > m_vArcs;
    bool m_bShowVectors;
    char m_sVecOut[1024];
    float m_fVecScale;
    bool m_bFreeze;
};



#endif

