
#ifndef __XRAND_H__
#define __XRAND_H__

void tabrandinit(unsigned int iSeed);
unsigned int get_seed();

unsigned int tabrand();

double gauss_rand();

#define uniform_randr(d0, d1) ( (d0)+rand()*( ((1.0*(d1))-(d0))/RAND_MAX ) )
#define uniform_randt(d0, d1) ( (d0)+tabrand()*( ((1.0*(d1))-(d0))/0xffffffff ) )
#define uniform_randd(d0, d1) ( (d0)+drand48()*((d1)-(d0)) )

#define uniform_rand(d0, d1) uniform_randd((d0),(d1))
//#define uniform_rand(d0, d1) uniform_randr((d0),(d1))

#endif
