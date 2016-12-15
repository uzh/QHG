#ifndef __GENOME_PROVIDER_H__
#define __GENOME_PROVIDER_H__


class GenomeProvider {
public:
    virtual int getGenomeSize() = 0;
    virtual const ulong *getGenome(idtype iID) = 0;
};

#endif
