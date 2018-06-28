#ifndef __ICOCONVERTER_H__
#define __ICOCONVERTER_H__

class Icosahedron;
class HeaderBase;

#define TO_NODE  true
#define TO_COORD false

class IcoConverter {
public:
    static IcoConverter*createInstance(IcoLoc *pIcoLoc, const char *pIcoName, bool bPreSel);
    ~IcoConverter();


    HeaderBase *getHeader(const char *pInputFile);

    int  nodifyFile(const char *pInput, const char *pOutput);
    int  coordifyFile(const char *pInput, const char *pOutput);
    
protected:
    IcoConverter(IcoLoc *pIcoLoc, const char *pIcoName, bool bPreSel);
    int  nodifyFileBin(const char *pInput, const char *pOutput);
    int  nodifyFileAsc(const char *pInput, const char *pOutput);
    int  coordifyFileBin(const char *pInput, const char *pOutput);
    int  coordifyFileAsc(const char *pInput, const char *pOutput);
    
    int init();

    IcoLoc *m_pIcoLoc;
    const char *m_pIcoName;
    bool m_bPreSel;
    HeaderBase *m_pHB;
};


#endif

