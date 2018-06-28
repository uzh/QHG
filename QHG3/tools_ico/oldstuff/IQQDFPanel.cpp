#include <gtkmm.h>

#include "IQQDFPanel.h"
#include "QDFValueProvider.h"
#include "SurfaceManager.h"

#include "icoutil.h"
#include "notification_codes.h"


IQQDFPanel::IQQDFPanel(SurfaceManager *pSurfaceManager, const char *pSurfFile, const char *pDataFile, bool bPreSel) 
    : m_pSurfaceManager(pSurfaceManager),
      m_lblIcoFile("QDF Grid File", Gtk::ALIGN_LEFT),
      m_butIcoBrowse("..."),
      m_butIcoLoad("Load"),
      m_butIcoAdd("Add Data"),
      m_lblDataFile("Data Set", Gtk::ALIGN_LEFT),
      m_butDataLoad("Show"),
      m_radPoints("Points"),
      m_radLines("Lines"),
      m_radPlanes("Triangles") {

    strcpy(m_sLastDir, "./");
    m_iOldDisp = MODE_POINTS;
    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp;
    m_radPoints.set_group(gDisp);
    m_radLines.set_group(gDisp);
    m_radPlanes.set_group(gDisp);
    setDisp(MODE_PLANES);
    // push button signals
    m_butIcoBrowse.signal_clicked().connect(
      sigc::mem_fun(*this, &IQQDFPanel::on_button_browse_ico_clicked));
    m_butIcoLoad.signal_clicked().connect(
      sigc::mem_fun(*this, &IQQDFPanel::on_button_load_ico_clicked));
    m_butIcoAdd.signal_clicked().connect(
      sigc::mem_fun(*this, &IQQDFPanel::on_button_add_ico_clicked));
    m_butDataLoad.signal_clicked().connect(
      sigc::mem_fun(*this, &IQQDFPanel::on_button_load_data_clicked));

    // radio button signals
    m_radPoints.signal_toggled().connect( 
      sigc::mem_fun(*this, &IQQDFPanel::disp_action));
    m_radLines.signal_toggled().connect( 
      sigc::mem_fun(*this, &IQQDFPanel::disp_action));
    m_radPlanes.signal_toggled().connect( 
      sigc::mem_fun(*this, &IQQDFPanel::disp_action));


    m_lstDSTypes.signal_key_press_event().connect(
      sigc::mem_fun(*this, &IQQDFPanel::press_action), false);
    m_txtIcoFile.signal_key_press_event().connect(
      sigc::mem_fun(*this, &IQQDFPanel::press_action), false);

    m_lstDSTypes.signal_changed().connect(sigc::mem_fun(*this, &IQQDFPanel::ds_change_action), false);

    
    signal_realize().connect( 
      sigc::mem_fun(*this, &IQQDFPanel::realize_action));

    // layout: radio buttons in a hbox
    m_HBox3.pack_start(m_radPoints, Gtk::PACK_EXPAND_WIDGET);
    m_HBox3.pack_start(m_radLines,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox3.pack_start(m_radPlanes, Gtk::PACK_EXPAND_WIDGET);


    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;

    attach(m_lblIcoFile,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_txtIcoFile,    0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butIcoBrowse,  2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_butIcoAdd,     1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butIcoLoad,    2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    attach(m_hSep1,         0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    
    attach(m_lblDataFile,   0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    //    attach(m_txtDataFile,   0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
        attach(m_lstDSTypes,    0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butDataLoad, 2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;

    attach(m_hSep4,         0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_HBox3,         0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);

    iTop++;


    
    // entry sizes and default values
    int iEH;
    int iEW;
  
    m_txtIcoFile.get_size_request(iEW, iEH);
    m_txtIcoFile.set_size_request(240, iEH);
    m_txtIcoFile.set_alignment(1);
    m_txtIcoFile.set_text(pSurfFile);
    m_lstDSTypes.set_size_request(240, iEH);
    m_lstDSTypes.set_active_text(pDataFile);

    /*
    if (pSurfFile != NULL) {
        loadSurface(pSurfFile, bPreSel);
    }
    */
    printf("Finished IQQDFPanel\n");
}

//-----------------------------------------------------------------------------
// destructor
//
IQQDFPanel::~IQQDFPanel() {

}
bool IQQDFPanel::press_action(GdkEventKey *e) {
    if (e->keyval == GDK_Return) {
        enter_pressed();
    }
    return false;
}

//-----------------------------------------------------------------------------
// enter pressed
// 
void IQQDFPanel::enter_pressed() {
    if (m_txtIcoFile.has_focus() || m_butIcoLoad.has_focus()) {
        on_button_load_ico_clicked();
    }  else if (m_lstDSTypes.has_focus() || m_butDataLoad.has_focus()) {
        on_button_load_data_clicked();
    }
}

//-----------------------------------------------------------------------------
// chooseFile
// 
std::string IQQDFPanel::chooseFile(tfilterdata &vFilterData) {
    std::string sFile = "";

    printf("Starting file chooser...\n");
    
    Gtk::FileChooserDialog dialog("Please choose a file",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    //    dialog.set_transient_for(*(this->get_toplevel()));
    dialog.set_current_folder(m_sLastDir);

    // add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    // add filters, so that only certain file types can be selected:
    for (unsigned int i = 0; i < vFilterData.size(); i++) {
        Gtk::FileFilter filter_temp;
        filter_temp.set_name(vFilterData[i].first);
        size_t pos = 0;
        size_t posPrev = 0;
        while (pos != std::string::npos) {
            pos = vFilterData[i].second.find('|', posPrev);
            filter_temp.add_pattern(vFilterData[i].second.substr(posPrev, pos));
            posPrev=pos+1;
        }
        dialog.add_filter(filter_temp);
    }

    
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
    strcpy(m_sLastDir, sFile.c_str());
    char *p = strrchr(m_sLastDir, '/');
    if (p != NULL) {
        p++;
        *p = '\0';
    }

    return sFile;
}


//-----------------------------------------------------------------------------
// on_button_browse_ico_clicked
// 
void IQQDFPanel::on_button_browse_ico_clicked() {
    tfilterdata vFilterData;
    vFilterData.push_back(tstringpair("qdf  files", "*.qdf"));
    vFilterData.push_back(tstringpair("any files", "*"));
    std::string sFile = chooseFile(vFilterData);
    if (!sFile.empty()) {
        m_txtIcoFile.set_text(sFile);
        m_txtIcoFile.grab_focus();
    }
}

//-----------------------------------------------------------------------------
// on_button_browse_data_clicked
// 
void IQQDFPanel::on_button_browse_data_clicked() {
    tfilterdata vFilterData;
    vFilterData.push_back(tstringpair("qdf files", "*.qdf"));
    vFilterData.push_back(tstringpair("any files", "*"));

    std::string sFile = chooseFile(vFilterData);
    if (!sFile.empty()) {
        m_lstDSTypes.set_active_text(sFile);
        m_lstDSTypes.grab_focus();
    }
}

//-----------------------------------------------------------------------------
// on_button_load_ico_clicked
// 
void IQQDFPanel::on_button_load_ico_clicked() {
    notifyObservers(NOTIFY_LOAD_GRID, m_txtIcoFile.get_text().c_str());
    m_pSurfaceManager->loadSurface(m_txtIcoFile.get_text().c_str(), "", true);
    
    
}

//-----------------------------------------------------------------------------
// on_button_add_ico_clicked
// 
void IQQDFPanel::on_button_add_ico_clicked() {
    notifyObservers(NOTIFY_LOAD_DATA, m_txtIcoFile.get_text().c_str());
    m_pSurfaceManager->addData(m_txtIcoFile.get_text().c_str());
    
    
}

//-----------------------------------------------------------------------------
// on_button_load_data_clicked
// 
void IQQDFPanel::on_button_load_data_clicked() {
    notifyObservers(NOTIFY_LOAD_DATA, m_lstDSTypes.get_active_text().c_str());
    bool bForceCol = true;
    m_pSurfaceManager->loadData(m_lstDSTypes.get_active_text().c_str(), bForceCol);
}

//-----------------------------------------------------------------------------
// ds_action
// 
void IQQDFPanel::ds_change_action() {
    std::string s = m_lstDSTypes.get_active_text();
    printf("list change: %s\n", s.c_str());
}

//-----------------------------------------------------------------------------
// disp_action
// 
void IQQDFPanel::disp_action() {
    int iDisp = m_radPoints.get_active()?0:(m_radLines.get_active()?1:2);
    if (iDisp != m_iOldDisp) {
        m_iOldDisp = iDisp;
        notifyObservers(NOTIFY_DISPLAY_MODE, &iDisp);
    }
}

//-----------------------------------------------------------------------------
// realize_action
// 
void IQQDFPanel::realize_action() {
    printf("on realize\n");
    if (m_pSurfaceManager != NULL) {
        notify(m_pSurfaceManager,  NOTIFY_NEW_GRID, NULL);       
        if (m_pSurfaceManager->getSurface() != NULL) {
            if (*(m_lstDSTypes.get_active_text().c_str()) != '\0') {
                bool bForceCol = true;
                notifyObservers(NOTIFY_NEW_GRIDDATA, &bForceCol);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// setDisp
// 
void IQQDFPanel::setDisp(int iDisp) {
    m_radPoints.set_active(iDisp == MODE_POINTS);
    m_radLines.set_active(iDisp  == MODE_LINES);
    m_radPlanes.set_active(iDisp == MODE_PLANES);
}

//-----------------------------------------------------------------------------
// setDisp
// 
void IQQDFPanel::setDispDirect(int iDisp) {
    setDisp(iDisp);
    //   notifyObservers(NOTIFY_DISPLAY_MODE, &iDisp);
}

//-----------------------------------------------------------------------------
// setDataName
// 
void IQQDFPanel::setDataName(char *pData) {
    m_lstDSTypes.set_active_text(pData);
}

//-----------------------------------------------------------------------------
// setDataName
// 
const char *IQQDFPanel::getDataName() {
    return m_lstDSTypes.get_active_text().c_str();
}

//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQQDFPanel::notify(Observable *pObs, int iType, const void *pCom) {
    //    m_bNotifyInProgress = true;
    if (pObs == m_pSurfaceManager) {
        switch (iType) {
        case NOTIFY_ADD_DATA:
        case NOTIFY_NEW_GRID:
            printf("EEEEEEEEEEEEEEEEEEEEEEvent fill list\n");
            // fill lookup list
            m_lstDSTypes.clear();
            QDFValueProvider *pV = dynamic_cast<QDFValueProvider*>(m_pSurfaceManager->getValueProvider());
            if (pV != NULL) {
                std::map<std::string, qdfdata *> &mDataSets = pV->getDataSets();
                std::map<std::string, qdfdata *>::iterator it;
                for (it = mDataSets.begin(); it != mDataSets.end(); ++it) {
                    m_lstDSTypes.append(it->first.c_str());
                    
                }
            }
        }
    }
    //    m_bNotifyInProgress = false;
}
