
#include <string.h>

#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/main.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/separator.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include "utils.h"
#include "xbv.h"

#include "xbvsd.h"

  
const int DEF_XBVD_W = 500;


//@@@@@@@@@ TODO
// initial dialog:
//  name part + Lookup part
//  namepart: txtName + butLoad
//  LookUpPart: dropdown list filled by means of LookUpFactory,
//              variable number of entries from Paramnames...
//  if a name is selected & type is found, choose a default LU
//     BMAP: BVLookUp(0,1) or UCHARLookUp
//     SNAP: PopLookUp or VegLookUp
//  if unable to load: assume generic bin map, show all data

//-----------------------------------------------------------------------------
// constructor
//
xbvsd::xbvsd(xbv *pxbvCaller) 
    : Gtk::Dialog("Data", false, false),
      m_lblProjections("Projection", Gtk::ALIGN_RIGHT),
      m_lblLambda0("Lambda0", Gtk::ALIGN_RIGHT),
      m_lblPhi0("Phi0", Gtk::ALIGN_RIGHT),
      m_lblGrid("Grid" , Gtk::ALIGN_RIGHT),
      m_lblGrid_x(" x "),
      m_lblOffs("Offset + ", Gtk::ALIGN_RIGHT),  
      m_lblOffs_x(" + "),
      m_lblReal("Real",Gtk::ALIGN_RIGHT),  
      m_lblReal_x(" x "),
      m_lblRadius("Radius", Gtk::ALIGN_RIGHT),

      m_butSet("Set"),
      m_butClose("Close"),
      
      m_VBox0(false,5),
      m_HBox1(true,5),
      m_HBox2(true,5),
      m_HBox3(true,5),
      m_HBox4(true,5),
      m_HBox5(true,5),
      m_HBox6(true,5),
      m_HBox7(true,5),
      m_HBox8(true,5),
      
      m_pxbvCaller(pxbvCaller) {

    
    // lookup combobox
    m_refTreeModelProj = Gtk::ListStore::create(m_ProjColumns);
    m_cboProjections.set_model(m_refTreeModelProj);
    int iL = GeoInfo::instance()->getNumProj();
    for (int i = 0; i < iL; i++) {
        Gtk::TreeModel::Row row = *(m_refTreeModelProj->append());
        
        row[m_ProjColumns.m_colProj] = i;
        row[m_ProjColumns.m_colName] = GeoInfo::instance()->getName(i);
    }
    m_cboProjections.pack_start(m_ProjColumns.m_colName);

    GeoInfo::free();

    // signals:
    // send all key_press events to our handler (load_action on Return)
    signal_key_press_event().connect(sigc::mem_fun(*this, &xbvsd::press_action), false);


    // button signals
    m_butSet.signal_clicked().connect(sigc::mem_fun(*this, &xbvsd::set_action) );
    m_butClose.signal_clicked().connect(sigc::mem_fun(*this, &xbvsd::close_action) );



    // structure: 
    // HBox1 file load



    // HBox4: LookUp cbo + set
    m_HBox1.pack_start(m_lblProjections,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox1.pack_start(m_cboProjections,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox2.pack_start(m_lblLambda0, Gtk::PACK_EXPAND_WIDGET);
    m_HBox2.pack_start(m_txtLambda0, Gtk::PACK_EXPAND_WIDGET);
    m_HBox2.pack_start(m_lblPhi0, Gtk::PACK_EXPAND_WIDGET);
    m_HBox2.pack_start(m_txtPhi0, Gtk::PACK_EXPAND_WIDGET);
    m_HBox4.pack_start(m_lblGrid, Gtk::PACK_EXPAND_WIDGET);
    m_HBox4.pack_start(m_txtGridW, Gtk::PACK_EXPAND_WIDGET);
    m_HBox4.pack_start(m_lblGrid_x, Gtk::PACK_EXPAND_WIDGET);
    m_HBox4.pack_start(m_txtGridH, Gtk::PACK_EXPAND_WIDGET);
    m_HBox5.pack_start(m_lblOffs, Gtk::PACK_EXPAND_WIDGET);
    m_HBox5.pack_start(m_txtOffsX, Gtk::PACK_EXPAND_WIDGET);
    m_HBox5.pack_start(m_lblOffs_x, Gtk::PACK_EXPAND_WIDGET);
    m_HBox5.pack_start(m_txtOffsY, Gtk::PACK_EXPAND_WIDGET);
    m_HBox6.pack_start(m_lblReal, Gtk::PACK_EXPAND_WIDGET);
    m_HBox6.pack_start(m_txtRealW, Gtk::PACK_EXPAND_WIDGET);
    m_HBox6.pack_start(m_lblReal_x, Gtk::PACK_EXPAND_WIDGET);
    m_HBox6.pack_start(m_txtRealH, Gtk::PACK_EXPAND_WIDGET);
    m_HBox7.pack_start(m_lblRadius, Gtk::PACK_EXPAND_WIDGET);
    m_HBox7.pack_start(m_txtRadius, Gtk::PACK_EXPAND_WIDGET);
    m_HBox8.pack_start(m_butClose,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox8.pack_start(m_butSet, Gtk::PACK_EXPAND_WIDGET);
   
    // VBox0: entire dialog
    m_VBox0.pack_start(m_HBox1, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox2, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox4, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox5, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox6, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox7, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox8, Gtk::PACK_SHRINK);
    
    
    get_vbox()->add(m_VBox0);


    show_all_children();

    Gtk::Requisition req = get_vbox()->size_request();
    resize(DEF_XBVD_W, req.height);

    // fill widgets
    set_data();


}

//-----------------------------------------------------------------------------
// destructor
//
xbvsd::~xbvsd() {
}


//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvsd::set_data() {
    char sNum[64];
    m_pxbvCaller->getProjInfo(&m_pt, &m_pd);

    m_cboProjections.set_active(m_pt.m_iProjType);
    m_txtLambda0.set_text(niceNum(sNum, RAD2DEG(m_pt.m_dLambda0)));
    m_txtPhi0.set_text(niceNum(sNum, RAD2DEG(m_pt.m_dPhi0)));
    
    m_txtGridW.set_text(niceNum(sNum, m_pd.m_iGridW));
    m_txtGridH.set_text(niceNum(sNum, m_pd.m_iGridH));
    m_txtOffsX.set_text(niceNum(sNum, m_pd.m_dOffsX));
    m_txtOffsY.set_text(niceNum(sNum, m_pd.m_dOffsY));
    m_txtRealW.set_text(niceNum(sNum, m_pd.m_dRealW));
    m_txtRealH.set_text(niceNum(sNum, m_pd.m_dRealH));
    m_txtRadius.set_text(niceNum(sNum, m_pd.m_dRadius));
    
}



//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvsd::set_action() {
    m_pt.m_iProjType = m_cboProjections.get_active_row_number();
    m_pt.m_dLambda0  = DEG2RAD(atof(m_txtLambda0.get_text().c_str()));
    m_pt.m_dPhi0     = DEG2RAD(atof(m_txtPhi0.get_text().c_str()));
    m_pt.m_iNumAdd   = 0;

    m_pd.m_iGridW    = (int)atof(m_txtGridW.get_text().c_str());
    m_pd.m_iGridH    = (int)atof(m_txtGridH.get_text().c_str());
    m_pd.m_dOffsX    = atof(m_txtOffsX.get_text().c_str());
    m_pd.m_dOffsY    = atof(m_txtOffsY.get_text().c_str());
    m_pd.m_dRealW    = atof(m_txtRealW.get_text().c_str());
    m_pd.m_dRealH    = atof(m_txtRealH.get_text().c_str());
    m_pd.m_dRadius   = atof(m_txtRadius.get_text().c_str());
    
    m_pxbvCaller->setProjInfo(&m_pt, &m_pd);
}

//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvsd::close_action() {
    hide();
}

//-----------------------------------------------------------------------------
// press_action
//   on return do load
// 
bool xbvsd::press_action(GdkEventKey *e) {
    //    printf("Keyval:%d\n", e->keyval);
    bool bReturn = false;
    if (e->keyval == GDK_Return) {
        set_action();
        bReturn = true;
    }
    return bReturn;
}

