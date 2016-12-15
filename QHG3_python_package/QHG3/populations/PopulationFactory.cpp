#include <stdio.h>
#include <string.h>

#include "ids.h"
#include "strutils.h"
#include "LineReader.h"
#include "PopulationFactory.h"
#include "PopBase.h"
#include "PopFinder.h"
#include "SCellGrid.h"
#include "SPopulation.h"

#include "IDGen.h"

// include headers for populations

#include "../populations/AltDensGenMoverPop.h"
#include "../populations/AltDensMoverPop.h"
#include "../populations/AltMoverFKPop.h"
#include "../populations/AltMoverPop.h"
#include "../populations/Anc4AltMoverPop.h"
#include "../populations/AncCapacityPop.h"
#include "../populations/AncestorAltMoverPop.h"
#include "../populations/CoastDwellerPop.h"
#include "../populations/DirTestPop.h"
#include "../populations/ExamplePop.h"
#include "../populations/FK1DPop.h"
#include "../populations/FKPop.h"
#include "../populations/GeneticMoverPop.h"
#include "../populations/GenLandDwellerConfPop.h"
#include "../populations/GenLandDwellerPop.h"
#include "../populations/LandDwellerPop.h"
#include "../populations/LVPredPop.h"
#include "../populations/LVPreyPop.h"
#include "../populations/PDAltPredPop.h"
#include "../populations/PDAltPreyPop.h"
#include "../populations/PDPredPop.h"
#include "../populations/PDPreyPop.h"
#include "../populations/SimplePredPop.h"
#include "../populations/SimplePreyPop.h"
#include "../populations/VegDwellerPop.h"




//----------------------------------------------------------------------------
// constructor
//
PopulationFactory::PopulationFactory(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, 
                                     IDGen **apIDG, uint32_t *aulState)
    : m_pCG(pCG),
      m_pPopFinder(pPopFinder),       
      m_iLayerSize(iLayerSize),
      m_apIDG(apIDG),
      m_aulState(aulState) {
}

//----------------------------------------------------------------------------
// readClass
//   determine class id from line.
//   line format:  "CLASS <classname>"
//   Uses clsValue() to obtain class id
//
spcid PopulationFactory::readClass(LineReader *pLR) {
    spcid ci = CLASS_NONE;
    
    if (pLR != NULL) {
        char *pLine = pLR->getNextLine();
        if (strstr(pLine, "CLASS") == pLine) {
            pLine += strlen("CLASS");
            pLine = trim(pLine);
            char sClassName[64];
            int iRet = sscanf(pLine, "%s", sClassName);
            if (iRet == 1) {
                ci = clsValue(sClassName);
                if (ci == CLASS_NONE) {
                    printf("Not a known class ID [%s]\n", sClassName);
                }
            } else {
                printf("Expected class name after \"CLASS\" [%s]\n", pLine);
            }
        } else {
            printf("Expected keyword \"CLASS\" [%s]\n", pLine);
        }
    } else {
        printf("LineReader is NULL\n");
    }
    return ci;
}


//----------------------------------------------------------------------------
// createPopulation
//
PopBase *PopulationFactory::createPopulation(const char *pClassName) {
    PopBase *pPop = NULL;
    spcid iClassID = clsValue(pClassName);
    if (iClassID != CLASS_NONE) {
        printf("Creating Population [%s] (id %d)\n", pClassName, iClassID);
        pPop = createPopulation(iClassID);
    } else {
        printf("No class id found for class name [%s]\n", pClassName);
    }            
    return pPop;
}


//----------------------------------------------------------------------------
// createPopulation
//
PopBase *PopulationFactory::createPopulation(spcid iClassID) {
    PopBase *pPop = NULL;
    bool bVerbose = true;
    if (m_pCG != NULL) {
        switch (iClassID) {

		case CLASS_ALTDENSGENMOVER:
			if (bVerbose) { printf("PopulationFactory is creating AltDensGenMoverPop\n"); }
			pPop = new AltDensGenMoverPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_ALTDENSMOVER:
			if (bVerbose) { printf("PopulationFactory is creating AltDensMoverPop\n"); }
			pPop = new AltDensMoverPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_ALTMOVERFK:
			if (bVerbose) { printf("PopulationFactory is creating AltMoverFKPop\n"); }
			pPop = new AltMoverFKPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_ALTMOVER:
			if (bVerbose) { printf("PopulationFactory is creating AltMoverPop\n"); }
			pPop = new AltMoverPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_ANC4ALTMOVER:
			if (bVerbose) { printf("PopulationFactory is creating Anc4AltMoverPop\n"); }
			pPop = new Anc4AltMoverPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_ANCCAPACITY:
			if (bVerbose) { printf("PopulationFactory is creating AncCapacityPop\n"); }
			pPop = new AncCapacityPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_ANCALTMOVER:
			if (bVerbose) { printf("PopulationFactory is creating AncestorAltMoverPop\n"); }
			pPop = new AncestorAltMoverPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_COASTDWELLER:
			if (bVerbose) { printf("PopulationFactory is creating CoastDwellerPop\n"); }
			pPop = new CoastDwellerPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_DIRTEST:
			if (bVerbose) { printf("PopulationFactory is creating DirTestPop\n"); }
			pPop = new DirTestPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_EXAMPLE:
			if (bVerbose) { printf("PopulationFactory is creating ExamplePop\n"); }
			pPop = new ExamplePop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_FK1D:
			if (bVerbose) { printf("PopulationFactory is creating FK1DPop\n"); }
			pPop = new FK1DPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_FK:
			if (bVerbose) { printf("PopulationFactory is creating FKPop\n"); }
			pPop = new FKPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_GENETICMOVER:
			if (bVerbose) { printf("PopulationFactory is creating GeneticMoverPop\n"); }
			pPop = new GeneticMoverPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_GENLANDDWELLERCONF:
			if (bVerbose) { printf("PopulationFactory is creating GenLandDwellerConfPop\n"); }
			pPop = new GenLandDwellerConfPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_GENLANDDWELLER:
			if (bVerbose) { printf("PopulationFactory is creating GenLandDwellerPop\n"); }
			pPop = new GenLandDwellerPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_LANDDWELLER:
			if (bVerbose) { printf("PopulationFactory is creating LandDwellerPop\n"); }
			pPop = new LandDwellerPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_LVPRED:
			if (bVerbose) { printf("PopulationFactory is creating LVPredPop\n"); }
			pPop = new LVPredPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_LVPREY:
			if (bVerbose) { printf("PopulationFactory is creating LVPreyPop\n"); }
			pPop = new LVPreyPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_PDALTPRED:
			if (bVerbose) { printf("PopulationFactory is creating PDAltPredPop\n"); }
			pPop = new PDAltPredPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_PDALTPREY:
			if (bVerbose) { printf("PopulationFactory is creating PDAltPreyPop\n"); }
			pPop = new PDAltPreyPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_PDPRED:
			if (bVerbose) { printf("PopulationFactory is creating PDPredPop\n"); }
			pPop = new PDPredPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_PDPREY:
			if (bVerbose) { printf("PopulationFactory is creating PDPreyPop\n"); }
			pPop = new PDPreyPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_SIMPLEPRED:
			if (bVerbose) { printf("PopulationFactory is creating SimplePredPop\n"); }
			pPop = new SimplePredPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_SIMPLEPREY:
			if (bVerbose) { printf("PopulationFactory is creating SimplePreyPop\n"); }
			pPop = new SimplePreyPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;
		case CLASS_VEGDWELLER:
			if (bVerbose) { printf("PopulationFactory is creating VegDwellerPop\n"); }
			pPop = new VegDwellerPop(m_pCG,m_pPopFinder,m_iLayerSize,m_apIDG,m_aulState);
			break;

        default:
            printf("unknown  Class ID [%d]\n", iClassID);
        }           
    }
    return pPop;

}

//----------------------------------------------------------------------------
// readPopulation
//
PopBase *PopulationFactory::readPopulation(const char *pConfig) {
    PopBase *pPop = NULL;

    LineReader *pLR = LineReader_std::createInstance(pConfig, "rt");
    if (pLR != NULL) {
        spcid ci = readClass(pLR);
        if (ci != CLASS_NONE) {
            int iResult = 0;
            pPop = createPopulation(ci);
            if (pPop != NULL) {
                iResult = pPop->readSpeciesData(pLR);
                if (iResult != 0) {
                    printf("error while reading config file\n");
                    delete pPop;
                    pPop = NULL;
                }
            } else {
                printf("Not a known class ID [%d]\n", ci);
            }
                
        } else {
            printf("Couldn't extract CLASS from [%s]\n", pConfig);
        }
        delete pLR;
    } else {
        printf("Couldn't open [%s] for reading\n", pConfig);
    }
    return pPop;
}
