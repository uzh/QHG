
#ifndef __XBVRD_H__
#define __XBVRD_H__

#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>

class xbv;


class xbvrd : public Gtk::Dialog {
public:
    xbvrd(xbv *pxbvCaller);
    virtual ~xbvrd();
  
    void set_data();

protected:
   
    virtual void save_action();
    virtual void cancel_action();
    virtual bool press_action(GdkEventKey *e);

   
    Gtk::Label m_lblTitleFile;
    Gtk::Label m_lblTitleLon;
    Gtk::Label m_lblTitleLat;  
    Gtk::Label m_lblTitleSize;  
    Gtk::Label m_lblTitleType;  
    Gtk::Label m_invDummy1;  
    Gtk::Label m_invDummy2;  
    Gtk::Label m_invDummy3;  
    Gtk::Label m_invDummy4;  
    Gtk::Label m_invDummy5;  
    Gtk::Label m_lblTitleMin;  
    Gtk::Label m_lblTitleMax;  
    Gtk::Label m_lblTitleDelta;  
    Gtk::Entry m_txtFile;
    Gtk::Entry m_txtLonMin;
    Gtk::Entry m_txtLonMax;
    Gtk::Entry m_txtDeltaLon;
    Gtk::Entry m_txtLatMin;
    Gtk::Entry m_txtLatMax;
    Gtk::Entry m_txtDeltaLat;
    Gtk::ComboBox m_cboType;
    


    Gtk::Button m_butSave;
    Gtk::Button m_butCancel;
    Gtk::VBox m_VBox0;

    Gtk::HBox m_HBoxExp;
    Gtk::EventBox m_EBoxExp;
    Gtk::HBox m_HBox3_1;
    Gtk::HBox m_HBox3_2;
    Gtk::VBox m_VBox3_2_1;
    Gtk::VBox m_VBox3_2_2;
    Gtk::VBox m_VBox3_2_3;

    Gtk::HBox m_HBox1_0;
    Gtk::HBox m_HBox1_1;
    Gtk::HBox m_HBox1;
    Gtk::HBox m_HBox2;
    Gtk::VBox m_VBox3;
    Gtk::HBox m_HBox4;

    xbv *m_pxbvCaller;
    
   //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
        { add(m_colType); add(m_colName); }

        Gtk::TreeModelColumn<int> m_colType;
        Gtk::TreeModelColumn<Glib::ustring> m_colName;
    };

    ModelColumns m_TypeColumns;

    //Child widgets:
    Glib::RefPtr<Gtk::ListStore> m_refTreeModelTypes;



};


#endif
