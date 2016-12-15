#ifndef _IGCONVERTERPANEL_H__
#define _IGCONVERTERPANEL_H__


#include <string>
#include <vector>
#include <gtkmm.h>

#include "Observable.h"

#include "FileListBox.h"

class IcoLoc;
class IcoScene;

typedef std::pair<std::string, std::string> tstringpair;
typedef std::vector<tstringpair> tfilterdata;

typedef std::vector<std::string> svector;

class IGConverterPanel : public Gtk::Table, public Observable {
public:
    IGConverterPanel(IcoScene *pIcoScene);
    virtual ~IGConverterPanel();

    std::string chooseFile(tfilterdata &vFilterData, svector &vFiles);
protected:
    virtual void on_button_browse_clicked();       
    virtual void on_button_load_clicked();         
    virtual void on_button_add_clicked();          
    virtual void on_button_del_clicked();          
    virtual void on_button_clear_clicked();        
    virtual void on_button_browse_dir_clicked();       
    virtual void on_button_coord_to_node_clicked();
    virtual void on_button_node_to_coord_clicked();
    virtual void on_geometry_mode_action();

    void doConversion(bool bToNode);
    std::string buildPrefix();
    IcoLoc *getIcoLoc();
    void disposeIcoLoc(IcoLoc *pIL);

protected:
    IcoScene *m_pIcoScene;

    int m_iC;
    svector m_vFiles;
    bool m_bDeleteLoc;

    Gtk::RadioButton m_radCurrent;
    Gtk::RadioButton m_radLoad;

    Gtk::Entry m_txtLoadFile;

    Gtk::Button m_butBrowse;
    Gtk::Button m_butLoad;

    FileListBox m_FL;
    Gtk::Button m_butAddNew;
    Gtk::Button m_butDelSelected;
    Gtk::Button m_butClear;
    Gtk::Button m_butCoordToNode;
    Gtk::Button m_butNodeToCoord;
    Gtk::Label  m_lblFileList;
    Gtk::Label  m_lblDirectory;
    Gtk::Entry  m_txtDirectory;
    Gtk::Button m_butBrowseDir;
    Gtk::Label  m_lblOutputPrefix;
    Gtk::Entry  m_txtOutputPrefix;

    Gtk::HSeparator m_hSep1;
    Gtk::HSeparator m_hSep2;
    Gtk::HSeparator m_hSep3;
    Gtk::HBox       m_HBox1;
 
};

#endif
