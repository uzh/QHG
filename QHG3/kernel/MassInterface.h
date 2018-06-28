#ifndef __MASSINTERFACE_H__
#define __MASSINTERFACE_H__

class MassInterface {
public:
    virtual double setMass(int iAgentIndex, double fMass) = 0;
    virtual double addMass(int iAgentIndex, double fMass) = 0;
    virtual double getMass(int iAgentIndex) = 0;
    virtual double getTotalMass(int iCellIndex) = 0;
    virtual double *getTotalMassArray() = 0;
    virtual double setSecondaryMass(int iAgentIndex, double fMass) = 0;
};

#endif
