#ifndef __ARRAYSHARE_H__
#define __ARRAYSHARE_H__

#include <string>
#include <map>
#include <vector>


typedef struct arraystruct {
    int m_iSize;
    void *m_pdArr;
    // constructor
    arraystruct(int iSize, void *pdArr) : m_iSize(iSize), m_pdArr(pdArr) {};
} arraystruct;


typedef std::map<std::string, arraystruct *> arraymap;

typedef std::vector<std::string> stringlist;

class ArrayShare {
public:
    ArrayShare();
    virtual ~ArrayShare();

    int shareArray(const char *pName, int iSize, void *pdArr);
    int getSize(const char *pName);
    void *getArray(const char *pName);
    arraystruct *getArrayStruct(const char *pName);
    int removeArray(const char *pName);
    const stringlist &getNamesLike(const char *pNamePattern);
    
    void display();    

protected:
    arraymap   m_mArrays;
    stringlist m_vNameMatches;

public:
    static ArrayShare *getInstance();
    static void freeInstance();
protected:
    static ArrayShare *s_pAS;

};

#endif
