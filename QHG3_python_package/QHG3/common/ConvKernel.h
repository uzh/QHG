#ifndef __CONVKERNEL_H__
#define __CONVKERNEL_H__

#include "types.h"

const int KERNEL_URECT = 0;
const int KERNEL_UCIRC = 1;
const int KERNEL_PYRA  = 2;
const int KERNEL_CONIC = 3;
const int KERNEL_GAUSS = 4;

static const char *s_asNames[] = {
    "Uniform Rectangular",
    "Uniform Circular",
    "Pyramidal",
    "Conical",
    "Gaussian",
};

class ConvKernel {

public:
    ConvKernel(size iW, size iH);
    ConvKernel(size iW, size iH, double **aadValues, bool bDeleteOnExit);
   ~ConvKernel();

    void clear();
    void set(size iW, size iH, double **aadValues, bool bDeleteOnExit);
    void setElement(coord iX, coord iY, double dValue);

    static ConvKernel *uniformRect(size iW, size iH);
    static ConvKernel *uniformCirc(size iW, size iH);
    static ConvKernel *pyramidal(size iW, size iH, double dAlt);
    static ConvKernel *conical(size iW, size iH, double dAlt);
    static ConvKernel *gaussian(size iW, size iH, double dSigma);

    static ConvKernel *createKernel(int iType,  size iW, size iH, double dParam);
    static const char *getName(int iType) { return s_asNames[iType];};
    static int   getNumKernelTypes() { return sizeof(s_asNames)/sizeof(char*);};

    void normalize();
    double sum();
    void scale(double dS);

    void deleteArray();
    void display();

    int      m_iW;
    int      m_iH;
    double **m_aadValues;
    bool     m_bDeleteData;
    
};

#endif

