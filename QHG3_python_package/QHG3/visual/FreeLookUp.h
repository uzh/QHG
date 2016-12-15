#ifndef __FREELOOKUP_H__
#define __FREELOOKUP_H__

#include "LookUp.h"

#include <vector>
#include <map>
#include <set>
#include <string>

struct ci_char_traits : public std::char_traits<char> {
    static bool eq(char c1, char c2) { return toupper(c1) == toupper(c2); }
    static bool ne(char c1, char c2) { return toupper(c1) != toupper(c2); }
    static bool lt(char c1, char c2) { return toupper(c1) <  toupper(c2); }
    static int compare(const char* s1, const char* s2, size_t n) {
        while( n-- != 0 ) {
            if( toupper(*s1) < toupper(*s2) ) return -1;
            if( toupper(*s1) > toupper(*s2) ) return 1;
            ++s1; ++s2;
        }
        return 0;
    }
    static const char* find(const char* s, int n, char a) {
        while( n-- > 0 && toupper(*s) != toupper(a) ) {
            ++s;
        }
        return s;
    }
};

typedef std::basic_string<char, ci_char_traits> ci_string;


typedef std::pair<ci_string, unsigned char>   byte_intensity; 
typedef std::pair<ci_string, double>          double_intensity; 

typedef std::vector<byte_intensity> byte_intensities;
typedef std::vector<double_intensity> double_intensities;
typedef std::map<ci_string, double> double_levels;

typedef std::pair<double, double> lineseg;

#define COL_R 0
#define COL_G 1
#define COL_B 2
#define COL_A 3
#define NUM_COLS 4

#define SPECIAL_NEGINF 0
#define SPECIAL_SUBMIN 1
#define SPECIAL_SUPMAX 2
#define SPECIAL_POSINF 3
#define SPECIAL_NANVAL 4
#define NUM_SPECIALS   5

static ci_string asSpecialNames[] = {
    "neginf",
    "submin",
    "supmax",
    "posinf",
    "nanval",
};

static ci_string asLevelNames[] = {
    "min",
    "max",
};
#define NUM_LEVELS 2

class FreeLookUp : public LookUp {


public:
    FreeLookUp(double dMinLevel, double dMaxLevel, 
               double_levels &diLevels, 
               double_intensities &vdiR, 
               double_intensities &vdiG, 
               double_intensities &vdiB, 
               double_intensities &vdiA);



    FreeLookUp(double dMinLevel, double dMaxLevel, 
               double_levels diLevels, 
               double_intensities *pvadiAll);

    virtual ~FreeLookUp() {};
    virtual void getColor(double dValue, double &dRed, double &dGreen, double &dBlue, double &dAlpha);

protected:
    int init(double_levels &diLevels, 
             double_intensities &vdiR, 
             double_intensities &vdiG, 
             double_intensities &vdiB, 
             double_intensities &vdiA);
    int testAndSetSpecial(double_intensity di, int iCol, double_levels diLevels, double *pdVal, double *pdLev);
    int getType(ci_string sName, double_levels &diLevels);

    int processIntensities(double_intensities &vbi, int iCol, double_levels diLevels);
    void defaultColors();

    double findLevel(double dValue, int iCol);
    double interpol(double dValue, double dLevel, int iCol);

    std::map<double, lineseg> m_mLineSegDefs[NUM_COLS];
    double                    m_adSpecial[NUM_SPECIALS][NUM_COLS];
    std::set<double>          m_vLevels[NUM_COLS];

};



#endif
