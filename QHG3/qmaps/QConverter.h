#ifndef __QCONVERTER_H__
#define __QCONVERTER_H__

class QMapHeader;


class QConverter {
public:
    static int convert(const char *pInput, const char *pOutput, int iTypeOut);
protected:
    template <class T, class U>
    static int convertAndWrite(QMapHeader *pQMH, T **aatData, T tDef, U uDef, const char *pOutput);
    
    template<class U>
    static int convertToType(const char *pInput, const char *pOutput, int iTypeOut, U uDef);
};

#endif

