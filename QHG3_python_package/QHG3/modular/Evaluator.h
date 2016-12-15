#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__


class Evaluator {
public:
    virtual ~Evaluator() {};
    virtual int initialize(float fTime)=0;
    virtual int preLoop(){ return 0;};
    virtual int postLoop(){ return 0;};
    
    virtual void setOutputWeights(double *adOutput)=0;

    virtual int finalize(float fTime) {return 0;};

    // get action parameters from QDF 
    virtual int extractParamsQDF(hid_t hSpeciesGroup) { return 0; };

    // write action parameters to QDF
    virtual int writeParamsQDF(hid_t hSpeciesGroup) {return 0; };

    // read action parameters from ascii POP file
    virtual int tryReadParamLine(char *pLine) { return 0; }; // we need to return 0 for Prioritizer

};


#endif
