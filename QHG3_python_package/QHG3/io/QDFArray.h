#ifndef __QDFARRAY_H__
#define __QDFARRAY_H__

#include <vector>
#include <hdf5.h>

#define NUM_GROUPS 4

class QDFArray {
public:
    static QDFArray *create(const char *pQDFFile);
    ~QDFArray();

    int init(const char *pQDFFile);

    int openArray(const char *pPathToDataset);
    int openArray(const char *pGroup, const char *Dataset);
    int openArray(const char *pGroup1, const char *pGroup2, const char *Dataset);
    int openArray(std::vector<const char *> &vGroups, const char *pDataSet);

    uint getSize() { return m_iArraySize;};
    int getFirstSlab(int *pBuffer, int iSize, const char *pFieldName=NULL);
    int getFirstSlab(long *pBuffer, int iSize, const char *pFieldName=NULL);
    int getFirstSlab(double *pBuffer, int iSize, const char *pFieldName=NULL);

    template <typename T>
    int getNextSlab(T *pBuffer, int iSize);

    void  closeArray();
    float getTimeStep() {return m_fTimeStep;};
protected:
    QDFArray();


    template<typename T>
    int getFirstSlab(T *pBuffer, int iSize, hid_t hBaseType, const char *pFieldName);

    template <typename T>
    int readSlab(T *pBuffer);

    int setDataType(const char *pFieldName, hid_t hBaseType, int iSize);

    hid_t m_hFile;
    hid_t m_hRoot;
    hid_t m_hDataSet;
    hid_t m_hDataSpace;
    hid_t m_hMemSpace;
    hid_t m_hDataType;
    std::vector<hid_t> m_vhGroups;

    uint   m_iArraySize;
    
    hsize_t m_offset;
    hsize_t m_count;  
    
    hsize_t m_stride;
    hsize_t m_block;
    hsize_t m_memsize;
    int m_iBufSize;
    bool m_bDeleteDataType;
    float m_fTimeStep;
};

#endif
