#ifndef __XBVOD_H__
#define __XBVOD_H__

#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/arrow.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>

class xbv;


class xbvod : public Gtk::Dialog {
public:
    xbvod(xbv *pxbvCaller);
    virtual ~xbvod();
  
    void set_data();

protected:
   
    virtual void set_action();
    virtual void close_action();
    virtual bool press_action(GdkEventKey *e);

   

    
    Gtk::Label m_lblShapes;  
    Gtk::ComboBox m_cboShapes;
    Gtk::Label m_lblSize;  
    Gtk::Entry m_txtSize;  
    Gtk::Label m_lblValue;  
    Gtk::Entry m_txtValue;  


    Gtk::Button m_butSet;
    Gtk::Button m_butClose;
  
    Gtk::VBox m_VBox0;

    Gtk::HBox m_HBox1;
    Gtk::HBox m_HBox2;
    Gtk::HBox m_HBox3;
    Gtk::HBox m_HBox4;


   
    xbv *m_pxbvCaller;
    



    //Signal handlers:

    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
        { add(m_colType); add(m_colName); }

        Gtk::TreeModelColumn<int> m_colType;
        Gtk::TreeModelColumn<Glib::ustring> m_colName;
    };

    ModelColumns m_ShapeColumns;

    //Child widgets:
    Glib::RefPtr<Gtk::ListStore> m_refTreeModelShapes;

};


#endif
