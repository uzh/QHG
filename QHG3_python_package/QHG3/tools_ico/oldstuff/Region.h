#ifndef __REGION_H__
#define __REGION_H__

class IcoNode;

class Region {
public:
    Region();
    Region(int iID);
    virtual ~Region();
    virtual bool contains(IcoNode *pBC)=0;
    virtual void display() = 0;
    virtual unsigned char *serialize()=0;
    virtual int deserialize(unsigned char *pBuffer)=0;
    virtual int dataSize() { return sizeof(int); };
    int m_iID;
    
};

#endif
