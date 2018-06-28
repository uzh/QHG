#include "BinSearch.h"

#include <stdio.h>

int bs(double dSearch, double *a, int iN) {
    int jL = -1;
    int jU = iN;
    //    int asc = (a[iN-1] > a[0]);
    while (jU-jL > 1) {
        int jM = (jU+jL)>>1;
        if (dSearch > a[jM]) {
            jL = jM;
        } else if (dSearch < a[jM]) {
            jU = jM;
        } else {
            return jM;
        }
    }
    return jL;
}

int binsearch(double dSearch, double *a, int iLeft, int iRight) {
    int iRes = 0;
    printf("%d <-> %d\n", iLeft, iRight);
    if (iLeft < iRight) {
        int iMid = (iLeft+iRight)/2;
        if (a[iMid] < dSearch) {
            iRes = binsearch(dSearch, a, iMid, iRight);
        } else if (a[iMid] > dSearch) {
            iRes = binsearch(dSearch, a, iLeft, iMid);
        } else {
            iRes = iMid;
        }
    } else {
        iRes = iLeft;
    }
        return iRes;
}

int binsearch(float dSearch, float *a, int iLeft, int iRight) {
    int iRes = 0;
    if (iLeft < iRight) {
        int iMid = (iLeft+iRight)/2;
        if (a[iMid] < dSearch) {
            iRes = binsearch(dSearch, a, iMid+1, iRight);
        } else if (a[iMid] > dSearch) {
            iRes = binsearch(dSearch, a, iLeft, iMid-1);
        } else {
            iRes = iMid;
        }
    } else {
        iRes = iLeft;
    }
        return iRes;
}

