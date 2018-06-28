#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_blas.h>

void usage(const char *pApp) {
    printf("%s - calculating PCO for a distance matrix\n", pApp);
    printf("Usage:\n");
    printf("  %s <size> <distmat> <output>\n", pApp);
    printf("where\n");
    printf("  size     number of matrix rows\n");
    printf("  distmat  ascii file containing distance matrix;\n");
    printf("           each line in the file corresponds to a row of the matrix\n");
    printf("  output   name of output file (if omitted, stdout is used)\n");
    printf("\n");
}


//----------------------------------------------------------------------------
// createCenteringMatrix
//  create centering matrix
//  see https://en.wikipedia.org/wiki/Centering_matrix
//
gsl_matrix *createCenteringMatrix(int n) {
    gsl_matrix *pI = gsl_matrix_alloc(n, n); 
    gsl_matrix_set_identity (pI);

    gsl_matrix *p1 = gsl_matrix_alloc(n, n); 
    gsl_matrix_set_all(p1, 1.0/n);

    gsl_matrix_sub(pI, p1);

    gsl_matrix_free(p1);
    return pI;
}

//----------------------------------------------------------------------------
// createRowMatrix
//  create a matrix where all elements in a row are equal
//  value for row i = v[i]
//
gsl_matrix *createRowMatrix(int n, gsl_vector *v) {
    gsl_matrix *d = gsl_matrix_calloc(n, n); 
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
        d->data[i*n+j] = v->data[j];
        }
    }
    return d;
}


//----------------------------------------------------------------------------
// readMatrix
//  read matrix from file
//  we assume n lines with n numbers each
//  lines starting with "#" are ignored
//
gsl_matrix *readMatrix(int n, const char *pData) {
    int iResult = -1;
    gsl_matrix *m = gsl_matrix_alloc(n, n);
    FILE *fIn = fopen(pData, "rt");
    int iSize = n*32;
    char *pLine = new char[n*32];

    int iL = 0;
    int iC = 0;
    if (fIn != NULL) {
        iResult = 0;
        char *p = fgets(pLine, iSize-1, fIn);
        while ((iResult == 0) && (p != NULL)) {
            iC = 0;
            char *px = strtok(p, " \t\n,;");
            while ((iResult == 0) && (px != NULL)) {
                char *pEnd;
                double d = strtof(px, &pEnd);
                if (*pEnd == '\0') {
                    m->data[n*iL+iC] = d;
                } else {
                    printf("Bad element in line %d, element #%d\n", iL, iC);
                    iResult = -1;
                }
                iC++;
                px = strtok(NULL, " \t\n,;");
            }
            if (iC != n) {
                printf("Bad number of elements in line %d: %d instead of %d\n", iL, iC, n);
                iResult = -1;
            }
            // next line
            iL++;
            p = fgets(pLine, iSize-1, fIn);
        }
        if (iC != n) {
            printf("Bad number of lines: %d instead of %d\n", iL, n);
            iResult = -1;
        }
        
        fclose(fIn);
        delete[] pLine;
    }

    
    if (iResult != 0) {
        gsl_matrix_free(m);
        m = NULL;
    }
    return m;
}


//----------------------------------------------------------------------------
// displayMatrix
//
template<typename T>
void displayMatrix(FILE *f,int n, T *m) {
   for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(f, "%+12.6f", m->data[n*i+j]);
        }
        fprintf(f, "\n");
    }    
}


//----------------------------------------------------------------------------
// main
//   compile like this:
//      g++ -g -Wall pco_2.cpp -lgsl 
//
int main (int iArgC, char *apArgV[]) {
    int iResult = -1;
    if (iArgC > 2) {
        int n = atoi(apArgV[1]);
        printf("Loading data\n");
        gsl_matrix *d = readMatrix(n,  apArgV[2]);
        if (d != NULL) {
            printf("Loaded data\n");
            gsl_matrix_mul_elements(d, d);

            printf("Preparing base matrix\n");
            gsl_matrix *j = createCenteringMatrix(n);

            gsl_matrix *dj = gsl_matrix_alloc(n, n); 
            gsl_blas_dgemm (CblasNoTrans, CblasNoTrans,  1.0, d, j,  0.0, dj);
    
            gsl_matrix *jdj = gsl_matrix_alloc(n, n); 
            gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, -0.5, j, dj, 0.0, jdj);
            printf("Prepared base matrix\n");

            printf("Doing eigen stuff\n");
            gsl_vector *eval = gsl_vector_alloc (n);
            gsl_matrix *evec = gsl_matrix_alloc (n, n);
            gsl_eigen_symmv_workspace *w = gsl_eigen_symmv_alloc(n);
            gsl_eigen_symmv(jdj, eval, evec, w);

            gsl_eigen_symmv_free (w);

            gsl_eigen_symmv_sort (eval, evec,  GSL_EIGEN_SORT_ABS_DESC);
            printf("Done eigen stuff\n");

        
            FILE *fOut = NULL;
            if (iArgC > 3) {
                fOut = fopen(apArgV[3], "wt");
            } else {
                fOut = stdout;
            }
            displayMatrix(fOut, n, evec);
            if (iArgC > 3) {
                fclose(fOut);
            }

            gsl_vector_free(eval);
            gsl_matrix_free(evec);
            gsl_matrix_free(jdj);
            gsl_matrix_free(dj);
            gsl_matrix_free(j);
            gsl_matrix_free(d);
            iResult = 0;
        } else {
            printf("Couldn't open or read [%s]\n", apArgV[2]);
        }
    } else {
        usage(apArgV[0]);
    }
    return iResult;
}
