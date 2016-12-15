#ifndef __IDS_H__
#define __IDS_H__

#include <map>
#include <string>

#include "types.h"

#define GET_SPC_SUFF(x) ((x) & 0xffff)

// typedef std::map<int, std::string> MAP_INT_STRING;

// static MAP_INT_STRING g_mapSpeciesNames;

const spcid SPC_ALL  = 0xffff;
const spcid SPC_NONE = 0xfffe;
const spcid CLASS_NONE = 0xfffe;

//-- vegetation: 0 - 99
// must start with 0, no "holes"
const spcid CLASS_VEG = 0;
const spcid SPC_GRASS = 0;
const spcid SPC_BUSH  = 1;
const spcid SPC_TREE  = 2;
const spcid MAX_VEG = 99;
const unsigned int NUM_VEG = 3;

const spcid CLASS_ALTDENSGENMOVER = 101;
const spcid CLASS_ALTDENSMOVER = 102;
const spcid CLASS_ALTMOVERFK = 103;
const spcid CLASS_ALTMOVER = 104;
const spcid CLASS_ANC4ALTMOVER = 105;
const spcid CLASS_ANCCAPACITY = 106;
const spcid CLASS_ANCALTMOVER = 107;
const spcid CLASS_COASTDWELLER = 108;
const spcid CLASS_DIRTEST = 109;
const spcid CLASS_EXAMPLE = 110;
const spcid CLASS_FK1D = 111;
const spcid CLASS_FK = 112;
const spcid CLASS_GENETICMOVER = 113;
const spcid CLASS_GENLANDDWELLERCONF = 114;
const spcid CLASS_GENLANDDWELLER = 115;
const spcid CLASS_LANDDWELLER = 116;
const spcid CLASS_LVPRED = 117;
const spcid CLASS_LVPREY = 118;
const spcid CLASS_PDALTPRED = 119;
const spcid CLASS_PDALTPREY = 120;
const spcid CLASS_PDPRED = 121;
const spcid CLASS_PDPREY = 122;
const spcid CLASS_SIMPLEPRED = 123;
const spcid CLASS_SIMPLEPREY = 124;
const spcid CLASS_VEGDWELLER = 125;



// if you add new SEL_XXX constants:
// --> update SetValue
const int SEL_NONE  = 0xfffe;

typedef struct {
    const char *pName;
    int   iVal;
} NameIVal;


static const NameIVal s_aClasses[] = {
    {"Vegetation", CLASS_VEG},
    {"AltDensGenMoverPop", CLASS_ALTDENSGENMOVER},
    {"AltDensMoverPop", CLASS_ALTDENSMOVER},
    {"AltMoverFKPop", CLASS_ALTMOVERFK},
    {"AltMoverPop", CLASS_ALTMOVER},
    {"Anc4AltMoverPop", CLASS_ANC4ALTMOVER},
    {"AncCapacityPop", CLASS_ANCCAPACITY},
    {"AncestorAltMoverPop", CLASS_ANCALTMOVER},
    {"CoastDwellerPop", CLASS_COASTDWELLER},
    {"DirTestPop", CLASS_DIRTEST},
    {"ExamplePop", CLASS_EXAMPLE},
    {"FK1DPop", CLASS_FK1D},
    {"FKPop", CLASS_FK},
    {"GeneticMoverPop", CLASS_GENETICMOVER},
    {"GenLandDwellerConfPop", CLASS_GENLANDDWELLERCONF},
    {"GenLandDwellerPop", CLASS_GENLANDDWELLER},
    {"LandDwellerPop", CLASS_LANDDWELLER},
    {"LVPredPop", CLASS_LVPRED},
    {"LVPreyPop", CLASS_LVPREY},
    {"PDAltPredPop", CLASS_PDALTPRED},
    {"PDAltPreyPop", CLASS_PDALTPREY},
    {"PDPredPop", CLASS_PDPRED},
    {"PDPreyPop", CLASS_PDPREY},
    {"SimplePredPop", CLASS_SIMPLEPRED},
    {"SimplePreyPop", CLASS_SIMPLEPREY},
    {"VegDwellerPop", CLASS_VEGDWELLER},

};

static const NameIVal s_aSpecies2[] = {
    {"Grass",  SPC_GRASS},
    {"Bush",   SPC_BUSH},
    {"Tree",   SPC_TREE},
};





spcid spcValue(const char *pSpcName);
const char *spcName(spcid i);
spcid clsValue(const char *pSpcName);
const char *clsName(spcid i);


static const char REVISION[] = "";
#endif

