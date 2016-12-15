
void splie2(double x1a[], double x2a[], double **ya, int m, int n, double **y2a)
{
	void spline(double x[], double y[], int n, double yp1, double ypn, double y2[]);
	int j;
	for (j=1;j<=m;j++)
            spline(x2a,ya[j],n,1.0e30,1.0e30,y2a[j]);
            

 
}

