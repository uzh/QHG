#define NRANSI
#include "nrutil.h"
#include <stdio.h>

void polin2(double x1a[], double x2a[], double **ya, int m, int n, double x1,
	double x2, double *y, double *dy)
{
	void polint(double xa[], double ya[], int n, double x, double *y, double *dy);
	int j;
	double *ymtmp;
        

	ymtmp=dvector(1,m);
	for (j=1;j<=m;j++) {
		polint(x2a,ya[j],n,x2,&ymtmp[j],dy);
	}
	polint(x1a,ymtmp,m,x1,y,dy);
	free_dvector(ymtmp,1,m);
}
#undef NRANSI
