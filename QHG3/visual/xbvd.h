#ifndef __XBVD_H__
#define __XBVD_H__

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

class ValReader;
class LookUp;
class xbv;
class LookUpFactory;

const int MAX_LU_PARAMS = 5;


class xbvd : public Gtk::Dialog {
public:
    xbvd(xbv *pxbvCaller);
    virtual ~xbvd();
  
    const char *getFile();

    static char m_sDir[2048];

    void setData();
protected:
    void setDataFromVR();
    void setDataFromSN();
    void setEmptyData();
    void setLookUpData();
    virtual void lu_change_action();
    virtual void set_action();
    virtual bool press_action(GdkEventKey *e);

   
    void show_extents(bool bShow);
   /*   
         Gtk::Label m_lblDummy;
    */

    Gtk::Label m_lblTitleLookUp;  
    Gtk::Label *m_lblParams[MAX_LU_PARAMS];  
    Gtk::Entry *m_txtParams[MAX_LU_PARAMS];  
    Gtk::HBox  *m_hboxParams[MAX_LU_PARAMS];
    Gtk::ComboBox m_cboLU;

    Gtk::Label m_lblSize;  

    Gtk::Button m_butLUSet;
  
    Gtk::VBox m_VBox0;

    Gtk::HBox m_HBox4;


   
    xbv *m_pxbvCaller;
    LookUp     *m_pLU;


    double m_dLUMin;
    double m_dLUMax;
    bool   m_bResetParams;

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

    ModelColumns m_LUColumns;

    //Child widgets:
    Glib::RefPtr<Gtk::ListStore> m_refTreeModelLU;

};


#endif
