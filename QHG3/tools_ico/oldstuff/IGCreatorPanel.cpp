#include <gtkmm.h>

#include "utils.h"
#include "strutils.h"
#include "icoutil.h"
#include "IGCreatorPanel.h"
#include "RegionSplitter.h"
#include "TileDialog.h"

#define PAGE_NONE -1 
#define PAGE_EQ    0
#define PAGE_ICO   1
#define PAGE_FLAT  2

//-----------------------------------------------------------------------------
// constructor
//   
// 
IGCreatorPanel::IGCreatorPanel(IcoScene *pIcoScene, IGTilingPanel *pIGT)
    : m_pIcoScene(pIcoScene),
      m_bFlat(false),
      m_bReady(false),
      m_iStateInfo(0),
      m_iStateIco(STATE_ICO | MODE_ICO_FULL),
      m_iStateFlat(STATE_FLAT | MODE_RECT_HEX),
      m_pIGT(pIGT),
      m_pRS(NULL),
      m_lblIco("Icosahedral", Gtk::ALIGN_LEFT),
      m_tblIco(pIcoScene),
      m_lblFlat("Flat", Gtk::ALIGN_LEFT),
      m_tblFlat(pIcoScene),
      m_lblEQ("EQ", Gtk::ALIGN_LEFT),
      m_tblEQ(pIcoScene),
      m_butBrowse("...") ,
      m_butSave("Save"),
      m_chkSurf("Surf"),
      m_chkIGN("IGN"),
      m_chkQDF("QDF") {

    m_tblIco.addObserver(this);
    m_tblFlat.addObserver(this);
    m_tblEQ.addObserver(this);

 
    // push button signals
    m_butBrowse.signal_clicked().connect(
      sigc::mem_fun(*this, &IGCreatorPanel::on_button_browse_clicked));
    m_butSave.signal_clicked().connect(
      sigc::mem_fun(*this, &IGCreatorPanel::on_button_save_clicked));
    m_nbkSubDialogs.signal_switch_page().connect(
      sigc::mem_fun (this, &IGCreatorPanel::on_page_switch) );
    m_nbkSubDialogs.append_page(m_tblEQ,    m_lblEQ);
    m_nbkSubDialogs.append_page(m_tblIco,   m_lblIco);   
    m_nbkSubDialogs.append_page(m_tblFlat,  m_lblFlat);

    int iEW;
    int iEH;
    m_chkSurf.get_size_request(iEW, iEH);
    m_chkSurf.set_size_request(50, iEH);
    m_chkIGN.set_size_request(50, iEH);
    m_chkQDF.set_size_request(50, iEH);
    m_chkSurf.set_active(false);
    m_chkIGN.set_active(false);
    m_chkQDF.set_active(true);


    
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;

    attach(m_nbkSubDialogs, 0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    
    iTop++;
    iTop++;
    attach(m_txtSaveFile,   0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butBrowse,     3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);

    iTop++;
    attach(m_chkSurf,       0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_chkIGN,        1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_chkQDF,        2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butSave,       3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    m_nbkSubDialogs.set_current_page(PAGE_EQ);

    //   m_bReady=true;
}


//-----------------------------------------------------------------------------
// destructor
//   
// 
IGCreatorPanel::~IGCreatorPanel() {
}

void IGCreatorPanel::setReady(bool bReady) {
    m_bReady = bReady; 
    if (m_bReady) {
        m_tblIco.activateSurface();
    }
}

//-----------------------------------------------------------------------------
// on_button_browse_clicked
//   
// 
void IGCreatorPanel::on_button_browse_clicked() {
    printf("Browse...\n");
}

//-----------------------------------------------------------------------------
// on_button_save_clicked
//   
// 
void IGCreatorPanel::on_button_save_clicked() {
    printf("Save...\n");
    char sFileName[512];
    strcpy(sFileName, m_txtSaveFile.get_text().c_str());
    char *pFileName = trim(sFileName);
    if (*pFileName != '\0') {
        char *p = strrchr(pFileName, '.');
        if (p != NULL) {
            
            if ((strcmp(p+1, "ico") == 0) || (strcmp(p+1, "ign") == 0)) {
                *p = '\0';
            }
        }
        int iCurPage = m_nbkSubDialogs.get_current_page();
        char sFileNameSurface[512];
        char sFileNameIGN[512];
        char sFileNameQDF[512];
        sprintf(sFileNameSurface, "%s.%s", pFileName, (iCurPage==PAGE_FLAT)?"ltc":((iCurPage==PAGE_EQ)?"ieq":"ico"));    
        printf("Ico file [%s]\n", sFileNameSurface);
        sprintf(sFileNameIGN, "%s_%%03d.ign", pFileName);    
        printf("IGN file [%s]\n", sFileNameIGN);
        sprintf(sFileNameQDF, "%s.qdf", pFileName);    
        printf("QDF file [%s]\n", sFileNameQDF);

        TileDialog *pTD = new TileDialog(m_pIGT);
        int iRes = pTD->run();
        printf("res was %d\n", iRes);
        if (iRes == CLICK_SET) {
            m_pRS = m_pIGT->getSplitter();
        } else {
            m_pRS = NULL;
        }

        delete pTD;

        if (m_pRS != NULL) {

            SurfaceHandler *pS = dynamic_cast<SurfaceHandler *>( m_nbkSubDialogs.get_nth_page(iCurPage));
            pS->saveSurface(m_chkSurf.get_active()?sFileNameSurface:NULL, 
                            m_chkIGN.get_active()?sFileNameIGN:NULL, 
                            m_chkQDF.get_active()?sFileNameQDF:NULL, 
                            m_pRS);
        } else {
            printf("No Tiling defined\n");
        }
    } else {
        printf("No file name provided\n");
    }
}

//-----------------------------------------------------------------------------
// on_page_switch
//   
// 
void IGCreatorPanel::on_page_switch(GtkNotebookPage *page, guint page_num ) {
    //    printf("switched to page %d\n", page_num);
    if (page_num == PAGE_FLAT) {
        m_bFlat  = true;
    } else  {
        m_bFlat  = false;
    }
    if (m_bReady) {
        SurfaceHandler *pS = dynamic_cast<SurfaceHandler *>( m_nbkSubDialogs.get_nth_page(page_num));
        pS->activateSurface();
        printf("flat is %d (%p)\n", m_bFlat, &m_bFlat);
        m_pIcoScene->setFlat(m_bFlat);
        m_iStateInfo = 0;
        
        notifyObservers(NOTIFY_SWITCH,  &(m_bFlat?m_iStateFlat:m_iStateIco));

    }
}


//-----------------------------------------------------------------------------
// loadSurface
//   
// 
void IGCreatorPanel::loadSurface(const char *pFileNameSurface) {
    // find file type
    printf("[IGCreatorPanel::loadSurface] have file to load: [%s]\n",pFileNameSurface); 
    int iPage = PAGE_NONE;
    FILE *fIn = fopen(pFileNameSurface, "rb");
    if (fIn != NULL) {
        char s[4];
        int iRead = fread(s, 1, 4, fIn);
        if (iRead == 4) {
            if ((s[3] == '3') || s[3] == '\n') {
                s[3] = '\0';
            }
            if (strcmp(s, "ICO") == 0) {
                iPage = PAGE_ICO;
            } else if (strcmp(s, "LTC") == 0) {
                iPage = PAGE_FLAT;
            }
        } else {
            printf("Couldn't read from [%s]\n", pFileNameSurface);
        }
        
        fclose(fIn);
        printf("[IGCreatorPanel::loadSurface] have type [%s]\n",s); 

        if (iPage != PAGE_NONE) {
            SurfaceHandler *pS = dynamic_cast<SurfaceHandler *>( m_nbkSubDialogs.get_nth_page(iPage));
            int iResult = pS->loadSurface(pFileNameSurface);
            if (iResult == 0) {
                pS->activateSurface();
                m_nbkSubDialogs.set_current_page(iPage);

            }
        } else {
            printf("Unknown file type: [%s]\n", s);
        }
    } else {
        printf("Couldn't open [%s]\n", pFileNameSurface);
    }
}

//-----------------------------------------------------------------------------
// notify
//   from Observer
// 
void IGCreatorPanel::notify(Observable *pObs, int iType, const void *pCom) {

    switch (iType) {
    case NOTIFY_ICO_MODE:
        printf("notif ICO_MODE\n");
        m_iStateIco = (m_iStateIco & !MASK_ICO_MODE) | *(int *)pCom;
        notifyObservers(NOTIFY_SWITCH,  (void *) &m_iStateIco);
        break;
    case NOTIFY_FLAT_PROJ:
        printf("notif FLAT_PROJ\n");
        m_iStateFlat = (m_iStateFlat & !MASK_FLAT_PROJ) |  *(int *)pCom;
        break;
    case NOTIFY_FLAT_LINK: {
        printf("notif FLAT_LINK\n");
        m_iStateFlat = (m_iStateFlat & !MASK_FLAT_LINK) |  *(int *)pCom;
        printf("   state %x\n",  m_iStateFlat);
        double dH = (m_iStateFlat==MODE_RECT_QUAD)?1:sqrt(3)/2;
        notifyObservers(NOTIFY_NEW_H, (void *)(&dH));
        break;
    }
    case NOTIFY_TILE_SPLITTER:
        printf("notif TILE_SPLITTER\n");
        m_pRS = (RegionSplitter *)pCom;
        break;
    case NOTIFY_NEW_GP:
        printf("notif GP SET\n");
        notifyObservers(NOTIFY_NEW_GP, pCom);
        break;
    case NOTIFY_CREATED:
        printf("notif CREATED\n");
        notifyObservers(NOTIFY_CREATED, pCom);
        break;
    case NOTIFY_TILED:
        printf("notif TILED\n");
        notifyObservers(NOTIFY_TILED, pCom);
        break;
    }

}
