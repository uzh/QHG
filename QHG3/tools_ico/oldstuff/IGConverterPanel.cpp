#include <gtkmm.h>

#include "utils.h"
#include "strutils.h"
#include "IGConverterPanel.h"
#include "Icosahedron.h"
#include "IcoLoc.h"
#include "RectLoc.h"
#include "IcoScene.h"
#include "IcoConverter.h"
#include "GridProjection.h"


#define TO_NODE  true
#define TO_COORD false

//-----------------------------------------------------------------------------
// constructor
//   
// 
IGConverterPanel::IGConverterPanel(IcoScene *pIcoScene)
    :  m_pIcoScene(pIcoScene),
       m_bDeleteLoc(false),
       m_radCurrent("Current Geometry"),
       m_radLoad("Load Geometry"),
       m_butBrowse("..."),
       m_butLoad("Load"),
       m_butAddNew("Add"),
       m_butDelSelected("Delete"),
       m_butClear("Clear"),
       m_butCoordToNode("Coord\u2192Node"),
       m_butNodeToCoord("Node\u2192Coord"),
       m_lblFileList("Files to convert", Gtk::ALIGN_LEFT),
       m_lblDirectory("Output Directory", Gtk::ALIGN_LEFT),
       m_butBrowseDir("..."),
       m_lblOutputPrefix("Output Prefix", Gtk::ALIGN_LEFT),
       m_HBox1(true,0) {

    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp;
    m_radCurrent.set_group(gDisp);
    m_radLoad.set_group(gDisp);
    m_radCurrent.set_active(true);
    
    m_radCurrent.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGConverterPanel::on_geometry_mode_action));
    m_radLoad.signal_toggled().connect( 
      sigc::mem_fun(*this, &IGConverterPanel::on_geometry_mode_action));
    //m_FL size request
    m_butBrowse.signal_clicked().connect(
      sigc::mem_fun(*this, &IGConverterPanel::on_button_browse_clicked));
    m_butBrowseDir.signal_clicked().connect(
      sigc::mem_fun(*this, &IGConverterPanel::on_button_browse_dir_clicked));
    m_butLoad.signal_clicked().connect(
      sigc::mem_fun(*this, &IGConverterPanel::on_button_load_clicked));
    m_butAddNew.signal_clicked().connect(
      sigc::mem_fun(*this, &IGConverterPanel::on_button_add_clicked));
    m_butDelSelected.signal_clicked().connect(
      sigc::mem_fun(*this, &IGConverterPanel::on_button_del_clicked));
    m_butClear.signal_clicked().connect(
      sigc::mem_fun(*this, &IGConverterPanel::on_button_clear_clicked));
    m_butCoordToNode.signal_clicked().connect(
      sigc::mem_fun(*this, &IGConverterPanel::on_button_coord_to_node_clicked));
    m_butNodeToCoord.signal_clicked().connect(
      sigc::mem_fun(*this, &IGConverterPanel::on_button_node_to_coord_clicked));

    m_FL.set_size_request(300,120);


    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;
    m_HBox1.pack_start(m_butCoordToNode,  Gtk::EXPAND, 0);
    m_HBox1.pack_start(m_butNodeToCoord,  Gtk::EXPAND, 0);

    attach(m_radCurrent,      0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_radLoad,         0, 1/*2*/, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_txtLoadFile,     0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butBrowse,       3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_butLoad,         3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_hSep1,           0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblFileList,     0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_FL,              0, 3, iTop, iTop+6, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    iTop++;
    attach(m_butAddNew,       3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_butDelSelected,  3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_butClear,        3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    iTop++;
    attach(m_hSep2,           0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblDirectory,    0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtDirectory,    1, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butBrowseDir,    3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblOutputPrefix, 0, 1, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtOutputPrefix, 1, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_hSep3,           0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_HBox1,           0, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
 

    m_txtLoadFile.set_sensitive(false);
    m_butBrowse.set_sensitive(false);
    m_butLoad.set_sensitive(false);

    m_txtDirectory.set_text("./");
}

//----------------------------------------------------------------------------
// destructor
// 
IGConverterPanel::~IGConverterPanel() {
}


//----------------------------------------------------------------------------
// chooseFile
// 
std::string IGConverterPanel::chooseFile(tfilterdata &vFilterData, svector &vFiles) {
    std::string sFile = "";

    printf("Starting file chooser...\n");
    
    Gtk::FileChooserDialog dialog("Please choose a file",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    //    dialog.set_transient_for(*(this->get_toplevel()));
    dialog.set_current_folder("./");
    dialog.set_select_multiple(true);
    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    //Add filters, so that only certain file types can be selected:
    
    for (unsigned int i = 0; i < vFilterData.size(); i++) {
        Gtk::FileFilter filter_temp;
        filter_temp.set_name(vFilterData[i].first);
        filter_temp.add_pattern(vFilterData[i].second);
        dialog.add_filter(filter_temp);
    }

    
    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch(result) {
    case(Gtk::RESPONSE_OK): {
        //Notice that this is a std::string, not a Glib::ustring.
        vFiles = dialog.get_filenames();
        if (vFiles.size() > 0) {
            printf("File selected: %s\n", vFiles[0].c_str());
        }
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
    

    return sFile;
}


//----------------------------------------------------------------------------
// on_button_browse_clicked
// 
void IGConverterPanel::on_button_browse_clicked() {
    svector vDummy;
    tfilterdata vFilterData;
    vFilterData.push_back(tstringpair("ico  files", "*.ico"));
    vFilterData.push_back(tstringpair("ltc  files", "*.ltc"));
    vFilterData.push_back(tstringpair("any files", "*"));
    
    chooseFile(vFilterData, vDummy);
    if (vDummy.size() > 0) {
        m_txtLoadFile.set_text(vDummy[0]);
    }
}

//----------------------------------------------------------------------------
// on_button_load_clicked
// 
void IGConverterPanel::on_button_load_clicked() {
    // notify appropriate surface handler to load file
    notifyObservers(NOTIFY_LOAD,  m_txtLoadFile.get_text().c_str());
}

//----------------------------------------------------------------------------
// on_button_add_clicked
// 
void IGConverterPanel::on_button_add_clicked() {
    tfilterdata vFilterData;
    vFilterData.push_back(tstringpair("pop  files", "*.pop"));
    vFilterData.push_back(tstringpair("any files", "*"));
    chooseFile(vFilterData, m_vFiles);
    
    for (unsigned int i =0; i < m_vFiles.size();i++) {
        m_FL.addNew(m_vFiles[i]);
    }
}

//----------------------------------------------------------------------------
// on_button_del_clicked
// 
void IGConverterPanel::on_button_del_clicked() {  
    m_FL.delSelected();
}

//----------------------------------------------------------------------------
// on_button_clear_clicked
// 
void IGConverterPanel::on_button_clear_clicked() {
    m_FL.clear();
}

//----------------------------------------------------------------------------
// buildPrefix
// 
std::string IGConverterPanel::buildPrefix() {
    std::string sPrefix = "";
    char sDir[512];
    char sPref[512];
    *sPref = '\0';
    strcpy(sDir, m_txtDirectory.get_text().c_str());
    char *pDir = trim(sDir);
    if (strlen(pDir) > 0) {
        if (pDir[strlen(pDir)-1] != '/') {
            strcat(pDir, "/");
        }
    } else {
        strcpy(pDir, "./");
    }
    strcat(sPref, pDir);
    strcat(sPref, m_txtOutputPrefix.get_text().c_str());
    sPrefix = sPref;
    return sPrefix;
}

//----------------------------------------------------------------------------
// getIcoLoc
// 
IcoLoc *IGConverterPanel::getIcoLoc() {
    IcoLoc *pIL = NULL;
    pIL = dynamic_cast<IcoLoc *>(m_pIcoScene->getSurface());
    m_bDeleteLoc = false;
    return pIL;
}
//----------------------------------------------------------------------------
// disposeIcoLoc
// 
void IGConverterPanel::disposeIcoLoc(IcoLoc *pIL) {
    if (m_bDeleteLoc) {
        delete pIL;
    }
}

//----------------------------------------------------------------------------
// on_button_coord_to_node_clicked
// 
void IGConverterPanel::on_button_coord_to_node_clicked() {
    doConversion(TO_NODE);
}

//----------------------------------------------------------------------------
// on_button_node_to_coord_clicked
// 
void IGConverterPanel::on_button_node_to_coord_clicked() {
    doConversion(TO_COORD);
}

//----------------------------------------------------------------------------
// on_button_node_to_coord_clicked
// 
void IGConverterPanel::doConversion(bool bToNode) {
    std::string sPrefix = buildPrefix();
    IcoLoc *pIcoloc = getIcoLoc();

    std::string sIcoName = "(unknown)";
    svector vFailed;

    int iNumFiles = m_FL.getNumEntries();
    // loop through the files, convert to node
    printf("Processing %d files\n", iNumFiles);
    if (iNumFiles > 0) {
        IcoConverter *pIC = IcoConverter::createInstance(pIcoloc, sIcoName.c_str(), true);
        std::string sCur  = m_FL.getFirstEntry();
        int iResult = 0;
        while (!sCur.empty()) {
            size_t iPos=sCur.find_last_of('/');
            if (iPos != std::string::npos) {
                iPos++;
            } else {
                iPos = 0;
            }
            std::string sOut = sPrefix + sCur.substr(iPos);

            if (bToNode) {
                iResult = pIC->nodifyFile(sCur.c_str(), sOut.c_str());
            } else {
                iResult = pIC->coordifyFile(sCur.c_str(), sOut.c_str());
            }

            if (iResult == 0) {
                printf("converted [%s] to [%s]\n", sCur.c_str(), sOut.c_str());
            } else {
                printf("failed to convert [%s]\n",  sCur.c_str());
                vFailed.push_back(sCur);
            }
            sCur = m_FL.getNextEntry();
        }
        delete pIC;
    }

    printf("Successfully converted %zd files to %s-format\n", iNumFiles-vFailed.size(), bToNode?"NODE":"COORD");
    if (vFailed.size() > 0) {
        printf("Failed to convert %zd files:\n", vFailed.size());
        for (unsigned int i = 0; i < vFailed.size(); i++) {
            printf("  [%s]\n", vFailed[i].c_str());
        }
    }
}


//----------------------------------------------------------------------------
// on_geometry_mode_action
// 
void IGConverterPanel::on_geometry_mode_action() {
    if (m_radCurrent.get_active()) {
        m_txtLoadFile.set_sensitive(false);
        m_butBrowse.set_sensitive(false);
        m_butLoad.set_sensitive(false);
    } else {
        m_txtLoadFile.set_sensitive(true);
        m_butBrowse.set_sensitive(true);
        m_butLoad.set_sensitive(true);
    }
}


//----------------------------------------------------------------------------
// on_button_browse_dir_clicked
// 
void IGConverterPanel::on_button_browse_dir_clicked() {
  
    Gtk::FileChooserDialog dialog("Please choose a directory",
                                  Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    //    dialog.set_transient_for(*(this->get_toplevel()));
    dialog.set_current_folder("./");

    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    //Add filters, so that only certain file types can be selected:
    
    
    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch(result) {
    case(Gtk::RESPONSE_OK): {
        printf("Open clicked.\n");
            
        //Notice that this is a std::string, not a Glib::ustring.
        m_txtDirectory.set_text(dialog.get_filename());
        printf("Dir selected: %s\n", dialog.get_filename().c_str());
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
}
