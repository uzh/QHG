
#include <stdlib.h>
#include <math.h>
#include "utils.h" 
#include "GeoInfo.h" 
#include "Projector.h" 
#include "GridProjection.h"
#include "LCCProjector.h"


ProjType pt1(
    PR_LAMBERT_AZIMUTHAL_EQUAL_AREA,
    15.5*M_PI/180,
    44.4*M_PI/180,
    0,
    NULL
);

double adAdd[] = {30.0*M_PI/180, 60.0*M_PI/180};

ProjType pt2(
    PR_LAMBERT_CONFORMAL_CONIC,
    12.5*M_PI/180,
    53.0*M_PI/180,
    2,
    adAdd
);

ProjGrid pg1(
    1200,
    900,
    4800.0,
    3600.0,
    600.0,
    450.0,
    RADIUS_EARTH_KM
);

ProjGrid pg2(
    83,
    56,
    4980.0,
    3360.0,
    0.0,
    0.0,
    RADIUS_EARTH_KM
);
int main(int iArgC, char *apArgV[]) {

  
    GeoInfo *pGI = GeoInfo::instance();

    Projector *pr1 = pGI->createProjector(&pt1);
    Projector *pr2 = pGI->createProjector(&pt2);
    //((LambertConformalConicalProjector *)p2)->setAdditional(pt2.m_iNumAdd, pt2.m_pdAdd);

    GridProjection *gp1 = new GridProjection(&pg1, pr1, false, false);
    GridProjection *gp2 = new GridProjection(&pg2, pr2, false, false);

    Projector *pr      = pr1;
    GridProjection *gp = gp1;

    double dLon;
    double dLat;
    double dGridX;
    double dGridY;

    dLon = -17.723;
    dLat =  52.328;
    bool bToGrid = true;
    if (iArgC > 3) {
	if (*(apArgV[1]) == 'g') {
	    bToGrid = true;
            dLon = atof(apArgV[2]);
            dLat = atof(apArgV[3]);
        } else {
            bToGrid = false;
            dGridX = atof(apArgV[2]);
            dGridY = atof(apArgV[3]);
        }
    }

    if (bToGrid) {
        gp->sphereToGrid(DEG2RAD(dLon), DEG2RAD(dLat), dGridX, dGridY);
        printf("Lon %f, Lat %f -> (%f,%f)\n", dLon, dLat, dGridX, dGridY);

        gp->gridToSphere(dGridX, dGridY, dLon, dLat);
        // get longitude/latitude in degrees
        dLat =  RAD2DEG(dLat);
        dLon =  RAD2DEG(dLon);
        printf("(%f,%f) -> Lon %f, Lat %f\n", dGridX, dGridY, dLon, dLat);
   

    } else {
        gp->gridToSphere(dGridX, dGridY, dLon, dLat);
        printf("(%f,%f) -> Lon %f, Lat %f\n", dGridX, dGridY, dLon, dLat);
        double dLatD =  RAD2DEG(dLat);
        double dLonD =  RAD2DEG(dLon);
        printf("(%f,%f) -> Lon %f, Lat %f\n", dGridX, dGridY, dLonD, dLatD);

        gp->sphereToGrid(dLon, dLat, dGridX, dGridY);
        printf("Lon %f, Lat %f -> (%f,%f)\n", dLonD, dLatD, dGridX, dGridY);
    }
    /*
    dGridX = 1;
    dGridY = 1;
    gp->gridToSphere(dGridX, dGridY, dLon, dLat);
    printf("(%f,%f) -> Lon %f, Lat %f\n", dGridX, dGridY, dLon, dLat);


    dLon = -17.723*M_PI/180;
    dLat =  52.328*M_PI/180;
    pr->sphere2Plane(dLon, dLat, dGridX, dGridY);
    printf("pr Lambda %f, Phi %f -> (%f,%f)\n", dLon, dLat, dGridX, dGridY);
    dGridX *= pd2.m_dRadius/60.0;
    dGridY *= pd2.m_dRadius/60.0;
    dGridX += 41.5;
    dGridY += 28.0;
    
    printf("                     -> (%f,%f)\n", dGridX, dGridY);
    
    pr->plane2Sphere(dGridX, dGridY, dLon, dLat);
    printf("pr (%f,%f) -> Lambda %f, Phi %f\n", dGridX, dGridY, dLon, dLat);
    */


    delete gp1;
    delete gp2;
    delete pr1;
    delete pr2;
}
