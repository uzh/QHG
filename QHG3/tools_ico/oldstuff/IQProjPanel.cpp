#include <gtkmm.h>
#include <string>

#include "strutils.h"
#include "GeoInfo.h"
#include "GridProjection.h"
#include "IQProjPanel.h"
#include "IQImageViewer.h"
#include "ProjInfo.h"
#include "notification_codes.h"
#include "init.h"

//@@#define DEF_IMG_W 400
//@@#define DEF_IMG_H 200

double realSizes[8][2] = {
    {2*M_PI, M_PI},
    {2, 2}, 
    {2*M_PI, 2*M_PI},
    {M_PI, M_PI},
    {4, 4},
    {2*M_PI, M_PI},
    {2*M_PI, M_PI},
    {2,1}
,
};

//----------------------------------------------------------------------------
// constructor
//
IQProjPanel::IQProjPanel(ProjInfo *pPI, IQImageViewer *pIV) 
    : m_pPI(pPI),
      m_pGP(NULL),
      //@@      m_pImgData(NULL),
      m_pIV(pIV),
      m_lblProjType("Type",  Gtk::ALIGN_LEFT),
      m_lblProjLon("Lon 0",  Gtk::ALIGN_RIGHT),
      m_lblProjLat("Lat 0",  Gtk::ALIGN_RIGHT),
 
      m_lblGridSize("Grid Size",  Gtk::ALIGN_LEFT),
      m_lblTimes1(" x "), 
      m_lblRealSize("Real Size",  Gtk::ALIGN_LEFT),
      m_lblTimes2(" x "), 
      m_lblOffset("Offset",  Gtk::ALIGN_LEFT),
      m_lblPlus(" + "), 
      m_chkInterp("Interpolate"),
      m_chkUseQuat("Use Quat"),
      m_butApply("Apply"),
      m_butSave("Save"),
      //@@      m_imgPreView(),
      //@@      m_iIW(DEF_IMG_W),
      //@@      m_iIH(DEF_IMG_H),
      m_iGW(-1),
      m_iGH(-1),
      m_iPW(-1),
      m_iPH(-1),
      m_bStartSettings(true) {


    m_pPI->addObserver(this);

    m_butApply.signal_clicked().connect(sigc::mem_fun(*this, &IQProjPanel::apply_action) );
    m_butSave.signal_clicked().connect(sigc::mem_fun(*this, &IQProjPanel::save_action) );
    m_lstProjTypes.signal_changed().connect( sigc::mem_fun(*this, &IQProjPanel::ptype_action) );
    m_chkInterp.signal_clicked().connect(sigc::mem_fun(*this, &IQProjPanel::interp_action) );                                               
    m_chkUseQuat.signal_clicked().connect(sigc::mem_fun(*this, &IQProjPanel::usequat_action) );

    m_HBox3.pack_start(m_chkInterp, Gtk::PACK_SHRINK);
    m_HBox3.pack_start(m_chkUseQuat, Gtk::PACK_SHRINK);
    m_HBox3.pack_start(m_butApply, Gtk::PACK_EXPAND_WIDGET);
    m_HBox3.pack_start(m_butSave,  Gtk::PACK_EXPAND_WIDGET);

    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    iTop = 0;
    attach(m_lblProjType,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_lstProjTypes,   1, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
       
    attach(m_lblProjLon,     0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtProjLon,     1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblProjLat,     2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtProjLat,     3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    
    attach(m_hSep2,          0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    
    attach(m_lblGridSize,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtGridW,       1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTimes1,      2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtGridH,       3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_lblRealSize,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtRealW,       1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblTimes2,      2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtRealH,       3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_lblOffset,      0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtOffsX,       1, 2, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_lblPlus,        2, 3, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    attach(m_txtOffsY,       3, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    attach(m_hSep3,          0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_HBox3,          0, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
    iTop++;
    //@@   attach(m_imgPreView,     0, 4, iTop, iTop+1, iXOpt, iYOpt, 0, iYPad);
  
    m_chkInterp.set_active(INIT_INTERP);
    m_chkUseQuat.set_active(INIT_USE_QUAT);
    int iEH;
    int iEW;
    m_txtGridW.get_size_request(iEW, iEH);
    m_txtGridW.set_size_request(60, iEH);
    m_txtGridW.set_alignment(1);
    m_txtGridH.set_size_request(60, iEH);
    m_txtGridH.set_alignment(1);
    m_txtRealW.set_size_request(60, iEH);
    m_txtRealW.set_alignment(1);
    m_txtRealH.set_size_request(60, iEH);
    m_txtRealH.set_alignment(1);
    m_txtOffsX.set_size_request(60, iEH);
    m_txtOffsX.set_alignment(1);
    m_txtOffsY.set_size_request(60, iEH);
    m_txtOffsY.set_alignment(1);
    m_txtProjLon.set_size_request(60, iEH);
    m_txtProjLon.set_alignment(1);
    m_txtProjLat.set_size_request(60, iEH);
    m_txtProjLat.set_alignment(1);

    m_hSep3.get_size_request(iEW, iEH);
    /*
    m_imgPreView.set_size_request(DEF_IMG_W,DEF_IMG_H);
    m_imgPreView.get_size_request(iEW, iEH);
    printf("img size req %d,%d\n", iEW, iEH);
    */
    m_txtProjLon.set_text("0");
    m_txtProjLat.set_text("0");
    m_txtOffsX.set_text("c");
    m_txtOffsY.set_text("c");
    char s[16];
    sprintf(s, "%d", DEF_IMG_W);
    m_txtGridW.set_text(s);
    sprintf(s, "%d", DEF_IMG_H);
    m_txtGridH.set_text(s);


    // fill projection list
    char sTemp[256];
    m_refTreeModelProj = Gtk::ListStore::create(m_ProjColumns);
    m_lstProjTypes.set_model(m_refTreeModelProj);
    for (int i = 0; i < GeoInfo::getNumProj(); i++) {
        Gtk::TreeModel::Row row = *(m_refTreeModelProj->append());
        strcpy(sTemp,  GeoInfo::getName(i));
        char *p = strrchr(sTemp, ' ');
        if (p != 0) {
            *p = '\0';
        }
        row[m_ProjColumns.m_colType] = i;
        row[m_ProjColumns.m_colName] = sTemp;
    }
    m_lstProjTypes.pack_start(m_ProjColumns.m_colName);
    
    m_butApply.set_sensitive(false);
    m_butSave.set_sensitive(false);
    
    m_lstProjTypes.set_active(0);
    ptype_action();
    createGridProjection(true);

}

//----------------------------------------------------------------------------
// destructor
//
IQProjPanel::~IQProjPanel() {
}


//----------------------------------------------------------------------------
// createGridProjection
//
int IQProjPanel::createGridProjection(bool bPreview) {
    int iResult = -1;

    // use data to create ProjType and ProjGrid

    int iProjType = m_lstProjTypes.get_active_row_number();

    double dLambda0;
    double dPhi0;
    double dRW;
    double dRH;
    int iOX=0;
    int iOY=0;
    bool bCenterX=false;
    bool bCenterY=false;

    if (strToNum(m_txtProjLon.get_text().c_str(), &dLambda0)) {
        dLambda0 *= M_PI/180;
        if (strToNum(m_txtProjLat.get_text().c_str(), &dPhi0)) {
            dPhi0 *= M_PI/180;
            // now the grid params
            if (strToNum(m_txtGridW.get_text().c_str(), &m_iGW)) {
                if (strToNum(m_txtGridH.get_text().c_str(), &m_iGH)) {
                    if (strToNum(m_txtRealW.get_text().c_str(), &dRW)) {
                        if (strToNum(m_txtRealH.get_text().c_str(), &dRH)) {
                            if (strToNum(m_txtOffsX.get_text().c_str(), &iOX)) {
                                iResult = 0;
                            } else {
                                if (m_txtOffsX.get_text().c_str()[0]=='c') {
                                    bCenterX = true;
                                    iResult = 0;
                                }
                            }
                            if (iResult == 0) {
                                if (strToNum(m_txtOffsY.get_text().c_str(), &iOY)) {
                                    iResult = 0;
                                } else {
                                    if (m_txtOffsY.get_text().c_str()[0]=='c') {
                                        bCenterY = true;
                                        iResult =0;
                                    } else {
                                        iResult = -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // use different grid size if preview is needed
    ProjGrid *pPG = NULL;
    if (iResult == 0) {
        //        printf("ProjType: %d, (%f,%f)\n", iProjType, dLambda0, dPhi0);
        if (bPreview) {
            int iIW = m_pIV->getWidth();
            int iIH = m_pIV->getHeight();
            double dAG = (1.0*m_iGW)/m_iGH;
            double dAI = (1.0*iIW)/iIH;
            
            if (dAI > dAG) {
                m_iPH = iIH;
                m_iPW = (int) (iIH*dAG);
            } else {
                m_iPW = iIW;
                m_iPH = (int)(iIW/dAG);
            }
            if (bCenterX) {
                iOX = -m_iPW/2;
            }
            if (bCenterY) {
                iOY = -m_iPH/2;
            }
            //            printf("Tsize %dx%d (%f); gsize %dx%d (%f) -> psize %dx%d (%f)\n", iIW, iIH, dAI, m_iGW, m_iGH, dAG, m_iPW, m_iPH, (1.0*m_iPW)/m_iPH);
            
            //            printf("ProjGrid: %dx%d, %fx%f, %d+%d\n",m_iGW, m_iGH, dRW, dRH, iOX, iOY);

            pPG = new ProjGrid(m_iPW, m_iPH, dRW, dRH, iOX, iOY, 1.0);
        } else {
            if (bCenterX) {
                iOX = -m_iGW/2;
            }
            if (bCenterY) {
                iOY = -m_iGH/2;
            }
            //            printf("ProjGrid: %dx%d, %fx%f, %d+%d\n",m_iGW, m_iGH, dRW, dRH, iOX, iOY);
            pPG = new ProjGrid(m_iGW, m_iGH, dRW, dRH, iOX, iOY, 1.0);
       
        }
        ProjType *pPT = new ProjType(iProjType, dLambda0, dPhi0, 0, NULL);
        m_pPI->setGP(pPT, pPG);
        // PI must delete GP
        m_pGP = m_pPI->getGP();
        // PT can be deleted
        delete pPT;
    }
    return iResult;
}

//----------------------------------------------------------------------------
// ptype_action
//  change real-size fields
//
void IQProjPanel::ptype_action() {
    int iProjType = m_lstProjTypes.get_active_row_number();
    char s[16];
    sprintf(s, "%f", realSizes[iProjType][0]);
    m_txtRealW.set_text(s);
    sprintf(s, "%f", realSizes[iProjType][1]);
    m_txtRealH.set_text(s);

    m_butApply.set_sensitive(true);
    m_butSave.set_sensitive(true);

    int iGW;
    if (strToNum(m_txtGridW.get_text().c_str(), &iGW)) {
        int iGH = (int)((iGW* realSizes[iProjType][1])/realSizes[iProjType][0]);
        sprintf(s, "%d", iGH);
        m_txtGridH.set_text(s);
    }

    if (iProjType == PR_EQUIRECTANGULAR) {
        m_txtProjLon.set_text("150");
    } else {
        m_txtProjLon.set_text("0");
    }
}
    
//----------------------------------------------------------------------------
// apply_action
//  - create a gridprojection with the given params for the preview
//  - acquire data for each pixel in the preview
//  - paint the data
//
void IQProjPanel::apply_action() {
    bool bInterp = m_chkInterp.get_active();
    bool bUseQuat = m_chkUseQuat.get_active();
    printf("Interpol check: %s\n", bInterp?"set":"off");
    // calculate a projection
    int iResult = createGridProjection(true);
    if (iResult == 0) {
        m_pIV->createImage(bInterp, bUseQuat, m_iPW, m_iPH);
    }
}

//----------------------------------------------------------------------------
// save_action
//  - create a gridprojection with the given params for the ouput
//  - acquire data for each pixel in the preview
//  - paint the data
//
void IQProjPanel::save_action() {
   
    bool bInterp = m_chkInterp.get_active();
    bool bUseQuat = m_chkUseQuat.get_active();

    
    // open a save-as dialog to get file name?
    std::string sPNGName = chooseFile();
    if (!sPNGName.empty()) {
        // calculate a projection
        int iResult = createGridProjection(false);
        if (iResult == 0) {
            unsigned char *pImage = new unsigned char[m_iGW*m_iGH*3]; 
            if (bInterp) {
                if (bUseQuat) {
                    m_pPI->fillRGBInterpolatedQ(m_iGW, m_iGH, pImage);
                } else {
                    m_pPI->fillRGBInterpolated(m_iGW, m_iGH, pImage);
                }
            } else {
                m_pPI->fillRGBPoints(m_iGW, m_iGH, pImage);
            }
            
            Glib::RefPtr< Gdk::Pixbuf > pixbuf;
            pixbuf = Gdk::Pixbuf::create_from_data(pImage, Gdk::COLORSPACE_RGB, false, 8,  m_iGW, m_iGH, 3*m_iGW);
            
            
            // And finally save the image
            pixbuf->save(sPNGName, "png" );
            
            delete[] pImage;
            
        } else {
            //error
        }
        // create projection for preview again
        createGridProjection(true);
    }
}

//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQProjPanel::notify(Observable *pObs, int iType, const void *pCom) {
    // printf("[IQScene::notify] received notification [%d] from %p\n", iCom , pObs);
    if ((pObs == m_pPI) && (iType == 0)){
        long iCom = (long) pCom;
        switch (iCom) {
        case NOTIFY_ALL_SET: 
            m_butApply.set_sensitive(true);
            m_butSave.set_sensitive(true);
            if (m_bStartSettings) {
                //                 apply_action();
                m_bStartSettings = false;
            }
            break;
        default:
            break;
        }
    }
}

//----------------------------------------------------------------------------
// interp_action
//
void IQProjPanel::interp_action() {
    m_pIV->setInterp(m_chkInterp.get_active());
}


//----------------------------------------------------------------------------
// usequat_action
//
void IQProjPanel::usequat_action() {
    m_pIV->setUseQuat(m_chkUseQuat.get_active());
}



//-----------------------------------------------------------------------------
// chooseFile
// 
std::string IQProjPanel::chooseFile() {
    std::string sFile = "";

    printf("Starting file chooser...\n");
    
    Gtk::FileChooserDialog dialog("Please choose a file",
                                  Gtk::FILE_CHOOSER_ACTION_SAVE);
    //    dialog.set_transient_for(*(this->get_toplevel()));
    dialog.set_current_folder("./");

    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    //Add filters, so that only certain file types can be selected:
    
    Gtk::FileFilter filter_png;
    filter_png.set_name("PNG files");
    filter_png.add_pattern("*.png");
    dialog.add_filter(filter_png);

    
    Gtk::FileFilter filter_any;
    filter_any.set_name("Any files");
    filter_any.add_pattern("*");
    dialog.add_filter(filter_any);
    
    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch(result) {
    case(Gtk::RESPONSE_OK): {
        printf("Open clicked.\n");
            
        //Notice that this is a std::string, not a Glib::ustring.
        sFile = dialog.get_filename();
        printf("File selected: %s\n", sFile.c_str());
        break;
    }
    case(Gtk::RESPONSE_CANCEL): {
        printf("Cancel clicked.\n");
        break;
    }
    default: {
        printf("Unexpected button clicked.\n");
        break;
    }
    }
    
    // remember the directory
    /*
    strcpy(m_sLastDir, sFile.c_str());
    char *p = strrchr(m_sLastDir, '/');
    if (p != NULL) {
        p++;
        *p = '\0';
    }
    */
    return sFile;
}

