#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "strutils.h"
#include "ParamReader.h"
#include "GeneUtils.h"
#include "GeneWriter.h"
#include "QDFUtils.h"
#include "QDFGenomeExtractor.h"


#define ATTR_GENOME_SIZE "Genetics_genome_size"
#define DATASET_GENOME   "Genome"


//----------------------------------------------------------------------------
// usage
//
void usage(char *pApp) {
    printf("%s - Creating ped and map files from genomes in a QDF population file\n", pApp);
    printf("Usage:\n");
    printf("%s -i <QDFPopFile> [-s <SpeciesName>] -o <OutputName> \n", pApp);
    printf("      --location-file=<LocationFile>\n");
    printf("      [-g <QDFGeoFile>]\n");
    printf("      [--attr-genome-size=<NameAttrGenomeSize>]\n");
    printf("      [--dataset-genome=<NameDSGenome>]\n");
    printf("where\n");
    printf("  QDFPopFile   QDF Population file with genome info\n");
    printf("  SpeciesName  Species name ( if omitted, first species will be used)\n");
    printf("  OutputName   Name for outpud '.ped' and '.map' files\n");
    printf("  SelectNum    Number of genomes to select (if omitted, all genomes will be used\n");
    printf("  Locationfile         name of location file (format: see below)\n");
    printf("  NameAttrGenomeSize   name of the genome size attribute in the QDF file (default \"%s\")\n", ATTR_GENOME_SIZE);
    printf("  NameDSGenome         name of the genome data set in the QDF file (default \"%s\")\n", DATASET_GENOME);
    printf("\n");
    printf("Location File Format\n");
    printf("  LocationFile ::= <LocationLine>*\n");     
    printf("  LocationLine ::= <identifier> <lon> <lat> <dist> <num> NL\n");     
    printf("  identifier   : string (name of location)\n");
    printf("  longitude    : double (longitude in degrees)\n");
    printf("  latitude     : double (latitude in degrees)\n");
    printf("  dist         : double (sample radius in km)\n");
    printf("  num          : int    (number of elements to sample)\n");
    printf("\n");

}
/*
//----------------------------------------------------------------------------
// displayData
//
int displayData(QDFGenomeExtractor *pQGE) {
    int iResult = 0;

    int iGenomeSize = pQGE->getGenomeSize();
    int iNumAgents =pQGE->getNumAgents();
    if (iNumAgents > -1) {
        const agentstuff *pAgentData = pQGE->getAgents();
        printf("num agents: %d\n", iNumAgents);
        double *pLon = pQGE->getLon();
        double *pLat = pQGE->getLon();
        if ((pLon != NULL) && (pLat != NULL)) {
            for (int i = 0; i < ((iNumAgents>10)?10:iNumAgents); i++) {
                printf("  #%d: id % 9ld, cell % 8d (% 7.2f,% 6.2f)\n", i, pAgentData[i].iAgentID,  pAgentData[i].iCellID, pLon[pAgentData[i].iCellID], pLat[pAgentData[i].iCellID]);
            }
        } else {
            for (int i = 0; i < ((iNumAgents>10)?10:iNumAgents); i++) {
                printf("  #%d: id % 9ld, cell % 8d (?,?)\n", i, pAgentData[i].iAgentID,  pAgentData[i].iCellID);
            }
        }
        if (iNumAgents>10) {
            printf("  ...\n");
        }
        int iNumBlocks = pQGE->getNumBlocks();
        int iNumBlocks2 = 2*iNumAgents*GeneUtils::numNucs2Blocks(iGenomeSize);
        if (iNumBlocks == iNumBlocks2) {
            const ulong *pGenomes = pQGE->getGenomes();
            printf("Read %d genome blocks (2 * %d nucleotides)",iNumBlocks, iNumBlocks*iGenomeSize);
            int iMax = 10*2*GeneUtils::numNucs2Blocks(iGenomeSize);
            for (int i = 0; i < ((iNumBlocks > iMax)?iMax:iNumBlocks); i++) {
                if (i % (2*GeneUtils::numNucs2Blocks(iGenomeSize)) == 0) {
                    printf("\n  #%d: ", i/(2*GeneUtils::numNucs2Blocks(iGenomeSize)));
                }
                printf("% ld ", pGenomes[i]);
            }
            printf("\n");
            if (iNumAgents>10) {
                printf("  ...\n");
            }

        } else {
            printf("Have %d blocks but should be %d\n", iNumBlocks, iNumBlocks2);
            iResult = -1;
        }
    } else {
        printf("No agents read\n");
        iResult = -1;
    }
    return iResult;
}
*/
//----------------------------------------------------------------------------
// writePlinkOutput
//
int writePlinkOutput(QDFGenomeExtractor *pQGE, const char *pOutputBody) {
    int iResult = -1;


    char sOutPed[256];
    char sOutMap[256];

    *sOutPed = '\0';
    *sOutMap = '\0';


    // get locations
    const locdata    &mLocDefs = pQGE->getLocData();

    // get agent data
    const IDSample *pSample = pQGE->getSample();

    // create name
    sprintf(sOutPed, "%s.asc", pOutputBody);
    GeneWriter::writeGenes(FORMAT_ASC, pQGE, sOutPed, mLocDefs, pSample);
    

    // write genes to ped file
    // create name
    sprintf(sOutPed, "%s.ped", pOutputBody);
    iResult = GeneWriter::writeGenes(FORMAT_PLINK, pQGE, sOutPed, mLocDefs, pSample);
    if (iResult == 0) {
    
        // create name for map file
        sprintf(sOutMap, "%s.map", pOutputBody);
        // write map file
        iResult = GeneUtils::writePlinkMapFile(sOutMap, pQGE->getGenomeSize());

    } else {
        printf("Couldn't write ped-file to [%s]\n", sOutPed);
    }

    if (iResult != 0) {
        // we don't care about the return value of remove
        remove(sOutPed);
        remove(sOutMap);
    }
    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = 0;
    char *pPopFile = NULL;
    char *pGeoFile = NULL;
    char *pSpecies = NULL;
    char *pOutputBody = NULL;
    char *pLocationFile = NULL;
    char *pAttGen = NULL;
    char *pDSGen  = NULL;

    ParamReader *pPR = new ParamReader();
    if (pPR != NULL) {
        pPR->setVerbose(true);
        bool bOK = pPR->setOptions(7,
                                   "-i:S!",  &pPopFile,
                                   "-g:S",   &pGeoFile,
                                   "-s:S",   &pSpecies,
                                   "-o:S!",  &pOutputBody,
                                   "--location-file:S!",   &pLocationFile,
                                   "--attr-genome-size:S", &pAttGen,
                                   "--dataset-genome:S",   &pDSGen);
        if (bOK) {
            iResult = pPR->getParams(iArgC, apArgV);
            if (iResult >= 0) {

                // set a default if no gender name is given
                // (attention: pGenderName will be deleted by ParamReader)
                char *pAttGen2     = defaultIfNULL(pAttGen,     ATTR_GENOME_SIZE);
                char *pDSGen2      = defaultIfNULL(pDSGen,      DATASET_GENOME);
                
                QDFGenomeExtractor *pQGE = NULL;
                if (pGeoFile != NULL) {
                    pQGE = QDFGenomeExtractor::createInstance(pGeoFile, pPopFile, pSpecies, pAttGen2, pDSGen2);
                } else {
                    pQGE = QDFGenomeExtractor::createInstance(pPopFile, pSpecies, pAttGen2, pDSGen2);
                }
                if (pQGE != NULL) {
                    // create a random selection of indexes
                    iResult = pQGE->createSelection(pLocationFile);
                    if (iResult == 0) {
                        printf("selected %d ids:", pQGE->getNumSelected());
                        const idset &sSelected = pQGE->getSelectedIDs();
                        for (idset::const_iterator it = sSelected.begin(); it != sSelected.end(); ++it) {
                            printf(" %ld", *it);
                        }
                        printf("\n");
                    } else {
                        printf("error creating selection\n");
                    }
                



                    if (iResult == 0) {
                        
                        iResult = writePlinkOutput(pQGE, pOutputBody);
                        if (iResult == 0) {
                            printf("outputs written to %s.ped and %s.map\n", pOutputBody, pOutputBody);
                            printf("+++ success +++\n");
                        } else {
                            printf("failed writing outputs\n");
                        }
                    }
                }
                // write output
               
                delete[] pDSGen2;
                delete[] pAttGen2;
                delete pQGE;
                
                
                
            } else {
                usage(apArgV[0]);
            }
        } else {
            printf("Couldn't set ParamReader options\n");
        }
        delete pPR;
    } else {
        printf("Couldn't create ParamReader\n");
    }

    return iResult;
}
