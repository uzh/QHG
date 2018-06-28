#ifndef __SQEUNCEDIST_H__
#define __SQEUNCEDIST_H__

#include <stdio.h>

#include <map>
#include <vector>
#include <algorithm>

#include "strutils.h"
#include "QDFUtils.h"
#include "QDFArray.h"
#include "DistMat.h"

#include "types.h"


#define TEMPLATE_DIST_MAT  "%s.full.mat"
#define TEMPLATE_REF_MAT   "%s.ref.mat"
#define TEMPLATE_TABLE     "%s.tagdists"
#define TEMPLATE_GEO_SEQ   "%s.geoseq"

#define IDBUFSIZE 4096

typedef std::pair<std::string,double>               loc_time;
typedef std::map<loc_time, std::vector<idtype> >    tnamed_ids;
typedef std::map<idtype, std::pair<double,double> > id_locs;
typedef std::map<idtype,int>                        id_node;
typedef std::map<int,int>                           node_index;
typedef std::map<int, float>                        node_value;
typedef std::map<idtype, double>                    id_value;


template<typename T>
class SequenceDist {
public:
    typedef std::map<idtype, T*> id_sequences;
    typedef typename DistMat<T>::calcdist_t calcdist_t;
    SequenceDist(int iNumSequences, 
                 int iNumSequenceSize, 
                 id_node      &mIDNodes, 
                 id_sequences &mIDSeq, 
                 tnamed_ids   &mvIDs, 
                 id_locs      &mIdLocs,  
                 calcdist_t fCalcDist);

    virtual ~SequenceDist();

    int prepareNodes(const char *pGridQDF, const char *pMoveStatQDF);
    int distGeoSequences2(const char *pOutput);
    int createAndWriteDistMat(const char *pOutput);

    int extractAndWriteGeoPhenomeDists(const char *pOutput);
protected:

    int loadSequences(const char *pSeqFile);

    int nodeIdsToIndexes(const char *pGridQDF, std::vector<int> &vNodeIDs, std::map<int, int> &mNodeIndexes);
    int nodeIdsToStat(const char *pStatQDF, std::map<int, int> &mNodeIndexes, std::map<int, float> &mNodeStats, const char *pStatName);
    int nodeIdsToDistances(const char *pStatQDF, std::map<int, int> &mNodeIndexes, std::map<int, float> &mNodeDistances);
    int nodeIdsToTravelTimes(const char *pStatQDF, std::map<int, int> &mNodeIndexes, std::map<int, float> &mNodeTimes);
    
    T **reorderSequences(id_sequences &mIDSeq, tnamed_ids &mvIDs);


    int writeFullMatrix(const char *pOutput, float **pM, int iNumSequences1, int iNumSequences2);
    int createAndWriteDistMatRef(id_sequences &mIDSeqRef, tnamed_ids &mvIDsRef, const char *pOutput);
    int extractAndWriteGeoSequenceDists(const char *pOutput);
    int selectFromClosest(std::map<int, float> &mNodeDistances, 
                          std::map<int, int>   &mNodeIndexes, 
                          id_sequences &mIDSeqRef,
                          tnamed_ids &mvIDsRef);

    int m_iSequenceSize;
    int m_iNumSequences;
    
    id_node       m_mIDNodes;
    tnamed_ids    m_mvIDs;
    id_locs       m_mIDLocs;
    
    id_sequences  m_mIDSeq;

    node_index    m_mNodeIndexes;     // map: node ID   -> node index
    node_value    m_mNodeDistances;   // map: node ID   -> distance travelled
    node_value    m_mNodeTimes;       // map: node ID   -> travel time
    id_value      m_mTravelDists;  // map: agent ID  -> distance travelled
    id_value      m_mTravelTimes;  // map: agent ID  -> travel time

    typename DistMat<T>::calcdist_t m_fcalcdist;

};


#endif
