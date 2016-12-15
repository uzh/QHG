/* CAUTION: This is the ANSI C (only) version of the Numerical Recipes
   utility file nrutil.h.  Do not confuse this file with the same-named
   file nrutil.h that is supplied in the 'misc' subdirectory.
   *That* file is the one from the book, and contains both ANSI and
   traditional K&R versions, along with #ifdef macros to select the
   correct version.  *This* file contains only ANSI C.               */

#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_
/*
static double maxarg1 = 0;
static double maxarg2 = 0;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
        (maxarg1) : (maxarg2))

static int iminarg1 = 0;
static int iminarg2 = 0;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
        (iminarg1) : (iminarg2))

static float sqrarg = 0;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

static double dsqrarg;
#define DSQR(a) ((dsqrarg=(a)) == 0.0 ? 0.0 : dsqrarg*dsqrarg)

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
*/
#define FMAX(a,b) ((a) > (b) ? (a) : (b))

#define IMIN(a,b) ((a) < (b) ? (a) : (b))

#define SQR(a) ((a) == 0.0 ? 0.0 : (a)*(a))

#define DSQR(a) ((a) == 0.0 ? 0.0 : (a)*(a))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))


void nrerror(const char error_text[]);

double *dvector(long nl, long nh);

void free_dvector(double *v, long nl, long nh);

int *ivector(long nl, long nh);

void free_ivector(int *v, long nl, long nh);

double **dmatrix(long nrl, long nrh, long ncl, long nch);

void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch);

#endif /* _NR_UTILS_H_ */
