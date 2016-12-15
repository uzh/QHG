#include <stdio.h>

#include "ArrayShare.h"

//-- a class of pairs
class C {
public:
    C(double dRe, double dIm)
        : m_dRe(dRe),
          m_dIm(dIm) {};

    double m_dRe;
    double m_dIm;
};

//-- class A --------------------------------------------
// has an array of doubles and an array of C
 
class A {
public:
    A(/*ArrayShare *pAE,*/ int iSize);
    ~A();
    int getArrays();
    void modify(int i);
    void show();
    ArrayShare *m_pAE;
    int m_iSize1;
    double *m_pdArr;
    int m_iSize2;
    C **m_pComplex;
};

// constructor
A::A(/*ArrayShare *pAE, */int iSize)
    : m_pAE(ArrayShare::getInstance()),
      m_iSize1(iSize),
      m_pdArr(NULL),
      m_iSize2(0),
      m_pComplex(NULL) {

    m_pdArr = new double[m_iSize1];
    for (int i = 0; i < m_iSize1; i++) {
        m_pdArr[i] = i/2.0;
    }

    int iResult = m_pAE->shareArray("gummi", m_iSize1, (void *)m_pdArr);
    if (iResult != 0) {
        printf("Failed to share array 'gummi'\n");
    }
}

// destructor
A::~A() {
    if (m_pdArr != NULL) {
        delete[] m_pdArr;
    }
// don't delete m_pComplex: owned by B

}

int A::getArrays() {
    int iResult = -1;
    m_iSize2 = m_pAE->getSize("immi");
    if (m_iSize2 > 0) {
        m_pComplex = (C**) m_pAE->getArray("immi");
        if (m_pComplex != NULL) {
            iResult = 0;
        }
    }
    return iResult;
}


// modify
void A::modify(int k) {
    for (int i = 0; i < m_iSize1; i++) {
        m_pdArr[i] += k;
    }
    for (int i = 0; i < m_iSize2;  i++) {
        m_pComplex[i]->m_dRe /= k+1;
        m_pComplex[i]->m_dIm /= k+2;
    }
}

// show
void A::show() {
    printf("A has [%s] %d:", "gummi", m_iSize1); 
    for (int i = 0; i < m_iSize1; i++) {
        printf(" %.2f", m_pdArr[i]);
    }
    printf("\n");
        
        
    printf("A has [%s] %d:", "immi", m_iSize2); 
    for (int i = 0; i < m_iSize2; i++) {
        printf(" (%.2f,%.2f)", m_pComplex[i]->m_dRe, m_pComplex[i]->m_dIm);
    }
    printf("\n");
}


//-- class B --------------------------------------------
// accesses and modifies A's arrays

class B {
public:
    B(/*ArrayShare *pAE, */int iSize);
    ~B();
    int getArrays();
    void show();
    void modify(int k);
    ArrayShare *m_pAE;
    
    int m_iSize1;
    double *m_pDoubles;
    int m_iSize2;
    C **m_pCs;
};

// constructor
B::B(/*ArrayShare *pAE, */int iSize) 
    : m_pAE(ArrayShare::getInstance()),
      m_iSize1(iSize),
      m_pDoubles(NULL),
      m_iSize2(0),
      m_pCs(NULL) {

    m_pCs = new C*[m_iSize1];
    for (int i = 0; i < m_iSize1; i++) {
        m_pCs[i] = new C(2*i, 3*i);
    }
    int iResult = m_pAE->shareArray("immi", m_iSize1, (void *)m_pCs);
    if (iResult != 0) {
        printf("Failed to share array 'immi'\n");
    }
}

// destructor
B::~B(){
    if (m_pCs != NULL) {
        for (int i = 0; i < m_iSize1; i++) {
            delete m_pCs[i];
        }
        delete[] m_pCs;
    }    
    // don't delete m_pDoubles: owned by A
};

// getArrays
int B::getArrays() {
    int iResult = -1;
    m_iSize2 = m_pAE->getSize("gummi");
    if (m_iSize2 > 0) {
        m_pDoubles = (double*)m_pAE->getArray("gummi");
        if (m_pDoubles != NULL) {
            iResult = 0;
        }
    }
    return iResult;
}


// show
void B::show() {
    printf("B has [%s] %d:", "gummi", m_iSize2); 
    for (int i = 0; i < m_iSize2; i++) {
        printf(" %.2f", m_pDoubles[i]);
    }
    printf("\n");

    printf("B has [%s] %d:", "immi", m_iSize1); 
    for (int i = 0; i < m_iSize1; i++) {
        printf(" (%.2f,%.2f)", m_pCs[i]->m_dRe, m_pCs[i]->m_dIm);
    }
    printf("\n");
  
}

//modify
void B::modify(int k) {
    for (int i = 0; i < m_iSize2; i++) {
        m_pDoubles[i] *= k;
    } 
    for (int i = 0; i < m_iSize1;  i++) {
        m_pCs[i]->m_dRe *= k+1;
        m_pCs[i]->m_dIm *= k+2;
    }
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    
    //    ArrayShare *pAS = new ArrayShare();

    A *pA = new A(/*pAE,*/ 7);
    B *pB = new B(/*pAE, */9);

    printf("--- init ---\n");
    pA->show();
    printf("---\n");
    pB->show();
    printf("\n");

    pA->getArrays();
    printf("--- after A gets arrays ---\n");
    pA->show();
    printf("---\n");
    pB->show();
    printf("\n");

    pB->getArrays();
    printf("--- after B gets arrays ---\n");
    pA->show();
    printf("---\n");
    pB->show();
    printf("\n");

    pA->modify(5);
    printf("--- after modify from A ---\n");
    pA->show();
    printf("---\n");
    pB->show();
    printf("\n");

    pB->modify(2);
    printf("--- after modify from B ---\n");
    pA->show();
    printf("---\n");
    pB->show();
    printf("\n");


    delete pB;
    delete pA;
    //    delete pAS;
    ArrayShare::freeInstance();
    return iResult;
}
