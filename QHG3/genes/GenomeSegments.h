#ifndef __GENOMESEGMENTS_H__
#define __GENOMESEGMENTS_H__

#include <map>
#include "types.h"

class segment {
public:
    uint m_iStartNuc;    // starting nucleotide in first block
    uint m_iLengthNuc;   // length in nucleotides
    uint m_iFirstBlock;
    uint m_iMaskLength;
    ulong *m_pMasks;

    segment(uint iStartNuc0, uint iLengthNuc0, uint iFirstBlock0); 
    virtual ~segment();
    int createMask();
    void showMask();
};

typedef std::vector<segment*>   segmentlist;

class GenomeSegments {
public:
    // regular segments
    static GenomeSegments *createInstance(uint iGenomeSize, uint iNumSegments=0);
 
    int addSegment(uint iStartNuc, int iLengthNuc);

    ulong *getMaskedSegment(const ulong *pFullGenome, int iSegmentNumber);
    //    static ulong *getMaskedSegment(ulong *pFullGenome, int iGenomeSize, uint iStartNuc, uint iLengthNuc);

    virtual ~GenomeSegments();

    int getNumSegments() { return m_vSegments.size();};
    int getSegmentSize(uint i);
    int getFirstBlock(uint i);
    int getMaskLength(uint i);

    int findSegmentForNuc(uint iNucPos);

    void showMasks();
    
protected:
    GenomeSegments(uint iGenomeSize);
    int init(uint iNumSegments, bool bEqualize);
    int createMask(segment *ps);

    uint m_iGenomeSize;
    int m_iNumBlocks;

    segmentlist m_vSegments;
    ulong      *m_pWorkspace; // we copy genomes hereand apply the masks    
};


#endif
