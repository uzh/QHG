
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
#include "QMapHeader.h"
#include "xbv.h"

#include "xbvrd.h"

  
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
xbvrd::xbvrd(xbv *pxbvCaller) 
    : Gtk::Dialog("Data", false, false),
      m_lblTitleFile("Output File", Gtk::ALIGN_RIGHT),
      m_lblTitleLon("Longitude range", Gtk::ALIGN_LEFT),
      m_lblTitleLat("Latitude range", Gtk::ALIGN_LEFT),
      m_lblTitleType("Data Type", Gtk::ALIGN_RIGHT),
      m_invDummy1(),
      m_invDummy2(),
      m_invDummy3("          "),
      m_invDummy4(),
      m_lblTitleMin("Min", Gtk::ALIGN_RIGHT),
      m_lblTitleMax("Max", Gtk::ALIGN_RIGHT),
      m_lblTitleDelta("Delta", Gtk::ALIGN_RIGHT),
      m_butSave("Save"),
      m_butCancel("Cancel"),

      m_VBox0(false,5),
      m_HBoxExp(false,5),
      m_HBox3_1(true,5),
      m_HBox3_2(true,1),
      m_VBox3_2_1(true,1),
      m_VBox3_2_2(true,1),
      m_VBox3_2_3(true,1),
      m_HBox1_0(false,5),
      m_HBox1_1(true,5),
      m_HBox1(true,5),
      m_HBox2(true,5),
      m_VBox3(false,5),
      m_HBox4(true,5),

      m_pxbvCaller(pxbvCaller) {
    
  

    // signals:
    // send all key_press events to our handler (load_action on Return)
    //    signal_key_press_event().connect(sigc::mem_fun(*this, &xbvrd::press_action), false);


    // button signals
    m_butSave.signal_clicked().connect(sigc::mem_fun(*this, &xbvrd::save_action) );
    m_butCancel.signal_clicked().connect(sigc::mem_fun(*this, &xbvrd::cancel_action) );

    // file type combobox
    m_refTreeModelTypes = Gtk::ListStore::create(m_TypeColumns);
    m_cboType.set_model(m_refTreeModelTypes);
    int iT = QMapHeader::getNumTypes();
    for (int i = 0; i < iT; i++) {
        Gtk::TreeModel::Row row = *(m_refTreeModelTypes->append());
        
        row[m_TypeColumns.m_colType] = i;
        row[m_TypeColumns.m_colName] = QMapHeader::getTypeName(i);
    }
    m_cboType.pack_start(m_TypeColumns.m_colName);


    // structure: 
    // HBox1 file load
   // structure: 
    // HBox1 file load
    m_HBox1_0.pack_start(m_lblTitleFile,   Gtk::PACK_SHRINK);
    m_HBox1_0.pack_start(m_txtFile,        Gtk::PACK_EXPAND_WIDGET);

    m_HBox1_1.pack_start(m_invDummy3,      Gtk::PACK_EXPAND_WIDGET);
    m_HBox1_1.pack_start(m_butCancel,      Gtk::PACK_EXPAND_WIDGET);
    m_HBox1_1.pack_start(m_butSave,        Gtk::PACK_EXPAND_WIDGET);


    //        m_HBox2.pack_start(m_butFlip,      Gtk::PACK_EXPAND_WIDGET);

    // HBox3_1 type combo, flip
    m_HBox3_1.pack_start(m_invDummy1, Gtk::PACK_EXPAND_WIDGET);
    m_HBox3_1.pack_start(m_lblTitleType,   Gtk::PACK_EXPAND_WIDGET);
    m_HBox3_1.pack_start(m_cboType,        Gtk::PACK_EXPAND_WIDGET);

    // VBox3_2_1: bmap data labels
    m_VBox3_2_1.pack_start(m_invDummy2,     Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_1.pack_start(m_lblTitleMin,   Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_1.pack_start(m_lblTitleMax,   Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_1.pack_start(m_lblTitleDelta, Gtk::PACK_EXPAND_WIDGET);

    // VBox3_2_2: bmap data lon
    m_VBox3_2_2.pack_start(m_lblTitleLon, Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_2.pack_start(m_txtLonMin,   Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_2.pack_start(m_txtLonMax,   Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_2.pack_start(m_txtDeltaLon, Gtk::PACK_EXPAND_WIDGET);
    
    // VBox3_2_3: bmap data lat
    m_VBox3_2_3.pack_start(m_lblTitleLat, Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_3.pack_start(m_txtLatMin,   Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_3.pack_start(m_txtLatMax,   Gtk::PACK_EXPAND_WIDGET);
    m_VBox3_2_3.pack_start(m_txtDeltaLat, Gtk::PACK_EXPAND_WIDGET);

    // HBox3: all bmap data together
    m_HBox3_2.pack_start(m_VBox3_2_1,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox3_2.pack_start(m_VBox3_2_2,  Gtk::PACK_EXPAND_WIDGET);
    m_HBox3_2.pack_start(m_VBox3_2_3,  Gtk::PACK_EXPAND_WIDGET);

    m_VBox3.pack_start(m_HBox3_1, Gtk::PACK_EXPAND_WIDGET);
    m_VBox3.pack_start(m_HBox3_2, Gtk::PACK_EXPAND_WIDGET);

   
    // VBox0: entire dialog
    m_VBox0.pack_start(m_HBox1_0, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox1_1, Gtk::PACK_SHRINK);
    //  m_VBox0.pack_start(m_HBox1b, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_HBox2, Gtk::PACK_SHRINK);
    m_VBox0.pack_start(m_VBox3, Gtk::PACK_SHRINK);
    
   

    
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
xbvrd::~xbvrd() {
}


//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvrd::set_data() {
    char sFileName[128];
    char sNum[64];
    int iType;
    double dLonMin;
    double dLonMax;
    double dDeltaLon;
    double dLatMin;
    double dLatMax;
    double dDeltaLat;
    m_pxbvCaller->getConvInfo(sFileName, iType,
                              dLonMin, dLonMax, dDeltaLon,
                              dLatMin, dLatMax, dDeltaLat);

    m_cboType.set_active(QMAP_TYPE_DOUBLE);
    m_txtLonMin.set_text(niceNum(sNum, dLonMin));
    m_txtLonMax.set_text(niceNum(sNum, dLonMax));
    m_txtDeltaLon.set_text(niceNum(sNum, dDeltaLon));
    m_txtLatMin.set_text(niceNum(sNum, dLatMin));
    m_txtLatMax.set_text(niceNum(sNum, dLatMax));
    m_txtDeltaLat.set_text(niceNum(sNum, dDeltaLat));

    m_txtFile.set_text(sFileName);
    
   
}



//-----------------------------------------------------------------------------
// save_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvrd::save_action() {
    int iType= m_cboType.get_active_row_number();
    double dLonMin   = atof(m_txtLonMin.get_text().c_str());
    double dLonMax   = atof(m_txtLonMax.get_text().c_str());
    double dDeltaLon = atof(m_txtDeltaLon.get_text().c_str());
    double dLatMin   = atof(m_txtLatMin.get_text().c_str());
    double dLatMax   = atof(m_txtLatMax.get_text().c_str());
    double dDeltaLat = atof(m_txtDeltaLat.get_text().c_str());

    
    m_pxbvCaller->setConvInfo(m_txtFile.get_text().c_str(), iType,
                              dLonMin, dLonMax, dDeltaLon,
                              dLatMin, dLatMax, dDeltaLat);
    response(1);
}

//-----------------------------------------------------------------------------
// set_action
//   callback for button event ("Set")
//   TODO: set new LookUp Params
void xbvrd::cancel_action() {
    response(0);
}

//-----------------------------------------------------------------------------
// press_action
//   on return do load
// 
bool xbvrd::press_action(GdkEventKey *e) {
    //    printf("Keyval:%d\n", e->keyval);
    bool bReturn = false;
    if (e->keyval == GDK_Return) {
        //        set_action();
        bReturn = true;
    }
    return bReturn;
}

