#include <stdio.h>
#include <string.h>
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <vector>

#include "utils.h"
#include "IDSampler2.h"


//----------------------------------------------------------------------------
// constructor
//
IDSample::IDSample() {
}


//----------------------------------------------------------------------------
// destructor
//
IDSample::~IDSample() {
    sampleinfo::iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        time_vagdata::iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                delete it_td->second[i];
            }
        }
        it_ltd->second.clear();
    }
    m_mmLocTimeAg.clear();
}


//----------------------------------------------------------------------------
// addAgentData
//  add a single agent data struct to the specified location and time
//
void IDSample::addAgentData(const char *pLocation, float fTimeStamp, agdata *pAD) {
    m_mmLocTimeAg[pLocation][fTimeStamp].push_back(pAD);
}


//----------------------------------------------------------------------------
// addAgentDataVec
//  add an entire vvector of agdata pointers to the specified location and time
void IDSample::addAgentDataVec(const char *pLocation, float fTimeStamp, std::vector<agdata*> &vAGD) {
    m_mmLocTimeAg[pLocation][fTimeStamp] = vAGD; 
}


//----------------------------------------------------------------------------
// getFullIDSet
//  fill sSelected with all ids
//
void IDSample::getFullIDSet(idset &sSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        time_vagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                sSelected.insert(it_td->second[i]->iID);
            }
        }
    }
}


//----------------------------------------------------------------------------
// getFullIndexIDMap
//  fill mSelected with Arrpos=>ID pairs
// 
void IDSample::getFullIndexIDMap(arrpos_ids &mSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        time_vagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                mSelected[it_td->second[i]->iArrayPos] = it_td->second[i]->iID;
            }
        }
    }
}


//----------------------------------------------------------------------------
// getLocationIDSet
//  map: location name => set of IDs
//
void IDSample::getLocationIDSet(loc_ids &msSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        time_vagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                msSelected[it_ltd->first].insert(it_td->second[i]->iID);
            }
        }
    }
}


//----------------------------------------------------------------------------
// getLocationIDSet
//  map: location name => set of agdata
//
void IDSample::getLocationADSet(loc_agd &msSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        time_vagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                msSelected[it_ltd->first].insert(it_td->second[i]);
            }
        }
    }
}


//----------------------------------------------------------------------------
// getIDADMap
//  map: ID => agdata
//
void IDSample::getIDADMap(id_agd &mIDAD) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        time_vagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                mIDAD[it_td->second[i]->iID] = it_td->second[i];
            }
        }
    }
}


//----------------------------------------------------------------------------
// write
//   getTimeLocationIDSet
//      map: (loc, time) => set(iDs)
//
void IDSample::getTimeLocationIDSet(tloc_ids &msSelected) const {
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); it_ltd != m_mmLocTimeAg.end(); ++it_ltd) {
        time_vagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); it_td != it_ltd->second.end(); ++it_td) {
            for (uint i = 0; i < it_td->second.size(); i++) {
                msSelected[std::pair<std::string, double>(it_ltd->first, it_td->first)].insert(it_td->second[i]->iID);
            }
        }
    }
}


//----------------------------------------------------------------------------
// write
//   m_mmiLocTimeAg: map: location name => (map: time => agdata list)
//   full ::= <nummaps><locmap>*
//   locmap ::= <namelen><locname><numsubmaps><submap>*
//   submap ::= <time><numagd><agdata>*
//
int IDSample::write(const char *pOutputFile) {
    int iResult = 0;
    FILE *fOut = fopen(pOutputFile, "wb");
    if (fOut != NULL) {
        int iNumMaps = m_mmLocTimeAg.size();
	int iWritten = fwrite(&iNumMaps, sizeof(int), 1, fOut);
        if (iWritten == 1) {
            sampleinfo::const_iterator it_ltd;
            for (it_ltd = m_mmLocTimeAg.begin(); (iResult == 0) && (it_ltd != m_mmLocTimeAg.end()); ++it_ltd) {

                const char *pName = it_ltd->first.c_str();
                int iLen = strlen(pName);
                iWritten =  fwrite(&iLen, sizeof(int), 1, fOut); 
                iWritten += fwrite(pName, strlen(pName), 1, fOut); 
                int iNumSubMaps = it_ltd->second.size();
                iWritten += fwrite(&iNumSubMaps, sizeof(int), 1, fOut);

                if (iWritten == 3) {
                    time_vagdata::const_iterator it_td;
                    for (it_td = it_ltd->second.begin(); (iResult == 0) && (it_td != it_ltd->second.end()); ++it_td) {
                        float fTime = it_td->first;
                        iWritten =  fwrite(&fTime, sizeof(float), 1, fOut);
                        int iNumDat = it_td->second.size();
                        iWritten += fwrite(&iNumDat, sizeof(int), 1, fOut);
                        if (iWritten == 2) {
                            iWritten = 0;
                            for (uint i = 0; i < it_td->second.size(); i++) {
                                iWritten += fwrite(it_td->second[i], sizeof(agdata), 1, fOut);   
                            }
                            if (iWritten != iNumDat) {
                                iResult = -1;
                                fprintf(stderr, "Couldn't write data items\n");
                            }
                        } else {
                            iResult = -1;
                            fprintf(stderr, "Couldn't write time and num data items\n");
                        }
                    }
                } else {
                    iResult = -1;
                    fprintf(stderr, "Couldn't write namelength name and num submaps\n");
                }
            }
        } else {
            iResult = -1;
            fprintf(stderr, "Couldn't write number of maps\n");
        }
    	fclose(fOut);
    } else {
        iResult = -1;
    	fprintf(stderr, "Couldn't open [%s] for writing\n", pOutputFile);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// read
//   m_mmiLocTimeAg: map: location name => (map: time => agdata list)
//   full ::= <nummaps><locmap>*
//   locmap ::= <namelen><locname><numsubmaps><submap>*
//   submap ::= <time><numagd><agdata>*
//   adata  ::= 
// 
//
int IDSample::read(const char *pInputFile) {
    int iResult = 0;
    FILE *fIn = fopen(pInputFile, "rb");
    if (fIn != NULL) {
        int iNumMaps;
	int iRead = fread(&iNumMaps, sizeof(int), 1, fIn);
        if (iRead != 1) {
            iResult = -1;
            fprintf(stderr, "Couldn't read num maps\n");
        }
	
	for(int i = 0; (iResult == 0) && (i < iNumMaps); i++) {
            int iLen;	  
	    iRead = fread(&iLen, sizeof(int), 1, fIn);

            char *pLoc = new char[iLen+1];
	    memset(pLoc, 0, iLen+1);
	    iRead += fread(pLoc, iLen, 1, fIn);

            int iNumSubMaps;
	    iRead += fread(&iNumSubMaps, sizeof(int), 1, fIn);
            if (iRead != 3) {
                iResult = -1;
                fprintf(stderr, "Couldn't read name len, name and num submaps\n");
            }
	    time_vagdata mvagd;
	    for (int j = 0; (iResult == 0) && (j < iNumSubMaps); j++) {

	    	float fTime;
	        iRead = fread(&fTime, sizeof(float), 1, fIn);
		std::vector<agdata*> vagd;
		int iNumDat;    
	        iRead += fread(&iNumDat, sizeof(int), 1, fIn);
                if (iRead == 2) {
                    iRead = 0;
                    for (int k = 0;  k < iNumDat; k++) {
                        agdata *pagd = new agdata();
                        iRead += fread(pagd, sizeof(agdata), 1, fIn);
                        vagd.push_back(pagd);
                    }
                    if (iRead == iNumDat) {
                        mvagd[fTime] = vagd;
                    } else {
                        iResult = -1;
                        fprintf(stderr, "Couldn't read data items\n");
                    }
                } else {
                    iResult = -1;
                    fprintf(stderr, "Couldn't read time and num data items\n");
                }

	    }
            m_mmLocTimeAg[pLoc] = mvagd;
	}
	
        iResult = 0;
    	fclose(fIn);
    } else {
        iResult = -1;
    	fprintf(stderr, "Couldn't open [%s] for reading\n", pInputFile);
    }
    printf("IDSample::read() returns %d\n",iResult);
    return iResult;
}


//----------------------------------------------------------------------------
// display
//   m_mmiLocTimeAg: map: location name => (map: time => agdata list)
//   full ::= <nummaps><locmap>*
//   locmap ::= <namelen><locname><numsubmaps><submap>*
//   submap ::= <time><numagd><agdata>*
//   adata  ::= 
// 
//
void IDSample::display() {
    int iResult = 0;
    int iNumMaps = m_mmLocTimeAg.size();
    printf("Num maps: %d\n", iNumMaps);
    sampleinfo::const_iterator it_ltd;
    for (it_ltd = m_mmLocTimeAg.begin(); (iResult == 0) && (it_ltd != m_mmLocTimeAg.end()); ++it_ltd) {
        const char *pName = it_ltd->first.c_str();
        int iLen = strlen(pName);
        int iNumSubMaps = it_ltd->second.size();
        printf("  [%s](%d) sub %d\n", pName, iLen, iNumSubMaps);
        
        time_vagdata::const_iterator it_td;
        for (it_td = it_ltd->second.begin(); (iResult == 0) && (it_td != it_ltd->second.end()); ++it_td) {
            float fTime = it_td->first;
            int iNumDat = it_td->second.size();
            printf("    %f dat %d\n", fTime, iNumDat);            
            for (uint i = 0; i < it_td->second.size(); i++) {
                printf("      item %d\n", i);
                printf("        ID      %ld\n", it_td->second[i]->iID);
                printf("        MomID   %ld\n", it_td->second[i]->iMomID);
                printf("        DadID   %ld\n", it_td->second[i]->iDadID);
                printf("        Gender  %d\n",  it_td->second[i]->iGender);
                printf("        CellID  %d\n",  it_td->second[i]->iCellID);
                printf("        lon     %f\n",  it_td->second[i]->dLon);
                printf("        lat     %f\n",  it_td->second[i]->dLat);
                printf("        arrpos  %d\n",  it_td->second[i]->iArrayPos);
            
            }
        }
    }
}




