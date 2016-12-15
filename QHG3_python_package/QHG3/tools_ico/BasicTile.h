#ifndef __BASICTILE_H__
#define __BASICTILE_H__

class IcoNode;

class BasicTile {
public:
    BasicTile();
    BasicTile(int iID);
    virtual ~BasicTile();
    virtual bool contains(IcoNode *pBC)=0;
    virtual void display() = 0;
    virtual unsigned char *serialize()=0;
    virtual int deserialize(unsigned char *pBuffer)=0;
    virtual int dataSize() { return sizeof(int); };
    int m_iID;
    
};

#endif
