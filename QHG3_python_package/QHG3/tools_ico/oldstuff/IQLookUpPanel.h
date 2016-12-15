#ifndef _IQLOOKUPPANEL_H__
#define _IQLOOKUPPANEL_H__


#include <gtkmm.h>

#include "Observer.h"

#define MAX_LU_PARAMS 5

class ProjInfo;

class IQLookUpPanel : public Gtk::Table, public Observer {
public:
    IQLookUpPanel(ProjInfo *pPI);
    virtual ~IQLookUpPanel();
    
    void setRange(double dMin, double dMax);


    ProjInfo *m_pPI;

    // from Observer
    void notify(Observable *pObs, int iType, const void *pCom);
    void enter_pressed();
  
protected:
    virtual void lu_change_action();
    virtual void set_action();
    virtual void reset_action();
    virtual bool press_action(GdkEventKey *e);
    //    virtual void pinval_action();
protected:
    Gtk::Label    m_lblLUType;
    Gtk::ComboBox m_lstLUTypes;
    Gtk::Button   m_butLUSet;
    Gtk::Label    *m_lblLUParams[MAX_LU_PARAMS];  
    Gtk::Entry    *m_txtLUParams[MAX_LU_PARAMS];   
    Gtk::Entry    *m_txtLUParamsReal[MAX_LU_PARAMS];   
    Gtk::Button   m_butRangeReset;
    Gtk::CheckButton m_chkPinValues;
    int m_iLUType;
    double m_adLUParams[MAX_LU_PARAMS];

    double m_dMinVal;
    double m_dMaxVal;
    bool m_bInit;

//Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
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
