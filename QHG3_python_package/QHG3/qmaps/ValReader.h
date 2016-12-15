#ifndef __VALREADER_H__
#define __VALREADER_H__

class ValReader {
public:
    virtual ~ValReader() {};
    virtual double   getDValue(double dLon, double dLat)=0;
    virtual double   getDValue(unsigned int iX, unsigned int iY)=0;

    virtual bool extractData()=0;
 
    virtual bool isOK()=0;

    virtual double getNextGridPointLon(double fM)=0;
    
    virtual double getNextGridPointLat(double fM)=0;

    virtual double getDLon()=0;
    virtual double getDLat()=0;
    virtual double getLonMin()=0;
    virtual double getLonMax()=0;
    virtual double getLatMin()=0;
    virtual double getLatMax()=0;
    
    virtual unsigned int getNLon() const =0;
    virtual unsigned int getNLat() const =0;
    virtual unsigned int getNRLon() const =0;
    virtual unsigned int getNRLat() const =0;

    virtual double X2Lon(double iX) const =0;
    virtual double Y2Lat(double iY) const =0;
    virtual double Lon2X(double fLon) const =0;
    virtual double Lat2Y(double fLat) const =0;
    virtual char *getFileName() const=0;
    virtual void   scanValues(bool bNormal=true) = 0;
    virtual double getMin() const = 0;
    virtual double getMax() const = 0;

    
    virtual char  *getVName() = 0;
    virtual char  *getYName() = 0;
    virtual char  *getXName() = 0;
    
    virtual int    getDataOffset()=0;
    virtual bool   sameFormat(ValReader *pVR, bool bStrict=false) = 0;
};

#endif
