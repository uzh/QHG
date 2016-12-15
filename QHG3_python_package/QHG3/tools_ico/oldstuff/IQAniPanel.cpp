#include <gtkmm.h>

#include "strutils.h"
#include "IQAniPanel.h"

#include "IQAnimator.h"
#include "LineReader.h"

#define MODE_MON  true
#define MODE_FILE false

IQAniPanel::IQAniPanel() 
    : m_radMonitor("Monitor directory"),
      m_lblTargetName("Directory"),
      m_butTargetBrowse("..."),
      m_lblFileMask("File mask"),
      m_chkUseQuat("Use Quat"),
      m_chkUseColor("Use Color"),
      m_radFile("Read from file"),
      m_lblInterval("Interval"),
      m_butGo("Go"),
      m_butStop("Stop"),
      m_pIQAni(NULL),
      m_pLR(NULL),
      m_bMode(MODE_MON) {

    // radiobutton grouping
    Gtk::RadioButtonGroup gDisp;
    m_radMonitor.set_group(gDisp);
    m_radFile.set_group(gDisp);
    m_radMonitor.set_active(true);

    // connect signals
    m_butTargetBrowse.signal_clicked().connect(
      sigc::mem_fun(*this, &IQAniPanel::on_button_browse_target_clicked));

    m_butGo.signal_clicked().connect(
      sigc::mem_fun(*this, &IQAniPanel::on_button_go_clicked));
    m_butStop.signal_clicked().connect(
      sigc::mem_fun(*this, &IQAniPanel::on_button_stop_clicked));

    // radio button signals
    m_radMonitor.signal_toggled().connect( 
      sigc::mem_fun(*this, &IQAniPanel::mode_action));
    m_radFile.signal_toggled().connect( 
      sigc::mem_fun(*this, &IQAniPanel::mode_action));

    // layout: push buttons and text entries in table
    int iTop = 0;
    Gtk::AttachOptions iXOpt =  Gtk::FILL | Gtk::SHRINK;
    Gtk::AttachOptions iYOpt =  Gtk::FILL | Gtk::SHRINK;

    int iXPad = 10;
    int iYPad =  3;

    iTop++;
    attach(m_radMonitor,       0, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_radFile,          2, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblTargetName,    1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtTargetName,    2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butTargetBrowse,  3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;                
    attach(m_lblFileMask,      1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtFileMask,      2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;               
    attach(m_chkUseQuat,       1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_chkUseColor,      2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_hSep1,            0, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_lblInterval,      1, 2, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_txtInterval,      2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    iTop++;
    attach(m_butGo,            2, 3, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);
    attach(m_butStop,          3, 4, iTop, iTop+1, iXOpt, iYOpt, iXPad, iYPad);


    setSensitivity(m_bMode);
    m_txtInterval.set_text("1");
}

IQAniPanel::~IQAniPanel() {
}

void IQAniPanel::setSensitivity(bool bMon) {
    m_lblTargetName.set_sensitive(true);
    m_lblTargetName.set_text(bMon?"Directory":"Filename");
    m_txtTargetName.set_sensitive(true);
    m_butTargetBrowse.set_sensitive(true);

    
    m_lblFileMask.set_sensitive(bMon);
    m_txtFileMask.set_sensitive(bMon);
    
    m_chkUseQuat.set_sensitive(!bMon);
    m_chkUseColor.set_sensitive(true);

}

void IQAniPanel::enableAll(bool bEnable) {
    if (bEnable) {
        m_radMonitor.set_sensitive(true);
        m_radFile.set_sensitive(true);
        m_lblInterval.set_sensitive(true);
        m_txtInterval.set_sensitive(true);
        m_butGo.set_sensitive(true);
        m_butStop.set_sensitive(false);
        setSensitivity(m_bMode);

    } else {
        m_butGo.set_sensitive(false);
        m_butStop.set_sensitive(true);
        m_lblInterval.set_sensitive(false);
        m_txtInterval.set_sensitive(false);

        m_radMonitor.set_sensitive(false);
        m_radFile.set_sensitive(false);

        m_lblTargetName.set_sensitive(false);
        m_txtTargetName.set_sensitive(false);
        m_butTargetBrowse.set_sensitive(false);
        m_lblFileMask.set_sensitive(false);
        m_txtFileMask.set_sensitive(false);
        
        m_chkUseQuat.set_sensitive(false);
        m_chkUseColor.set_sensitive(false);
        
        
    }
}

void IQAniPanel::mode_action() {
    m_bMode = m_radMonitor.get_active();
    setSensitivity(m_bMode);
}

//-----------------------------------------------------------------------------
// on_button_go_clicked
// 
void IQAniPanel::on_button_go_clicked() {
    float fInterval;
    if (strToNum(m_txtInterval.get_text().c_str(), &fInterval)) {
        if (m_radFile.get_active()) {
            m_pLR = LineReader_std::createInstance(m_txtTargetName.get_text().c_str(), "rt");
            if (m_pLR != NULL) {
                enableAll(false);
                m_pIQAni->launch(m_pLR, m_chkUseQuat.get_active(), m_chkUseColor.get_active(), fInterval);
            } else {
                // error message for status
                printf("couldnit open [snapori.txt]\n");
            }
        } else {
            // launch monitor
            // check existence of directory?
            enableAll(false);
            m_pIQAni->launch(m_txtTargetName.get_text().c_str(), m_txtFileMask.get_text().c_str(), m_chkUseColor.get_active(), fInterval);
        }
    } else {
        // error message for status
        
    }
}

//-----------------------------------------------------------------------------
// on_button_stop_clicked
// 
void IQAniPanel::on_button_stop_clicked() {
    m_pIQAni->stop();
    enableAll(true);
}

//-----------------------------------------------------------------------------
// chooseDir
// 
void IQAniPanel::on_button_browse_target_clicked() {
    if (m_bMode) {
        on_button_browse_dir_clicked();
    } else {
        on_button_browse_file_clicked();
    }
}

//-----------------------------------------------------------------------------
// chooseDir
// 
void IQAniPanel::on_button_browse_dir_clicked() {

    printf("Starting dir chooser...\n");
    
    Gtk::FileChooserDialog dialog("Please choose a file",
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
        m_txtTargetName.set_text(dialog.get_filename());
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

//-----------------------------------------------------------------------------
// on_button_browse_file_clicked
// 
void IQAniPanel::on_button_browse_file_clicked() {

    printf("Starting file chooser...\n");
    
    Gtk::FileChooserDialog dialog("Please choose a file",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    //    dialog.set_transient_for(*(this->get_toplevel()));
    dialog.set_current_folder("./");

    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    //Add filters, so that only certain file types can be selected:
    
    Gtk::FileFilter filter_qmap;
    filter_qmap.set_name("text files");
    filter_qmap.add_pattern("*.txt");
    dialog.add_filter(filter_qmap);

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
        m_txtTargetName.set_text(dialog.get_filename());
        printf("File selected: %s\n", dialog.get_filename().c_str());
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


