#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ply.h"

#include "ParamReader.h"
#include "LineReader.h"
#include "SnapHeader.h"
#include "LookUp.h"
#include "NodeLister.h"
#include "Icosahedron.h"
#include "VertexLinkage.h"
#include "PolyFace.h"
#include "LookUpFactory.h"

#define MAX_FILE 256

const char *apFormatNames[] = {
    "",
    "ASCII",
    "BIN_BE",
    "BIN_LE",
};

const int iDefFormat = PLY_BINARY_BE;

#define V_PURE   0
#define V_VAL    1
#define V_COL    2
#define V_VALCOL 3

const char *apPropNames[] = {
    "PURE",
    "VAL",
    "COL",
    "VALCOL",
};

const char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

// structure for vertex data
typedef struct VertexVC {
    double x,y,z;            // the usual 3-space position of a vertex
    double v;            // the usual 3-space position of a vertex
    double r,g,b,a;  // vertex color
} VertexVC; 

// structure for face data
typedef struct Face0 {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
} Face0;

PlyProperty vertVC_props[] = { // list of property information for a vertex
  {"x", PLY_DOUBLE, PLY_DOUBLE, offsetof(VertexVC,x), 0, 0, 0, 0},
  {"y", PLY_DOUBLE, PLY_DOUBLE, offsetof(VertexVC,y), 0, 0, 0, 0},
  {"z", PLY_DOUBLE, PLY_DOUBLE, offsetof(VertexVC,z), 0, 0, 0, 0},
  {"v", PLY_DOUBLE, PLY_DOUBLE, offsetof(VertexVC,v), 0, 0, 0, 0},
  {"r", PLY_DOUBLE, PLY_DOUBLE, offsetof(VertexVC,r), 0, 0, 0, 0},
  {"g", PLY_DOUBLE, PLY_DOUBLE, offsetof(VertexVC,g), 0, 0, 0, 0},
  {"b", PLY_DOUBLE, PLY_DOUBLE, offsetof(VertexVC,b), 0, 0, 0, 0},
  {"a", PLY_DOUBLE, PLY_DOUBLE, offsetof(VertexVC,a), 0, 0, 0, 0},
};

PlyProperty face_props[] = { //list of property information for a face
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face0,verts),
   PLY_LIST, PLY_UCHAR, PLY_UCHAR, offsetof(Face0,nverts)},
};

//----------------------------------------------------------------------------
// 
//
void usage(char *pApp) {
    printf("%s - convert a snapfile to  PLY file\n", pApp);
    printf("Usage:\n");
    printf("  %s -i <icofile> -s <snapfile> -o  <plyfile>\n", pApp);
    printf("     [-f <format>] [-p <props>] [-l <lookupDesc]\n");
    printf("where\n");
    printf("  icofile     icosahedron file for snap\n");
    printf("  snapfile    snap file\n");
    printf("  format      \"ASCII\" | \"BIN_BE\" | \"BIN_LE\n");
    printf("              BIN_BE: big endian\n");
    printf("              BIN_LE: little endian\n");
    printf("              default: %s\n", apFormatNames[iDefFormat]); 
    printf("  props       \"PURE\" | \"VAL\" | \"COL\" | \"VALCOL\"\n");
    printf("              PURE:   vertex coordinates only\n");
    printf("              VAL:    vertex coordinates and value\n");
    printf("              COL:    vertex coordinates and color (RGBA)\n");
    printf("              VALCOL: vertex coordinates, value and color (RGBA)\n");
    printf("  lookupdesc  <lookupname>[\":\"<params>]*\n");
    printf("              lookup name is one o\n");
    for (int i =0; i < LookUpFactory::instance()->getNumLookUps(); i++) {
        printf("                %s\n",  LookUpFactory::instance()->getLookUpName(i));
    }
    printf("  plyfile     name of output file\n"); 
    printf("\n");
}

//----------------------------------------------------------------------------
// getFormat
//
int getFormat(char *pFormat) {
    int iFormat = 0;
    for (int i = 1; (iFormat == 0) && (i < 4); i++) {
        if (strcmp(pFormat, apFormatNames[i]) == 0) {
            iFormat = i;
        }
    }
    return iFormat;
}
//----------------------------------------------------------------------------
// getProp
//
int getProp(char *pProp) {
    int iProp = -1;
    for (int i = 0; (iProp< 0) && (i < 4); i++) {
        if (strcmp(pProp, apPropNames[i]) == 0) {
            iProp = i;
        }
    }
    return iProp;
}

//----------------------------------------------------------------------------
// getLookUp 
//
LookUp *getLookUp(char *pLookUpDesc, double dMin, double dMax) {
    LookUp *pLookUp = NULL;
    char *pParams = strchr(pLookUpDesc, ':');
    if (pParams != NULL) {
        *pParams = '\0';
        pParams++;
    }
    int iType =  LookUpFactory::instance()->getLookUpType(pLookUpDesc);
    if (iType >= 0) {
        // fill param array with defaults
        double adParams[5];
        int iNumParams = LookUpFactory::instance()->getNumParams(iType);
        for (int i = 0; i < iNumParams; i++) {
            adParams[i] = LookUpFactory::instance()->getParamDefault(iType, i);
        }
        
        // put min/max where useful
        switch (iType) {
        case LOOKUP_UCHAR:
            adParams[0] = dMax;
            break;
        case LOOKUP_BINVIEW:
        case LOOKUP_SUN:
        case LOOKUP_RAINBOW:
        case LOOKUP_VEG:
        case LOOKUP_DENSITY:
            adParams[0] = dMin;
            adParams[1] = dMax;
            break;
        case LOOKUP_POP:
        case LOOKUP_BIOME:
            adParams[0] = dMax;
            break;
        case LOOKUP_GEO:
            adParams[0] = dMin;
            adParams[1] = dMax;
            adParams[2] = 0;
            break;
        case LOOKUP_THRESH:
            adParams[0] = (dMax - dMin)/2;
            break;
        case LOOKUP_ZEBRA:
            adParams[0] = 16;
            break;
        case LOOKUP_SEGMENT:
            adParams[0] = dMin;
            adParams[1] = dMax;
            adParams[2] = 100;
            adParams[3] =   8;
        }
        // overwrite with user-defined
        int iResult = 0;
        int iC = 0;
        char *pCtx;
        char *pEnd;
        if (pParams != NULL) {
            char *p = strtok_r(pParams, ":", &pCtx);
            while ((iResult == 0) && (p != NULL) && (iC < iNumParams)) {
                if (*p != '#') {
                    double d = strtod(p, &pEnd);
                    if (*pEnd == '\0') {
                        adParams[iC] = d;
                    } else {
                        iResult = -1;
                    }
                }
                p = strtok_r(NULL, ":", &pCtx);
                iC++;
            }
        }
        if (iResult == 0) {
            pLookUp = LookUpFactory::instance()->getLookUp(iType, adParams, iNumParams);

            if (pLookUp != NULL) {
                iResult = 0;
                printf("Have LookUp %d (%p) with params: ", iType, pLookUp);
                for (int i = 0; i < iNumParams; i++) {
                    printf("%f ", adParams[i]);
                }
                printf("\n");   
            }
        } else {
            printf("Bad param info\n");
        }
    } else {
        printf("No LookUp named [%s]\n", pLookUpDesc);
    }

    return pLookUp;
}

//----------------------------------------------------------------------------
// setPLYHeader
//
int setPLYHeader(PlyFile *ply, int iProp, int iNumVerts, int iNumFaces) {
    int iResult = 0;

    printf("Creating header...\n");

    // only describe properties required by prop 
    // x,y,z   (indexes 0, 1, 2) always
    // v       (index 3) only for V_VAL and V_VALCOL
    // r,g,b,a (indexes 4,5,6,7) only for V_COL and V_VALCOL
    for (unsigned int i = 0; i < sizeof(vertVC_props)/sizeof(PlyProperty); i++) {
        if ((i < 3) || // xyz
            ((i == 3) && ((iProp & V_VAL) != 0)) || // val if what is VAL or VALCOL
            ((i >  3) && (iProp > V_VAL))) {        // rgba if what is COL or VALCOL 
            ply_describe_property (ply, "vertex", &vertVC_props[i]);
        }
    }
   
    if (iResult == 0) {
        // set face properties
        ply_element_count (ply, "face", iNumFaces);
        ply_describe_property (ply, "face", &face_props[0]);
        
        // header finished
        ply_header_complete (ply);
    } 
    return iResult;
}

//----------------------------------------------------------------------------
// setPLYVertices
//
int setPLYVertices(PlyFile *ply, int iProp, nodelist &nlVal, VertexLinkage *pVL, LookUp *pLookUp) {
    int iResult = 0;
    // now write data
    printf("Writing data...\n");
    nodelist::const_iterator it;
    
    ply_put_element_setup (ply, "vertex");
    for (it = nlVal.begin(); it != nlVal.end(); it++) {
        Vec3D *vCur = pVL->getVertex(it->first);
        double dVal = it->second;
      
        double dCol[4];
        if (pLookUp != NULL) {
            pLookUp->getColor(dVal, dCol[0],dCol[1],dCol[2],dCol[3]);
        }
        
        double dProps[sizeof(vertVC_props)/sizeof(PlyProperty)];
        int iC = 0;
        dProps[iC++] = vCur->m_fX;
        dProps[iC++] = vCur->m_fY;
        dProps[iC++] = vCur->m_fZ;
        if ((iProp & V_VAL) != 0) {
            dProps[iC] = dVal;
        }
        iC++;
        if (iProp > V_VAL) {
            dProps[iC++] = dCol[0];
            dProps[iC++] = dCol[1];
            dProps[iC++] = dCol[2];
            dProps[iC++] = dCol[3];
        }
        ply_put_element (ply, (void *) dProps);

     }
    return iResult;
}

//----------------------------------------------------------------------------
// setPLYFaces
//
int setPLYFaces(PlyFile *ply, Icosahedron *pIco, VertexLinkage *pVL) {
    int iResult = 0;

    ply_put_element_setup (ply, "face");
    Face0 f;
    int Ids[3];
    f.nverts=3;
    PolyFace *pF =  pIco->getFirstFace();
    while (pF != NULL) {
        for (int j  =0; j < 3; j++) {
            //            Ids[j] = pVL->getVertexID(pF->getVertex(j));
            Ids[j] = pF->getVertexID(j);
        }
        f.verts=Ids;
        ply_put_element (ply, (void *) &f);
        
        pF = pIco->getNextFace();
    }
    return iResult;
}


//----------------------------------------------------------------------------
// createPLY
//
int createPLY(Icosahedron *pIco, nodelist &nlVal, char *pOutputName, int iMode, int iProp, LookUp *pLookUp) {
    int iResult =0;
    float version;
    VertexLinkage *pVL = pIco->getLinkage();
 
    printf("Creating PLY file...\n");
    // open ply file
    PlyFile *ply = ply_open_for_writing(pOutputName, 2, elem_names, iMode, &version);

    // fill it 
    iResult = setPLYHeader(ply, iProp, pVL->getNumVertices(), pVL->getNumFaces());
    if (iResult == 0) {
        iResult = setPLYVertices(ply, iProp, nlVal, pVL, pLookUp);
        if (iResult == 0) {
            iResult = setPLYFaces(ply, pIco, pVL);
        }
    }

    // close file    
    printf("Closing PLY file ...\n");
    ply_close (ply);

    return iResult;
}


//----------------------------------------------------------------------------
// main
//
int main(int iArgC, char *apArgV[]) {
    int iResult = -1;
    char sIcoFile[MAX_FILE];
    char sSnapFile[MAX_FILE];
    char sOutputFile[MAX_FILE];
    char sFormat[MAX_FILE];
    int iFormat = iDefFormat;
    char sProp[MAX_FILE];
    int iProp = V_PURE;
    char sLookUpDesc[MAX_FILE];
    *sIcoFile = '\0';
    *sSnapFile = '\0';
    *sOutputFile = '\0';
    *sLookUpDesc = '\0';
    *sFormat = '\0';
    *sProp = '\0';

    ParamReader *pPR = new ParamReader();
    bool bOK = pPR->setOptions(6,  
                               "-i:s!",    sIcoFile,
                               "-s:s!",    sSnapFile,
                               "-o:s!",    sOutputFile,
                               "-f:s",     sFormat,
                               "-p:s",     sProp,
                               "-l:s",     sLookUpDesc);
 
    if (bOK) {
        iResult = pPR->getParams(iArgC, apArgV);
        if (iResult >= 0) {
            iResult = 0;
            if (*sFormat != '\0') {
                iFormat = getFormat(sFormat);
                if (iFormat > 0) {
                    iResult = 0;
                    printf("Have format %d [%s]\n", iFormat, apFormatNames[iFormat]);
                } else {
                    printf("Invalid format [%s]\n", sFormat);
                    iResult = -1;
                }
            }

            if (iResult == 0) {
                if (*sProp != '\0') {
                    iProp = getProp(sProp);
                    if (iProp >= V_PURE) {
                        printf("Have prop %d [%s]\n", iProp, apPropNames[iProp]);
                        iResult = 0;
                    } else {
                        printf("Invalid prop [%s]\n", sProp);
                        iResult = -1;
                    }
                }
            }

            if (((iProp == V_COL) || (iProp == V_VALCOL)) && (*sLookUpDesc == '\0')) {
                printf("For outputs COL and VALCOL a color LookUp must be specified\n");
                iResult = -1;
            }

            
            if (iResult == 0) {
                // determine presel
                LineReader *pLR = LineReader_std::createInstance(sSnapFile, "rt");
                if (pLR != NULL) {
                    // try snap
                    SnapHeader *pSH = new SnapHeader();
                    iResult = pSH->read(pLR, BIT_GRID | BIT_TIME);
                    if (iResult == 0) {
                        bool bPreSel = pSH->m_bPreSel;
                        Icosahedron  *pIco = Icosahedron::create(1, POLY_TYPE_ICO);
                        pIco->setStrict(true);
                        pIco->setPreSel(bPreSel);
                        iResult = pIco->load(sIcoFile);
                        
                        if (iResult == 0) {
                            pIco->relink();
                            nodelist nlVal;
                            double   dMax;
                            double   dMin;
                            LookUp *pLookUp = NULL;
                            if (*sLookUpDesc != '\0') {
                                pLookUp = getLookUp(sLookUpDesc, dMin, dMax);
                                if (pLookUp != NULL) {
                                    if ((iProp == V_COL) || (iProp == V_VALCOL)) {
                                        printf("LookUp has no effect for outputs PURE and VAL\n");
                                    }
                                    
                                    iResult = NodeLister::createList(sSnapFile, nlVal, &dMin, &dMax);
                                    if (iResult == 0) {
                                        // if lookups min and max are undefined use the actual ones
                                        if (isinf(pLookUp->m_dMinLevel)) {
                                            pLookUp->m_dMinLevel = dMin;
                                        }
                                        if (isinf(pLookUp->m_dMaxLevel)) {
                                            pLookUp->m_dMaxLevel = dMax;
                                        }
                                        iResult = createPLY(pIco, nlVal, sOutputFile, iFormat, iProp, pLookUp);
                                    } else {
                                        iResult = -1;
                                        printf("Couldn't create LookUp from Description [%s]\n", sLookUpDesc);
                                    }
                                }
                            } else {
                                iResult = -1;
                                printf("Error while reading icosahedron\n");
                            }
                        } else {
                            iResult = -1;
                            printf("Error while reading icosahedron\n");
                        }
                        
                        delete pIco;
                    } else {
                        iResult = -1;
                        printf("Error while reading snap header\n");
                        printf("%s\n", pSH->m_sError);
                    }
                    
                    delete pSH;
                    delete pLR;
                } else {
                    iResult = -1;
                    printf("Couldn't open snap file [%s]\n", sSnapFile);
                }
            }
        
        } else {
            usage(apArgV[0]);
        }

    } else {
        printf("Error in setOptions\n");
    }

    return iResult;
}
