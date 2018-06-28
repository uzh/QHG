#ifndef __PROTEINNBUILDER_H__
#define __PROTEINNBUILDER_H__

#include <vector>

#include "types.h"

const char aNucleotides[4] ={'A', 'C', 'G', 'T'};

// Amino acid symbols and short names
// note: M (ATG) is also the start symbol
const char aAminoAcids[][2][8] = {
    {"A", "Ala"}, {"C", "Cys"}, {"D", "Asp"}, {"E", "Glu"}, 
    {"F", "Phe"}, {"G", "Gly"}, {"H", "His"}, {"I", "Ile"},
    {"K", "Lys"}, {"L", "Leu"}, {"M", "Met"}, {"N", "Asn"},
    {"P", "Pro"}, {"Q", "Gln"}, {"R", "Arg"}, {"S", "Ser"},
    {"T", "Thr"}, {"V", "Val"}, {"W", "Trp"}, {"Y", "Tyr"},
    {"X", "Stp"}
};

#define AA_ALA    0
#define AA_CYS    1
#define AA_ASP    2
#define AA_GLU    3
#define AA_PHE    4
#define AA_GLY    5
#define AA_HIS    6
#define AA_ILE    7
#define AA_LYS    8
#define AA_LEU    9
#define AA_MET   10
#define AA_ASN   11
#define AA_PRO   12
#define AA_GLN   13
#define AA_ARG   14
#define AA_SER   15
#define AA_THR   16
#define AA_VAL   17
#define AA_TRP   18
#define AA_TYR   19
#define AA_STP   20
// MET also codes for start
#define AA_STR   AA_MET
// some symbols for parsing
#define AA_NUL          32
#define AA_END_TOTAL    33
#define AA_END_ALLELE   34

// aTranslationTable assigns
//   index of nucleotide triplet => AminoAcid Index
// (index of "XYZ": X*16 + Y*4 +Z; where A=0; C=1; G=2; T=3)
const int aTranslationTable[64+3] = {
    AA_LYS, AA_ASN, AA_LYS, AA_ASN, AA_THR, AA_THR, AA_THR, AA_THR,
    AA_ARG, AA_SER, AA_ARG, AA_SER, AA_ILE, AA_ILE, AA_MET, AA_ILE,
    AA_GLN, AA_HIS, AA_GLN, AA_HIS, AA_PRO, AA_PRO, AA_PRO, AA_PRO,
    AA_ARG, AA_ARG, AA_ARG, AA_ARG, AA_LEU, AA_LEU, AA_LEU, AA_LEU, 
    AA_GLU, AA_ASP, AA_GLU, AA_ASP, AA_ALA, AA_ALA, AA_ALA, AA_ALA, 
    AA_GLY, AA_GLY, AA_GLY, AA_GLY, AA_VAL, AA_VAL, AA_VAL, AA_VAL, 
    AA_STP, AA_TYR, AA_STP, AA_TYR, AA_SER, AA_SER, AA_SER, AA_SER, 
    AA_STP, AA_CYS, AA_TRP, AA_CYS, AA_LEU, AA_PHE, AA_LEU, AA_PHE,
    // pseudo aminno acids
    AA_NUL, AA_END_TOTAL, AA_END_ALLELE, 
};

#define MD5_SIZE 16

typedef std::vector<uchar>   protein;
typedef std::vector<protein> proteome;
typedef std::vector<ulong*> prothash; 
class ProteinBuilder {
public:
    static ProteinBuilder *createInstance(const ulong *pGenome, int iGenomeSize);
    ~ProteinBuilder(); 
    static const char *getAAName(int iAAIndex); 
    static char        getAASymbol(int iAAIndex);
    
 
    int translateGenome();
    const proteome &getProteome() { return m_vProteome;};

    int translateGenomeHash();
    const prothash &getProtHash() { return m_vAAHash;};
    void displaySequence();
protected:
    ProteinBuilder();

    int init(const ulong *pGenome, int iGenomeSize);
    uchar getNextAA();
    uchar getNextNuc();

    int    m_iCurPos;
    int    m_iCurBlockIndex;
    ulong  m_lCurBlock;
    int    m_iCurAllele;

    int    m_iNumBlocks;


    proteome m_vProteome;
    std::vector<std::pair<int, int> > m_vAll[2];

    const ulong *m_pGenome;
    int    m_iGenomeSize;

    prothash m_vAAHash;
};



#endif
