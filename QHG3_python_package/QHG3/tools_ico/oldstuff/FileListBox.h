#ifndef __FILELISTBOX_H__
#define __FILELISTBOX_H__

#include <gtkmm.h>


class FileListBox : public Gtk::ScrolledWindow {
public:
    FileListBox();
    virtual ~FileListBox();
    
    void addNew(Glib::ustring sLine);
    void delSelected();
    void clear();

    int getNumEntries();
    std::string getFirstEntry();
    std::string getNextEntry();

protected:

    unsigned int m_iCur;
    std::vector<std::string> m_vFiles;
    //Signal handlers:

    
    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        
        ModelColumns()
        { add(m_col_name);add(m_col_full);}
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_full;
    };
    
    ModelColumns m_Columns;
    
    Gtk::ScrolledWindow m_ScrolledWindow;
    Gtk::TreeView m_TreeView;
    Glib::RefPtr<Gtk::ListStore> m_refTreeModel;


};


#endif
