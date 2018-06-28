
#include <stdio.h>
#include <math.h>

static double s_coeff[6]={76.18009172947146,-86.50532032941677,
                          24.01409824083091,-1.231739572450155,
                          0.1208650973866179e-2,-0.5395239384953e-5};

static double s_c0 = 2.5066282746310005;
static double s_s0 = 1.000000000190015;


double gammaln(double xx) {
    double dResult = 0;

    double ser = s_s0;
    double x   = xx;
    double y   = xx+1;
    double tmp = x+5.5;

    tmp -= (x+0.5)*log(tmp);

    for (int k = 0; k <= 5; k++) {
        ser += s_coeff[k]/y;
        y++;
    }
    
    dResult = -tmp+log(s_c0*ser/x);
    return dResult;
}


#define MAX_ITERS 100
#define EPS 3.0e-7
#define FLOAT_MIN 1.0e-30

double betacf(double a, double b, double x) {

    double qab = a + b;
    double qap = a + 1.0;
    double qam = a - 1.0;
    double c = 1.0;
    double d = 1.0 - qab*x/qap;

    if (fabs(d) < FLOAT_MIN) {
        d=FLOAT_MIN;
    }

    d=1.0/d;
    double h=d;
    int m;
    for (m=1; m <= MAX_ITERS; m++) {
        int m2 = 2*m;
        double aa= m*(b-m)*x/((qam+m2)*(a+m2));
        d=1.0+aa*d;

        if (fabs(d) < FLOAT_MIN) {
            d = FLOAT_MIN;
        }

        c = 1.0+aa/c;
        if (fabs(c) < FLOAT_MIN) {
            c = FLOAT_MIN;
        }

        d = 1.0/d;
        h *= d*c;
        aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
        d = 1.0+aa*d;

        if (fabs(d) < FLOAT_MIN) {
            d=FLOAT_MIN;
        }

        c = 1.0+aa/c;
        if (fabs(c) < FLOAT_MIN) {
            c = FLOAT_MIN;
        }
        d = 1.0/d;
        double del=d*c;
        h *= del;
        if (fabs(del-1.0) < EPS) {
            break;
        }
    }
    if (m > MAX_ITERS) {
        fprintf(stderr, "a or b too big, or MAXITERS too small in betacf");
        h = -1;
    }
    return h;
}


double beta(double a, double b, double x) {
    double dResult = -1;
    double bt;

    if ((x < 0.0) || (x > 1.0)) {
        dResult = -1;
        fprintf(stderr, "Bad x value for function beta");
    } else {
        if ((x == 0.0) || (x == 1.0)) {
            bt=0.0;
	} else {
            bt =  exp(gammaln(a+b) - gammaln(a) - gammaln(b) + a*log(x) + b*log(1.0-x));
        }
        
        if (x < (a+1.0)/(a+b+2.0)) {
            dResult = bt*betacf(a,b,x)/a;
        } else {
            dResult = 1.0-bt*betacf(b,a,1.0-x)/b;
        }
    }

    return dResult;
}
