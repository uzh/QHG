#include <stdio.h>
#include <stdlib.h>

int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 1) {
        iResult = 0;
        char *pEnd;
        double d1 = strtod(apArgV[1], &pEnd);
        if (*pEnd == '\0') {
            int i = 2;
            while ((iResult == 0) && (i < iArgC)) {
                double d = strtod(apArgV[i], &pEnd);
                if (*pEnd == '\0') {
                    d1 /= d;   
                } else {
                    iResult = -1;
                    printf("nan\n");
                }
                i++;
            }
            if (iResult == 0) {
                printf("%f\n", d1);
            } 
        } else {
            iResult = -1;
            printf("nan\n");
        }
    } else {
        iResult = -1;
        printf("nan\n");
    }
    return iResult;
}
 
int main1(int iArgC, char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 1) {
        char *pEnd;
        double d1 = strtod(apArgV[1], &pEnd);
        if (*pEnd == '\0') {
            if (iArgC > 2) {
                double d2 = strtod(apArgV[2], &pEnd);
                if (*pEnd == '\0') {
                    if (iArgC > 3) {
                        double d3 = strtod(apArgV[3], &pEnd);
                        if (*pEnd == '\0') {
                            printf("%f\n", (d1/d2)/d3);
                        } else { 
                            printf("nan\n");
                        }
                    } else {
                        printf("%f\n", d1/d2);
                    }
                } else {
                    printf("nan\n");
                }
            } else {
                printf("%f\n", d1);
            }
        } else {
	    printf ("nan\n");
        }
    } else {
        printf("nan\n");
    }

    return iResult;
}

