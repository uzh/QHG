#include <math.h>
#include "Latitudinal.h"
#include "utils.h"

double Latitudinal::calc(double dLon, double dLat) {
    return cos(M_PI*dLat/180);
}
