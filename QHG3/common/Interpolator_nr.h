#ifndef __INTERPOLATOR_NR_H__
#define __INTERPOLATOR_NR_H__

// utility functions from Numerical Recipes

void splie2(double x1a[], double x2a[], double **ya, int m, int n, double **y2a);
void splin2(double x1a[], double x2a[], double **ya, double **y2a, int m, int n,
        double x1, double x2, double *y);
void spline(double x[], double y[], int n, double yp1, double ypn, double y2[]);
void splint(double xa[], double ya[], double y2a[], int n, double x, double *y);
void polin2(double x1a[], double x2a[], double **ya, int m, int n,
        double x1, double x2, double *y, double *dy);
void polint(double xa[], double ya[], int n, double x, double *y, double *dy);

#endif
