
#ifndef __XBVSD_H__
#define __XBVSD_H__

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

const int MAX_PR_PARAMS =  4;


class xbvsd : public Gtk::Dialog {
public:
    xbvsd(xbv *pxbvCaller);
    virtual ~xbvsd();
  
    void set_data();

protected:
   
    virtual void set_action();
    virtual void close_action();
    virtual bool press_action(GdkEventKey *e);

   

    
    Gtk::Label m_lblProjections;  
    Gtk::ComboBox m_cboProjections;
    Gtk::Label m_lblLambda0;  
    Gtk::Entry m_txtLambda0;  
    Gtk::Label m_lblPhi0;  
    Gtk::Entry m_txtPhi0;  

    Gtk::Label m_lblGrid;  
    Gtk::Entry m_txtGridW;  
    Gtk::Label m_lblGrid_x;  
    Gtk::Entry m_txtGridH;  
                           
    Gtk::Label m_lblOffs;  
    Gtk::Entry m_txtOffsX;  
    Gtk::Label m_lblOffs_x;  
    Gtk::Entry m_txtOffsY;  
                           
    Gtk::Label m_lblReal;  
    Gtk::Entry m_txtRealW;  
    Gtk::Label m_lblReal_x;  
    Gtk::Entry m_txtRealH;  
                           
    Gtk::Label m_lblRadius;  
    Gtk::Entry m_txtRadius;  

    Gtk::Label *m_lblParams[MAX_PR_PARAMS];  
    Gtk::Entry *m_txtParams[MAX_PR_PARAMS];  
    Gtk::HBox  *m_hboxParams[MAX_PR_PARAMS];


    Gtk::Button m_butSet;
    Gtk::Button m_butClose;
  
    Gtk::VBox m_VBox0;

    Gtk::HBox m_HBox1;
    Gtk::HBox m_HBox2;
    Gtk::HBox m_HBox3;
    Gtk::HBox m_HBox4;
    Gtk::HBox m_HBox5;
    Gtk::HBox m_HBox6;
    Gtk::HBox m_HBox7;
    Gtk::HBox m_HBox8;


    ProjType m_pt;
    ProjGrid m_pd;

    xbv *m_pxbvCaller;
    



    //Signal handlers:

    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
        { add(m_colProj); add(m_colName); }

        Gtk::TreeModelColumn<int> m_colProj;
        Gtk::TreeModelColumn<Glib::ustring> m_colName;
    };

    ModelColumns m_ProjColumns;

    //Child widgets:
    Glib::RefPtr<Gtk::ListStore> m_refTreeModelProj;

};


#endif
