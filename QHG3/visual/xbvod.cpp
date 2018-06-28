
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

#include "xbv.h"

#include "xbvod.h"

#include "utils.h"
  
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
xbvod::xbvod(xbv *pxbvCaller) 
    : Gtk::Dialog("Data", false, false),
      m_lblShapes("Shapes"),
      m_lblSize("Size"),
      m_lblValue("Value"),
      m_butSet("Set"),
      m_butClose("Close"),
      
      m_VBox0(false,5),
      m_HBox4(true,5),
      
      m_pxbvCaller(pxbvCaller) {

    
    // lookup combobox
    m_refTreeModelShapes = Gtk::ListStore::create(m_ShapeColumns);
    m_cboShapes.set_model(m_refTreeModelShapes);
    int iL = 1;
    for (int i = 0; i < iL; i++) {
        Gtk::TreeModel::Row row = *(m_refTreeModelShapes->append());
        
        row[m_ShapeColumns.m_colType] = i;
        row[m_ShapeColumns.m_colName] = "square";
    }
    m_cboShapes.pack_start(m_ShapeColumns.m_colName);

    // signals:
    // send all key_press events to our handler (load_action on Return)
    signal_key_press_event().connect(sigc::mem_fun(*this, &xbvod::press_action), false);


    // button signals
    m_butSet.signal_clicked().connect(sigc::mem_fun(*this, &xbvod::set_action) );
    m_butClose.signal_clicked().connect(sigc::mem_fun(*this, &xbvod::close_action) );



    // structure: 
    // HBox1 file load



    // HBox4: LookUp cbo + set
    m_HBox1.pack_start(m_lblShapes,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox1.pack_start(m_cboShapes,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox2.pack_start(m_lblSize, Gtk::PACK_EXPAND_WIDGET);
    m_HBox2.pack_start(m_txtSize, Gtk::PACK_EXPAND_WIDGET);
    m_HBox3.pack_start(m_lblValue, Gtk::PACK_EXPAND_WIDGET);
    m_HBox3.pack_start(m_txtValue, Gtk::PACK_EXPAND_WIDGET);
    m_HBox4.pack_start(m_butClose,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox4.pack_start(m_butSet, Gtk::PACK_EXPAND_WIDGET);
   
    // VBox0: entire dialog
    m_VBox0.pack_start(m_HBox1, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox2, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox3, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox4, Gtk::PACK_SHRINK);
    
    
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
xbvod::~xbvod() {
}


//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvod::set_data() {
    char sNum[32];
    int iShape;
    double dSize;
    double dValue;
    m_pxbvCaller->getOverlayData(iShape, dSize, dValue);
    printf("got %d, %f, %f\n", iShape, dSize, dValue);
    m_cboShapes.set_active(iShape);
    m_txtSize.set_text(niceNum(sNum, dSize));
    m_txtValue.set_text(niceNum(sNum, dValue));
    
}



//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvod::set_action() {

    double dSize  = atof(m_txtSize.get_text().c_str());
    double dValue = atof(m_txtValue.get_text().c_str());
    int    iShape = m_cboShapes.get_active_row_number();
    m_pxbvCaller->setOverlayData(iShape, dSize, dValue);
    
}

//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvod::close_action() {
    hide();
}

//-----------------------------------------------------------------------------
// press_action
//   on return do load
// 
bool xbvod::press_action(GdkEventKey *e) {
    //    printf("Keyval:%d\n", e->keyval);
    bool bReturn = false;
    if (e->keyval == GDK_Return) {
        set_action();
        bReturn = true;
    }
    return bReturn;
}

