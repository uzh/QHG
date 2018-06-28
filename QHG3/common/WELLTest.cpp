#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>

#include "WELL512.h"
//#include "LWELL512.h"


static unsigned int s_aInit[] = {
    0x2ef76080, 
    0x1bf121c5, 
    0xb222a768, 
    0x6c5d388b, 
    0xab99166e, 
    0x326c9f12, 
    0x3354197a, 
    0x7036b9a5, 
    0xb08c9e58, 
    0x3362d8d3, 
    0x037e5e95, 
    0x47a1ff2f, 
    0x740ebb34, 
    0xbf27ef0d, 
    0x70055204, 
    0xd24daa9a,
};

int main2(int iArgC, char *apArgV[]) {
    int iResult = -1;

    if (iArgC > 1) {
        int iN = atoi(apArgV[1]);
        int iLoops = 100;
        if (iArgC > 2) {
            iLoops = atoi(apArgV[2]);
        }
        if ((iN > 0) && (iLoops > 0)) {
            WELL512 **apW = new WELL512*[iN];
            //            LWELL512 **apLW = new LWELL512*[iN];

            unsigned int temp[STATE_SIZE];
            //            unsigned long ltemp[STATE_SIZE];
            for (int i = 0; i < iN; i++) {
                for (unsigned int j = 0; j < STATE_SIZE; j++) {
                    temp[j] = s_aInit[(i+j)%16];
                    //                    ltemp[j] = s_aInit[(i+j)%16];
                }
                apW[i] = new WELL512(temp);
                //                apLW[i] = new LWELL512(ltemp);
            }
            int aiBins[100];
            //            int aiLBins[100];
            for (int i = 0; i < 100; i++) {
                aiBins[i] = 0;
                //                aiLBins[i] = 0;
            }
            for (int i = 0; i < iLoops; i++) {
                /*
                for (int j = 0; j < iN; j++) {
                    printf("%08x ", apW[j]->wrand());
                }
                printf("\n");
                for (int j = 0; j < iN; j++) {
                    printf("%f ", apW[j]->wrandd());
                }
                printf("\n");
                */
                for (int j = 0; j < iN; j++) {
                    int ii =  (int)(apW[j]->wrandr(0, 100));
                    aiBins[ii]++;
                    //                    printf("%d ", ii);
                    //                    ii =  (int)(apLW[j]->wrandr(0, 100));
                    //                    aiLBins[ii]++;
                    //                    printf("%d ", ii);
                }
                //                printf("\n");
            }

            FILE *fOut = fopen("binwell.dat", "wt");
            double dSum=0;
            double dSum2=0;
            for (int i = 0; i < 100; i++) {
                fprintf(fOut, "%d  %d\n", i,aiBins[i]);
                dSum += aiBins[i];
                dSum2 += aiBins[i]*aiBins[i];
            }
            printf("WELL: avg %f, dev %f\n", dSum/100, dSum2/100-dSum*dSum/10000);
            fclose(fOut);
            /*
            double dlSum=0;
                        double dlSum2=0;
            fOut = fopen("binlwell.dat", "wt");
            for (int i = 0; i < 100; i++) {
                fprintf(fOut, "%d  %d\n", i,aiLBins[i]);
                dlSum += aiLBins[i];
                dlSum2 += aiLBins[i]*aiLBins[i];
            }
            fclose(fOut);
            printf("WELL: avg %f, dev %f\n", dlSum/100, dlSum2/100-dlSum*dlSum/10000);
            */

            for (int j = 0; j < iN; j++) {
                delete apW[j];
            }
            delete[] apW;
            iResult = 0;

        } else {
            printf("num-generators (%d) and loop (%d) must be positive\n", iN, iLoops);
        }
    } else {
        printf("%s <num-generators> <loop>\n",apArgV[0]);
    }
    return iResult;
}


int main3(int iArgC, char *apArgV[]) {
    WELL512 *pW = new WELL512(s_aInit);
    for (long i = 0; i < atol(apArgV[1]); i++) {
        uint x = pW->wrandr(0,20);
        if (x == 20) {
            printf("Hit %d\n",x);
        }
       
    }
    return 0;
}

int main4(int iArgC, char *apArgV[]) {
    if (iArgC > 2) {

        int iBufSize = atoi(apArgV[1]);
        

        int iN = omp_get_max_threads();
        WELL512 **apWELL = new WELL512*[iN];
        printf("creating %d WELLs\n", iN); 
#pragma omp parallel
        {
            int iT = omp_get_thread_num();
            apWELL[iT] = new WELL512(s_aInit);
        }
        
        printf("creating %d random numbers\n", iBufSize); 
        uint a[iBufSize];
#pragma omp parallel for 
        for (int i =0; i < iBufSize; i++) {
            int iT = omp_get_thread_num();
            a[i] = apWELL[iT]->wrand();

            time_t t = time(NULL)/1000;
            int c = 0;

            for (uint k = 0; k < t; k++) {
                c += a[i];
            }
        }
        
        printf("writing numbers\n"); 
        FILE *f = fopen(apArgV[2], "wb");
        fwrite(a, sizeof(uint), iBufSize, f);
        fclose(f);

        printf("deleting WELLs\n"); 
        
#pragma omp parallel
        {
            int iT = omp_get_thread_num();
            delete apWELL[iT];
        }
        
        delete[] apWELL;
    } else {
        printf("%s <arrsize> <outputfile>\n", apArgV[0]);
    } 
    return 0;
}


int main1(int iArgC, char *apArgV[]) {
    if (iArgC > 4) {
        
        WELL512 *pWELL = new WELL512(s_aInit);
        int iMin   = atoi(apArgV[1]);
        int iMax   = atoi(apArgV[2]);
        int iStep   = atoi(apArgV[3]);
        long iIters = atol(apArgV[4]);

        int iNumBins = iMax-iMin;
        long *bins = new long[iNumBins];
        memset(bins, 0,  iNumBins*sizeof(long));
        
        double t00  = omp_get_wtime();
        for (long i = 0; i < iIters; i++) {
            uint k = pWELL->wrandi(iMin, iMax, iStep);
            bins[k-iMin]++;
        }
        double t01  = omp_get_wtime();
        printf("wrandi used %f secs\n", t01-t00);
        for (int i = 0; i < iNumBins; i++) {
            printf("bin %d (%d): %ld\n", i, i+iMin, bins[i]);
        }

        memset(bins, 0,  iNumBins*sizeof(long));
        double t10  = omp_get_wtime();
        for (long i = 0; i < iIters; i++) {
            int r = iMax - iMin;
            r += (r%iStep);
            uint k = iMin + iStep*(int)(1.0*r*pWELL->wrandd()/iStep);
            bins[k-iMin]++;
        }
        double t11  = omp_get_wtime();
        printf("wrandd+transf used %f secs\n", t11-t10);
        for (int i = 0; i < iNumBins; i++) {
            printf("bin %d (%d): %ld\n", i, i+iMin, bins[i]);
        }

        memset(bins, 0,  iNumBins*sizeof(long));
        double t20  = omp_get_wtime();
        for (long i = 0; i < iIters; i++) {
            int r = iMax - iMin;
            r += (r%iStep);
            uint k = (uint) (pWELL->wrandr(iMin, (1.0+iMax-iMin)/iStep))*iStep;
            bins[k-iMin]++;
        }
        double t21  = omp_get_wtime();
        printf("wrandr+transf used %f secs\n", t21-t20);

        for (int i = 0; i < iNumBins; i++) {
            printf("bin %d (%d): %ld\n", i, i+iMin, bins[i]);
        }

        delete[] bins;
        delete pWELL;
    } else {
        printf("%s <min> <max> <step> <iters>\n", apArgV[0]);
    } 
    return 0;
}

int main(int iArgC, char *apArgV[]) {
    if (iArgC > 2) {
	double PI = 3.14159265358979323846;
        
        WELL512 *pWELL = new WELL512(s_aInit);
        long iNum   = atol(apArgV[1]);
        double dSigma = atof(apArgV[2]);
        double sum = 0;
        double sum2 = 0;

        for (int i = 0; i < iNum; i++) {
            double dR = pWELL->wgauss(dSigma);
            sum  += dR;
            sum2 += dR*dR;
        }
        double dAvg = sum/iNum;
        double dSig = sum2/iNum -  dAvg*dAvg;
        printf("N=%ld: mean %.12lf; sigm %.12lf\n", iNum, dAvg, dSig);
        printf("average number of rands pre gaussian: %f (theoretical: 4/PI=%f)\n", (1.0*pWELL->getCount())/iNum, 4/PI);
        delete pWELL;
    }
} 
