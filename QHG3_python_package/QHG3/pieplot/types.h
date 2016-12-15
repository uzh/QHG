#ifndef __TYPES_H__
#define __TYPES_H__

#include <vector>
#include <set>
#include <string>
#include <map>


typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

typedef long long           llong;
typedef unsigned long long  ullong;
typedef long double         ldouble;

typedef std::set<int>          intset;
typedef intset::iterator       intset_it;
typedef intset::const_iterator intset_cit;

typedef int  gridtype;

typedef long idtype;
typedef std::set<idtype>       idset;
typedef idset::iterator        idset_it;
typedef idset::const_iterator  idset_cit;

typedef FILE *PFILE;

typedef ushort coord;
typedef ushort size;
typedef ushort spcid;


typedef std::pair<int, int>        IPOINT;
typedef std::pair<float, float>    FPOINT;
typedef std::pair<double, double>  DPOINT;
typedef std::pair<DPOINT*, double> DVPOINT;
typedef std::vector<DPOINT*>        VEC_POINTS;
typedef std::vector<std::string>    VEC_STRINGS;
typedef std::vector<std::string *>  VEC_PSTRINGS;
typedef std::vector<char>           VEC_CHARS;
typedef std::vector<int>            VEC_INTS;
typedef std::vector<bool>           VEC_BOOLS;
typedef std::vector<uint>           VEC_UINTS;
typedef std::vector<uchar>          VEC_UCHARS;
typedef std::vector<double>         VEC_DOUBLES;
typedef std::set<uchar>             SET_UCHARS;

/*
typedef std::vector<spcid>          VEC_SPCID;
typedef std::vector<coord>          VEC_COORDS;
typedef std::vector<float>          VEC_FLOATS;
typedef std::vector<IPOINT>         VEC_IPOINTS;
typedef std::vector<DVPOINT*>       VEC_VPOINTS;

typedef std::set<float>           SET_FLOATS;
typedef std::set<std::string>     SET_STRINGS;

typedef std::map<FPOINT, SET_UCHARS/ *, FPointComp* /> TRBACC;
typedef std::map<FPOINT, SET_UCHARS > TESTMAP;q
*/
typedef std::map<std::string, spcid> MAP_STR2SPC;



#endif
