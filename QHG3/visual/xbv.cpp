#include <string.h>
#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/main.h>
#include <gtkmm/scrollbar.h>
#include <gdk/gdk.h>

#include <cmath>

#include "xbv.h"
#include "xbvd.h"
#include "xbvod.h"
#include "xbvsd.h"
#include "xbvrd.h"

#include "types.h"
#include "utils.h"
#include "LineReader.h"
#include "QMapUtils.h"
#include "QMapUtils_T.cpp"
#include "ValReader.h"
#include "QMapReader.h"
#include "QMapReader.cpp"
#include "LookUpFactory.h"
#include "LookUp.h"
#include "BVLookUp.h"
#include "UCharLookUp.h"
#include "PopLookUp.h"
#include "VegLookUp.h"
#include "SunLookUp.h"
#include "RainbowLookUp.h"
#include "ZebraLookUp.h"
#include "SegmentLookUp.h"

#include "PNGImage.h"


const char *DEF_MAP_UCHAR  = "/home/jody/progs/neander/data/qhg_biomes.map";
const char *DEF_MAP_DOUBLE = "/home/jody/progs/neander/import/massTree.dat";

const int DATA_BMAP = 1;
const int DATA_SNAP = 2;
const int DATA_PNG  = 3;

const double DEF_GRID_REAL_W = 4;
const double DEF_GRID_REAL_H = 4;
const double DEF_REAL_RADIUS = 1;
const int    DEF_PROJ_TYPE = PR_LAMBERT_AZIMUTHAL_EQUAL_AREA;
const double DEF_PROJ_C_LON = 17.0;//15
const double DEF_PROJ_C_LAT = 23.0;//44
const double DEF_OFFSET_X    = -1.0/2.0;
const double DEF_OFFSET_Y    = -1.0/2.0;



const double Y_SPACING = sqrt(3)/2;

//@@@@@@@@@@@ TODO
// function loadData(pFile): checks header: if BMAP, SNAP or PNG : create reader, else return NULL
// function loadBinData(pFile, lonMin,LonMax,... etc) called if all else f
//


//-----------------------------------------------------------------------------
// constructor
//
xbv::xbv() 
    : m_imgMain(),
      m_lblCoords("aaaa", Gtk::ALIGN_LEFT), 
      m_lblLon("bbbbb", Gtk::ALIGN_LEFT),
      m_lblLat("ccccc", Gtk::ALIGN_LEFT),
      m_lblValName(" ", Gtk::ALIGN_RIGHT),
      m_lblValue("ddddd", Gtk::ALIGN_LEFT),
      m_lblZoom("Z[z]", Gtk::ALIGN_LEFT),
      m_HBox0(false,0),
      m_HBox1(true,5),
      m_HBox2(true,1),
      m_HBox3(true,1),
      m_VBox(false,1),
      m_adjh(50, 0, 100), 
      m_adjv(50, 0, 100), 
      m_scrW(m_adjh, m_adjv),
      m_ali(Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP, 0, 0),    

      m_pVR(NULL), 
      m_pLU(NULL),
      m_bFlipped(false),
      m_iZoom(1),
      m_dZoom(1),
      m_bMagnify(true),
      m_bDying(false),
      m_bGeoView(false),
      m_bCross(false),
      m_dCurSpacing(Y_SPACING),
      m_pImgData(NULL),
      m_xbvlDialog(this),
      m_bOverlay(false),
      m_dOAlpha(0.7),
      m_dOValue(10),
      m_dOSize(1),
      m_aadOverlay(NULL),
      m_xbvoDialog(this),
      m_ptLocal(DEF_PROJ_TYPE, DEG2RAD(DEF_PROJ_C_LON), DEG2RAD(DEF_PROJ_C_LAT), 0, NULL),
      m_pgLocalY(900, 900, 4, 4, -450, -450, 1),
      m_xbvsDialog(this),
      m_bShowVectors(false),
      m_fVecScale(1),
      m_bFreeze(false) {

    // set window title
    set_title("XBV");
    m_xbvlDialog.hide();
    m_xbvoDialog.hide();
    m_luvCur.iType = LOOKUP_UNDEF;
    //    m_luvCur.pvParamVals = new std::vector<double>();


    m_ali.add(m_imgMain);
    m_scrW.add(m_ali);
    Widget *pw = m_scrW.get_child();
    pw->add_events(Gdk::POINTER_MOTION_MASK);
    pw->signal_motion_notify_event().connect(sigc::mem_fun(*this, &xbv::doMotion));
    pw->signal_button_press_event().connect(sigc::mem_fun(*this, &xbv::doClick));
    pw->signal_button_release_event().connect(sigc::mem_fun(*this, &xbv::doRelease));


    // set real coords of center
    getCenter();

    *m_sPrevFile = '\0';
   
    // connect event handlers
    //  m_scrW.add_events(Gdk::POINTER_MOTION_MASK);
    //    m_scrW.signal_motion_notify_event().connect(sigc::mem_fun(*this, &xbv::motion_action));
    signal_key_press_event().connect(sigc::mem_fun(*this, &xbv::press_action));
    signal_expose_event().connect(sigc::mem_fun(*this, &xbv::expose_action));

    // set label properties
    m_lblCoords.set_justify(Gtk::JUSTIFY_LEFT);
    m_lblCoords.modify_bg(Gtk::STATE_NORMAL,Gdk::Color("035050"));
    m_lblLat.set_justify(Gtk::JUSTIFY_LEFT);
    m_lblLon.set_justify(Gtk::JUSTIFY_LEFT);
    m_lblValue.set_justify(Gtk::JUSTIFY_RIGHT);
    m_lblZoom.set_justify(Gtk::JUSTIFY_LEFT);
    // box hierarchy for image & labels
    m_HBox0.pack_start(m_scrW/*m_imgMain*/, Gtk::PACK_EXPAND_WIDGET);
    m_VBox.pack_start(m_HBox0, Gtk::PACK_EXPAND_WIDGET);
    m_VBox.pack_start(m_HBox2, Gtk::PACK_SHRINK);
    m_HBox1.pack_start(m_lblCoords, Gtk::PACK_EXPAND_WIDGET);
    m_HBox1.pack_start(m_lblLon, Gtk::PACK_EXPAND_WIDGET);
    m_HBox1.pack_start(m_lblLat, Gtk::PACK_EXPAND_WIDGET); 
    m_HBox1.pack_start(m_lblZoom, Gtk::PACK_EXPAND_WIDGET); 
    //    m_HBox1.pack_start(m_lblValue, Gtk::PACK_EXPAND_WIDGET);
    m_HBox3.pack_start(m_lblValName, Gtk::PACK_EXPAND_WIDGET);
    m_HBox3.pack_start(m_lblValue, Gtk::PACK_EXPAND_WIDGET);

    m_HBox2.pack_start(m_HBox1, Gtk::PACK_EXPAND_WIDGET);
    m_HBox2.pack_start(m_HBox3, Gtk::PACK_EXPAND_WIDGET);
  
    
    resize(745, 400);
   
    
  

    // show the stuff
    add(m_VBox);
    show_all_children();
    //    m_imgMain.show();
}

//-----------------------------------------------------------------------------
// destructor
//
xbv::~xbv() {
    if (m_pLU != NULL) {
        printf("destr deleting LU %p\n", m_pLU);
        delete m_pLU;
    }
    if (m_pVR != NULL) {
        delete m_pVR;
    }
    if (m_pImgData != NULL) {
        delete[] m_pImgData;
    }

    LookUpFactory::instance()->free();

    destroyOverlay();
}
//-----------------------------------------------------------------------------
// getRange
//
int xbv::getRange(double &dMin, double &dMax) {
    dMin = dNaN;
    dMax = dNaN;
    int iResult = 0;
    if (m_pVR != NULL) {
        dMin = m_pVR->getMin();
        dMax = m_pVR->getMax();
    } else {
        iResult = -1;
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// press_action
//   callback for key press event
//      'c': center cross hair
//      'd': open lookup dialog
//      'h': help
//      'j': open projection dialog
//      'o': toggle overlay
//      'p': save as png
//      'q': quit
//      'esc': close all dialogs
//      'r': open Snap 2 qmap dialog (using current projection setting (-> 'j'))
//      'w': save as raw data
//      'y': toggle y-spacing (snap)
//  --- vectors, overlays ---
//      'a': draw arcs
//      's': save overlay
//      'v': show vectors
//  --- lookup shortcuts ---
//      'R': set rain lookup
//      'S': set segment lookup
//      'U': set uchar lookup
//      'T': set thresh lookup
//      'Z': set zebra lookup
//  --- view ---
//      '1'-'9' : zoomfactor
//      '+': increase zoom factor by 1 
//      '-': increase zoom factor by 1 
//      'f': flip image
//      'm': set window to 800x600
//      'M': set window to fit all
//      'z': (debug) diagonal scroll
//

bool xbv::press_action(GdkEventKey *e) {


    printf("Keyval:%d\n", e->keyval);
    switch (e->keyval) {
    case 'm':
        set_size_request(800, 600);
        resize(800,600);
        break;
    case 'M': {
        Glib::RefPtr<Gdk::Screen> scr = Gdk::Screen::get_default();
        set_size_request(200, 200);
        int w = scr->get_width()-40;
        int h = scr->get_height()-80;

        int iWI = w;
        int iHI = h;
        
        if (m_pVR != NULL) {
            iWI = m_pVR->getNRLon();
            iHI = m_pVR->getNRLat();
        }
        if (w > iWI+20) {
            w = iWI+20;
        }
        if (h > iHI+20) {
            h = iHI+20;
        }
        resize(w, h);
        int x = (scr->get_width()-w)/2;
        int y = 0;
        move(x,y);
        break;
    }
    case 't':
        m_bFreeze = !m_bFreeze;
        break;
    case GDK_Escape:
        printf("escapeowitsch\n");
        if (m_xbvlDialog.is_visible()) {
            m_xbvlDialog.hide();
        }
        if (m_xbvoDialog.is_visible()) {
            m_xbvoDialog.hide();
        }
        if (m_xbvsDialog.is_visible()) {
            m_xbvsDialog.hide();
        }
        break;

    case GDK_Left:
        printf("Leftleftleftleft\n");
        break;
    case GDK_Right:
        printf("Rightrightrightright\n");
        break;
    case GDK_Up:
        printf("Upupupupupupup\n");
        break;
    case GDK_Down:
        printf("Downdowndowndowndownndowndown\n");
        break;
    case GDK_Home:
        printf("Homehomehomehomehomehome\n");
        break;
    case GDK_End:
        printf("Endendendendend\n");
        break;
    case 'c':
        // display a crosshair centered on the center of the window
        m_bCross = true;
        getCenter();
        createImage();
        m_bCross = false;
        break;

    case 'd': {
        // 'd' -- open load/param dialog
        if (!m_xbvlDialog.is_visible()) {
            m_xbvlDialog.setData();
            m_xbvlDialog.show_now();
            
        }
        //   	xbvd xbvDialog(this);
        //        int iResp = xbvDialog.run(); 
	//printf("Got response: %d\n", iResp);
        // the getFile() call seems to do something illegal according to valgrind
        //	printf("File: %s\n",  xbvDialog.getFile());
        break;
    }
    case 'p':
        // 'p' -- create a PNG from the QMAP or SNAP using current lookup
        createPNG();
        break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        // '1' - '9' -- set zoom
        getCenter();
        m_iZoom = e->keyval-'0';
        m_dZoom = m_iZoom;
        m_bMagnify = true;
        showZoom();
        createImage();
        setCenter();
        break;

    case '+':
        // '+' -- zoom in (to center of view)
        getCenter();
        if (m_bMagnify) {
            m_iZoom++;
        } else {
            m_iZoom--;
            if (m_iZoom == 0) {
                m_iZoom = 1;
                m_bMagnify = true;
            }
        }
        m_dZoom = (m_bMagnify)?m_iZoom:1.0/m_iZoom;
        showZoom();
        createImage();
        setCenter();
        break;

    case '-':
        // '-' -- zoom out (from center of view)
        getCenter();
        if (m_bMagnify) {
            m_iZoom--;
            if (m_iZoom == 0) {
                m_iZoom = 1;
                m_bMagnify = false;
            }
        } else {
            m_iZoom++;
        }
        m_dZoom = (m_bMagnify)?m_iZoom:1.0/m_iZoom;
        showZoom();
        createImage();
        setCenter();
        break;

    case 'q':
        // 'q' -- quit
        printf("Bye\n"); fflush(stdout);
        m_bDying=true;
        Gtk::Main::quit();
        // printf("Bye\n");
        break;

    case 'h':
    case '?':
        // 'h' -- help dialog
        helpDialog();
        break;

    case 'f':
        // 'f' -- flip image
        m_bFlipped = !m_bFlipped;
        createImage();
        break;
    case 'z': {
        // 'z' 
        double dho = m_scrW.get_hscrollbar()->get_value();
        double dvo = m_scrW.get_vscrollbar()->get_value();
        printf("cur %f,%f v:%dx%d, scr: %d, %d\n", dho, dvo, m_scrW.get_width(), m_scrW.get_height(), m_scrW.get_hscrollbar()->get_height(), m_scrW.get_vscrollbar()->get_width() );
        m_scrW.get_hscrollbar()->set_value(dho+10);
        m_scrW.get_vscrollbar()->set_value(dvo+10);
        break;
    }
    case 'R':
        // 'R' -- set Rainbow lookup
        setLU(LOOKUP_RAINBOW);
        break;

    case 'S':
        // 'S' -- set Segment lookup
        setLU(LOOKUP_SEGMENT);
        break;

    case 'U':
        // 'U' -- set UChar lookup
        setLU(LOOKUP_UCHAR);
        break;

    case 'T':
        // 'T' -- set Threshhold lookup
        setLU(LOOKUP_THRESH);
        break;

    case 'Z':
        // 'Z' -- set Threshhold lookup
        setLU(LOOKUP_ZEBRA);
        break;

    case 'o':
        // 'o' -- toggle overlay
        if (m_pVR != NULL) {
            if (m_bOverlay) {
                m_xbvoDialog.hide();
                destroyOverlay();
                m_bOverlay = false;
            } else { 
                m_xbvoDialog.set_data();
                m_xbvoDialog.show_now();
                
                createOverlay();
                m_bOverlay = true;
            }
        }
        break;

    case 's':
        // 's' -- save overlay
        if (m_bOverlay) {
            saveOverlay();
        }
        break;

    case 'v':
        if (m_bShowVectors) {
            drawVectors();
        }            
        m_bShowVectors = !m_bShowVectors;
        
        break;
    case 'a':
        if (m_bShowVectors) {
            drawArcs();
        }            
        m_bShowVectors = !m_bShowVectors;
        
        break;

    default: 
        printf("Unknown command:%d\n", e->keyval);
      
    }

    return false;
}

void xbv::helpDialog() {
    Gtk::MessageDialog dialog(*this, "Key commands", false, Gtk::MESSAGE_INFO);
    dialog.set_secondary_text("ESC\t\tclose filename/param dialog\n"
                              "'q'\t\tquit application\n"
                              "'+'\t\tzoom in\n"
                              "'-'\t\tzoom out\n"
                              "'f'\t\tflip image\n"
                              "'p'\t\tcreate PNG file\n"
                              "'R'\t\tset rainbow lookup\n"
                              "'S'\t\tsetsegment lookup\n"
                              "'U'\t\tset uchar lookup\n"
                              "'T'\t\tset thresh lookup\n"
                              "'Z'\t\tset zebra lookup\n"
                              "'d'\t\topen filename/param dialog\n"
                              "'j'\t\topen projection dialog\n"
                              "'t'\t\ttoggle freeze\n"
                              "'h','?'\thelp");

  dialog.run();
}

//-----------------------------------------------------------------------------
// showZoom
//   call back for mouse motion
//   get coordinates, calc & show world coords, show value at pointer
//
void xbv::showZoom() {
    char sZ[32];
    sprintf(sZ, "Z[%c%3d]", m_bMagnify?'+':'-', m_iZoom);
    m_lblZoom.set_text(sZ);
}

//-----------------------------------------------------------------------------
// getCenter
//
void xbv::getCenter() {
    m_CX = (m_scrW.get_hscrollbar()->get_value() + (1.0*m_scrW.get_width()-m_scrW.get_vscrollbar()->get_width()-6)/2)/m_dZoom;
    m_CY = (m_scrW.get_vscrollbar()->get_value() + (1.0*m_scrW.get_height()-m_scrW.get_hscrollbar()->get_height()-4)/2)/m_dZoom;
}

//-----------------------------------------------------------------------------
// setCenter
//
void xbv::setCenter() {
    
    double dW = 1.0*m_scrW.get_width()-m_scrW.get_vscrollbar()->get_width()-6;
    double dH = 1.0*m_scrW.get_height()-m_scrW.get_hscrollbar()->get_height()-4;

    // set the new scroll ranges 
    int iWidth  = 0;
    int iHeight = 0;
    if (m_pVR != NULL) {
        iWidth  = m_pVR->getNRLon();
        iHeight = m_pVR->getNRLat();
    }

    m_scrW.get_hadjustment()->set_lower(0);
    m_scrW.get_hadjustment()->set_upper(m_dZoom*iWidth);
    m_scrW.get_vadjustment()->set_lower(0);
    m_scrW.get_vadjustment()->set_upper(m_dZoom*iHeight);
    
    double iOX = m_CX*m_dZoom - dW/2;
    double iOY = m_CY*m_dZoom - dH/2;
    if (iOX >  m_dZoom*iWidth - dW) {
        iOX = m_dZoom*iWidth - dW;
    }
    if (iOY >  m_dZoom*iHeight - dH) {
        iOX = m_dZoom*iHeight - dH;
    }

    m_scrW.get_hscrollbar()->set_value(iOX);
    m_scrW.get_vscrollbar()->set_value(iOY);
}

//-----------------------------------------------------------------------------
// doClick
//
bool xbv::doClick(GdkEventButton *e) {
    uint iX = (uint)(e->x/m_dZoom);
    uint iY = (uint)(e->y/m_dZoom);
    printf("Clicked at %d, %d (type:%d)\n", iX, iY, e->type);

    if (m_bOverlay) {
       bool bPaint = false;
        iX = (uint)((e->x+m_scrW.get_hscrollbar()->get_value())/m_dZoom);
        iY = (uint)((e->y+m_scrW.get_vscrollbar()->get_value())/m_dZoom);

        double dVal;
        if ((e->state & GDK_BUTTON1_MASK) != 0) {
            dVal = m_dOValue;
            bPaint = true;
        } else if ((e->state & GDK_BUTTON2_MASK) != 0) {
            dVal = dNaN;
            bPaint = true;
        }
        

        if (bPaint) {
            printf("rect (%f,%f,%f,%f)\n", 
                   (iX-m_dOSize/2),
                   (iY-m_dOSize/2),
                   iX+m_dOSize/2,iY+m_dOSize/2);

            for (uint i = (int)(iY-m_dOSize/2); i <= iY+m_dOSize/2; i++) {
                if ((i >= 0) && (i < m_iOH)) {
                    for (uint j = (int)(iX-m_dOSize/2); j <= iX+m_dOSize/2; j++) {
                        if ((j >= 0) && (j < m_iOW)) {
                            setOverlayVal(j, i, dVal);
                        }
                    }
                }
            }

           
        }
        createImage();
        
    }

    return true;
}

//-----------------------------------------------------------------------------
// doRelease
//
bool xbv::doRelease(GdkEventButton *e) {
    printf("Release!\n");
    if (m_bOverlay) {
        createImage();
    }
    return true;
}

//-----------------------------------------------------------------------------
// setOverlay
//  Ass: m_pVR != NULL, m_aadOverlay != NULL
//
void xbv::setOverlayVal(uint iX, uint iY, double dValue) {
    if ((iX >= 0) && (iX < m_pVR->getNRLon()) &&
        (iY >= 0) && (iY < m_pVR->getNRLat())) {
        printf("Overlay action %f\n", dValue);

        m_aadOverlay[iY][iX] = dValue;
    }
}

//-----------------------------------------------------------------------------
// destroyOverlay
//  Ass: m_pVR != NULL, m_aadOverlay != NULL
//
void xbv::destroyOverlay() {
    if (m_aadOverlay != NULL) {
        QMapUtils::deleteArray(m_iOW, m_iOH, m_aadOverlay);
        m_aadOverlay = NULL;
    }
}
//-----------------------------------------------------------------------------
// createOverlay
//  Ass: m_pVR != NULL, m_aadOverlay != NULL
//
void xbv::createOverlay() {
    m_iOW = m_pVR->getNRLon();
    m_iOH = m_pVR->getNRLat();
    m_aadOverlay = QMapUtils::createArray(m_iOW, m_iOH, dNaN);
}
//-----------------------------------------------------------------------------
// saveOverlay
//  Ass: m_pVR != NULL, m_aadOverlay != NULL
//
void xbv::saveOverlay() {
    QMapHeader *pQMH = new QMapHeader();
    int iResult = pQMH->readHeader(m_sPrevFile);
    if (iResult == 0) {
        pQMH->m_iType = QMAP_TYPE_DOUBLE;
        FILE *fOut = fopen("overlay.qmap", "wb");
        if (fOut != NULL) {
            bool bOK = pQMH->addHeader(fOut);
            if (bOK) {
                bOK = QMapUtils::writeArray(fOut, m_iOW, m_iOH, m_aadOverlay);
                if (bOK) {
                    fclose(fOut);
                }
            }
        }
    }
    delete pQMH;
}


//-----------------------------------------------------------------------------
// setOverlayData
//  Ass: m_pVR != NULL, m_aadOverlay != NULL
//
void xbv::setOverlayData(int iShape, double dSize, double dValue) {
    m_dOValue = dValue;
    m_dOSize  = dSize;
}


//-----------------------------------------------------------------------------
// getOverlayData
//  Ass: m_pVR != NULL, m_aadOverlay != NULL
//
void xbv::getOverlayData(int &iShape, double &dSize, double &dValue) {
    dValue = m_dOValue;
    dSize = m_dOSize;
    iShape = 0;
}


//-----------------------------------------------------------------------------
// doMove
//
bool xbv::doMotion(GdkEventMotion *e) {
    uint iX = (uint)(e->x/m_dZoom);
    uint iY = (uint)(e->y/m_dZoom);

    showValue(iX, iY);
    
    if (m_bOverlay) {
        bool bPaint = false;
        iX = (uint)((e->x+m_scrW.get_hscrollbar()->get_value())/m_dZoom);
        iY = (uint)((e->y+m_scrW.get_vscrollbar()->get_value())/m_dZoom);

        double dVal;
        if ((e->state & GDK_BUTTON1_MASK) != 0) {
            dVal = m_dOValue;
            bPaint = true;
        } else if ((e->state & GDK_BUTTON2_MASK) != 0) {
            dVal = dNaN;
            bPaint = true;
        }
        

        if (bPaint) {
            printf("rect (%f,%f,%f,%f)\n", 
                   (iX-m_dOSize/2),
                   (iY-m_dOSize/2),
                   iX+m_dOSize/2,iY+m_dOSize/2);
            double dSize = m_dOSize*m_dZoom;
            for (uint i = (int)(iY-dSize/2); i <= iY+dSize/2; i++) {
                if ((i >= 0) && (i < m_iOH)) {
                    for (uint j = (int)(iX-dSize/2); j <= iX+dSize/2; j++) {
                        if ((j >= 0) && (j < m_iOW)) {
                            setOverlayVal(j, i, dVal);
                        }
                    }
                }
            }

            Glib::RefPtr<Gdk::Window> window = m_imgMain.get_window();
            if(window) {
                Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
                
                cr->save();
                double ux=m_dZoom, uy=m_dZoom;
                cr->device_to_user_distance (ux, uy);
                if (ux < uy)
                    ux = uy;
                cr->set_line_width (ux);

                cr->set_source_rgba(0.7, 0.2, 0.8, 1.0);
                //                cr->set_line_width(1);
                cr->rectangle(e->x+m_scrW.get_hscrollbar()->get_value(), 
                              e->y+m_scrW.get_vscrollbar()->get_value(), m_dOSize/2, m_dOSize/2); // full circle

                cr->stroke();
                cr->fill_preserve();
                
                cr->restore();  // back to opaque black 
            }
        }
            
    }

    return true;
}


//-----------------------------------------------------------------------------
// showValue
//   call back for mouse motion
//   get coordinates, calc & show world coords, show value at pointer
//
bool xbv::showValue(uint iX, uint iY) {
    double dLon = dNaN;
    double dLat = dNaN;
    double dVal = dNaN;

    uint iW = 0;
    uint iH = 0;
    // calculate longitude & Latitude
    if (m_pVR != NULL) {
        dLon = m_pVR->getLonMin()+iX*m_pVR->getDLon();
        
        if (m_bFlipped) {
            dLat = m_pVR->getLatMax()-iY*m_pVR->getDLat();
        } else {
            dLat = m_pVR->getLatMin()+iY*m_pVR->getDLat();
        }
        iW = m_pVR->getNRLon();
        iH = m_pVR->getNRLat();
        int iYReal = m_bFlipped?m_pVR->getNRLat()-iY:iY;
        dVal = m_pVR->getDValue(iX, iYReal);
    }

    char sC[32];
    char sLat[32];
    char sLon[32];
    char sVN[32];
    char sV0[32];
    
    if (m_pVR != NULL) {
        sprintf(sLon, "%s: ---", m_pVR->getXName());
        sprintf(sLat, "%s: ---", m_pVR->getYName());
    } else {
        strcpy(sLon, "Lon: ---");
        strcpy(sLat, "Lat: ---");
    }

    //    printf("coords: %f %f -> %f,%f -> %f\n", iX, iY, dLon, dLat, dVal);
    sprintf(sC, "[%4d,%4d]", iX, iY);
    if ((iX >= 0) && (iX < iW) &&
        (iY >= 0) && (iY < iH)) {
        
        if (m_bGeoView) {
            if (dLon > 180.0) {
                dLon = dLon - 360;
            } else if (dLon < -180) {
                dLon = dLon + 360;
            }
        }
        if (m_pVR != NULL) {
            sprintf(sLon, "%s: %7.3f", m_pVR->getXName(), dLon);
            sprintf(sLat, "%s: %7.3f", m_pVR->getYName(), dLat);
        }
        niceNum(sV0, dVal); //sprintf(sV, "%f", dVal);
     
        if (m_pVR != NULL) {
            sprintf(sVN, "%s: ", m_pVR->getVName());
        } else {
            strcpy(sVN, "Val: ");
        }
    } else {
        if (m_pVR != NULL) {
            sprintf(sVN, "%s: ", m_pVR->getVName());
            strcpy(sV0, "---");
        } else {
            strcpy(sVN, "Val: ");
            strcpy(sV0, "---");
        }
    }

    if (!m_bFreeze) {
        m_lblCoords.set_text(sC);
        m_lblLon.set_text(sLon);
        m_lblLat.set_text(sLat);
        m_lblValName.set_text(sVN);
        m_lblValue.set_text(sV0);
        showZoom();
    }
    return true;
}


//-----------------------------------------------------------------------------
// setLookUp
//   set LookUp object
//
int xbv::setLookUp(LookUp *pLU) {
    if (m_pLU != NULL) {
        delete m_pLU;
    }
    m_pLU = pLU;
    //redraw
    return 0;
}

//-----------------------------------------------------------------------------
// setLookUp
//   set LookUp object
//
LookUp *xbv::setLookUp(int iType, double *pdParams) {
    if (m_pLU != NULL) {
        printf("setlookup(type params) deleting LU %p\n", m_pLU);
        delete m_pLU;
    }
    
    
    int iNumParams =  LookUpFactory::instance()->getNumParams(iType);
    
    m_pLU = LookUpFactory::instance()->getLookUp(iType, pdParams, iNumParams);

    m_luvCur.iType = iType;
    m_luvCur.iNumParams = iNumParams;
    m_luvCur.vParamVals.clear();
    for (int i = 0; i < iNumParams; i++) {
        m_luvCur.vParamVals.push_back(pdParams[i]);
    }
    printf("setlookup:creating image with LU type %d, %f - %f\n", iType, pdParams[0], pdParams[1]); 

    createImage();

 
    return m_pLU;
};

//-----------------------------------------------------------------------------
// createImage
//   creates image array from current Reader & LookUp
//
void xbv::createImage() {
    guint8 *pImgData=NULL;
    int iW = 0;
    int iH = 0;

    if (m_pLU != NULL) {
        if (m_pImgData != NULL) {
            delete[] m_pImgData;
        }
        if (m_pVR != NULL) {
            if (m_bOverlay) {
                pImgData = createImageFromQMAPOVR();
            } else {
                pImgData = createImageFromQMAP();
            }
            iW = m_pVR->getNRLon();
            iH = m_pVR->getNRLat();
        }
            
        if (pImgData != NULL) {
            m_pImgData = pImgData;
            if (m_bFlipped) {
                // flip image
                int iW3 = 3*iW;
                guint8 *pRow = new guint8[iW3];
                for (int u = 0; u < iH/2; u++) {
                    memcpy(pRow, pImgData+u*iW3, iW3*sizeof(guint8));
                    memcpy(pImgData+u*iW3, pImgData+(iH-u-1)*iW3, iW3*sizeof(guint8));
                    memcpy(pImgData+(iH-u-1)*iW3, pRow, iW3*sizeof(guint8));
                }
                delete[] pRow;
            }
            printf("creating pixbuf\n");fflush(stdout);
            Glib::RefPtr<Gdk::Pixbuf> pP;
            Glib::RefPtr<Gdk::Pixbuf> pP1 = Gdk::Pixbuf::create_from_data(pImgData, 
                                                                         Gdk::COLORSPACE_RGB,
                                                                         false, 
                                                                         8, 
                                                                         iW, 
                                                                         iH,
                                                                         iW*3 );
            if (m_iZoom != 1) {
                pP = pP1->scale_simple((int)(m_dZoom*iW), (int)(m_dZoom*iH), Gdk::INTERP_NEAREST);
            } else {
                pP = pP1;
            }

            printf("setting pixbuf\n");fflush(stdout);
            m_imgMain.set(pP);
            //            delete[] pImgData;

         
        }
    }
}


//-----------------------------------------------------------------------------
// createImageFromQMAP
//   creates image array from current ValReader & LookUp
//
guint8 *xbv::createImageFromQMAP() {
    guint8 *pData = new guint8[m_pVR->getNRLon()*m_pVR->getNRLat()*3];
    guint8 *p = pData;
    printf("creating %dx%d image with LU %p (p:%p)\n", m_pVR->getNRLon(),  m_pVR->getNRLat(), m_pLU, p);
   
    for (uint i = 0; i < m_pVR->getNRLat(); i++) {
          
        for (uint j = 0; j < m_pVR->getNRLon(); j++) {
  
            double dValue = m_pVR->getDValue(j, i);

            uchar uRed;
            uchar uGreen;
            uchar uBlue;
            uchar uAlpha;
            m_pLU->getColor(dValue, uRed, uGreen, uBlue, uAlpha);

            *p++ = (guint8) uRed;
            *p++ = (guint8) uGreen;
            *p++ = (guint8) uBlue;
        }
    }

    if (m_bCross) {
        p = pData;
        printf("draw cross at (%f,%f) in %dx%d, p = %p\n", m_CX, m_CY, m_pVR->getNRLon(), m_pVR->getNRLat(), p);
        for (uint i = 0; i < m_pVR->getNRLat(); i++) {
            p = (guint8*)(pData+(3*(i*m_pVR->getNRLon()+(int)m_CX)));
            *p++ = (guint8) 0xff;
            *p++ = (guint8) 0;
            *p++ = (guint8) 0;
        }
        p = (guint8*)(pData+(3*m_pVR->getNRLon()*(int)m_CY));
        for (uint i = 0; i < m_pVR->getNRLon(); i++) {
            *p++ = (guint8) 0xff;
            *p++ = (guint8) 0;
            *p++ = (guint8) 0;
        }        
    }
    printf("image done\n");fflush(stdout);

    return pData;
}

//-----------------------------------------------------------------------------
// createImageFromQMAPOVR
//   creates image array from current ValReader & LookUp
//
guint8 *xbv::createImageFromQMAPOVR() {
    guint8 *pData = new guint8[m_pVR->getNRLon()*m_pVR->getNRLat()*3];
    guint8 *p = pData;
    printf("creating %dx%d image plus overlay with LU %p (p:%p)\n", m_pVR->getNRLon(),  m_pVR->getNRLat(), m_pLU, p);
   
    for (uint i = 0; i < m_pVR->getNRLat(); i++) {
          
        for (uint j = 0; j < m_pVR->getNRLon(); j++) {
  
            uchar uRed;
            uchar uGreen;
            uchar uBlue;
            uchar uAlpha;

            double dValue = m_pVR->getDValue(j, i);
            double dValueO = m_aadOverlay[i][j];
            if (!std::isnan(dValueO)) {
                
                if (!std::isnan(dValue)) {
                    dValue = dValueO*m_dOAlpha + dValue*(1-m_dOAlpha);;
                } else {
                    dValue = dValueO;
                }
            }
            m_pLU->getColor(dValue, uRed, uGreen, uBlue, uAlpha);

            *p++ = (guint8) uRed;
            *p++ = (guint8) uGreen;
            *p++ = (guint8) uBlue;
        }
    }

    if (m_bCross) {
        p = pData;
        printf("draw cross at (%f,%f) in %dx%d, p = %p\n", m_CX, m_CY, m_pVR->getNRLon(), m_pVR->getNRLat(), p);
        for (uint i = 0; i < m_pVR->getNRLat(); i++) {
            p = (guint8*)(pData+(3*(i*m_pVR->getNRLon()+(int)m_CX)));
            *p++ = (guint8) 0xff;
            *p++ = (guint8) 0;
            *p++ = (guint8) 0;
        }
        p = (guint8*)(pData+(3*m_pVR->getNRLon()*(int)m_CY));
        for (uint i = 0; i < m_pVR->getNRLon(); i++) {
            *p++ = (guint8) 0xff;
            *p++ = (guint8) 0;
            *p++ = (guint8) 0;
        }        
    }
    printf("image done\n");fflush(stdout);

    return pData;
}


//-----------------------------------------------------------------------------
// createPNG
//   creates PNG file from current ValReader or Snap using current LookUp
//
bool xbv::createPNG(char *pName) {
    bool bOK = false;
    char *pOutput = pName;
    if (pName == NULL) {
        // create output file name
        char sOutput[128];
        if (m_pVR != NULL) {
            strcpy(sOutput, m_pVR->getFileName());
        } else {
            strcpy(sOutput, "unknown");
        }
        // change suffix
        pOutput = strrchr(sOutput, '.');
        if (pOutput != NULL) {
            *pOutput='\0';
        }
        strcat(sOutput, ".png");

        // remove path
        pOutput = strrchr(sOutput, '/');
        if (pOutput != NULL) {
            pOutput++;
        } else {
            pOutput = sOutput;
        }
    }

    if (m_pVR != NULL) {
       bOK = createPNGFromQMAP(pOutput);
    }
    return bOK;
}
//-----------------------------------------------------------------------------
// createRAW
//   creates RAW file from current ValReader or Snap using current LookUp
//
bool xbv::createRAW(char *pName) {
    bool bOK = false;
    char *pOutput = pName;
    if (pName == NULL) {
        // create output file name
        char sOutput[128];
        if (m_pVR != NULL) {
            strcpy(sOutput, m_pVR->getFileName());
        } else {
            strcpy(sOutput, "unknown");
        }
        // change suffix
        pOutput = strrchr(sOutput, '.');
        if (pOutput != NULL) {
            *pOutput='\0';
        }
        strcat(sOutput, ".png");

        // remove path
        pOutput = strrchr(sOutput, '/');
        if (pOutput != NULL) {
            pOutput++;
        } else {
            pOutput = sOutput;
        }
    }
    if (m_pVR != NULL) {
        bOK = false;
    }
    return bOK;
}

//-----------------------------------------------------------------------------
// createPNGFromQMAP
//   creates PNG file from current ValReader & LookUp
//
bool xbv::createPNGFromQMAP(char *pName) {
    bool bOK = false;

    // prepare array
    uchar ** ppuData = new uchar*[m_pVR->getNRLat()];
    for (uint i = 0; i < m_pVR->getNRLat(); i++) {
        ppuData[i] = new uchar[4*m_pVR->getNRLon()];
        bzero(ppuData[i], 4*m_pVR->getNRLon()*sizeof(uchar));
    }

    // fill it
    printf("creating %dx%d PNG with LU %p\n", m_pVR->getNRLon(),  m_pVR->getNRLat(), m_pLU);
    for (uint i = 0; i < m_pVR->getNRLat(); i++) {
        double dLat = m_pVR->Y2Lat(i);
        uchar *pData = ppuData[i];

        for (uint j = 0; j < m_pVR->getNRLon(); j++) {
            double dLon = m_pVR->X2Lon(j);
            double dValue = m_pVR->getDValue(dLon, dLat);
            uchar uRed;
            uchar uGreen;
            uchar uBlue;
            uchar uAlpha;
            m_pLU->getColor(dValue, uRed, uGreen, uBlue, uAlpha);
            *pData++ = uRed;
            *pData++ = uGreen;
            *pData++ = uBlue;
            *pData++ = uAlpha;
        }
    }
    
    // create image from data
    PNGImage *pPI = new PNGImage(m_pVR->getNRLon(),  m_pVR->getNRLat());
    bOK = pPI->createPNGFromData(ppuData, pName);
    delete pPI;
    
    // clean up data
    for (uint i = 0; i < m_pVR->getNRLat(); i++) {
        delete[] ppuData[i];
    }
    delete[] ppuData;

    printf("PNG [%s] done:%s\n", pName, bOK?"OK":"ERR");fflush(stdout);

    return bOK;
}


//-----------------------------------------------------------------------------
// createImageFromPNG
//   creates image array from current ValReader 
//
guint8 *xbv::createImageFromPNG() {
    return NULL;
}


//-----------------------------------------------------------------------------
// setValReader
//   set Reader & LookUp, create image and show it
//
int xbv::setValReader(ValReader *pVR, LookUp *pLU/*=NULL*/) {
    if (m_pVR != NULL) {
        delete m_pVR;
    }
    m_pVR = pVR;
    
    if (m_pLU != NULL)  {
        if (m_pLU != pLU) {
            delete m_pLU;
        }
    }
    m_pLU = pLU;

        
    set_title(pVR->getFileName());
    // determine data type
    createImage();
  
    return 0;
    
}


//-----------------------------------------------------------------------------
// setImage
//   show specified image
//
int xbv::setImage(const char *pFileName) {
    m_imgMain.set(pFileName);
    return 0;
}


//-----------------------------------------------------------------------------
// overwriteParams
//   overwrite values in array pParams (iMax elements) with 
//   converted corresponding values in string
//   string format [<double>[:<double>]*]
//
int overwriteParams(char *pParams, double *adParams, int iMax) {
    int iResult = 0;
    int iC = 0;
    char *pCtx;
    char *pEnd;
    char *p = strtok_r(pParams, ":", &pCtx);
    while ((iResult == 0) && (p != NULL) && (iC < iMax)) {
        if (*p != '#') {
            double d = strtod(p, &pEnd);
            if (*pEnd == '\0') {
                adParams[iC] = d;
            } else {
                iResult = -1;
            }
        }
        p = strtok_r(NULL, ":", &pCtx);
        iC++;
    }

    return iResult;
}

//-----------------------------------------------------------------------------
// setLookUp
//   set LookUp by name with default values
//
int xbv::setLookUp(const char *pLookUpName, const char *pParams) {
    printf("LU:[%s], Par:[%s]\n", pLookUpName, pParams);
    char sParams[512];
    strncpy(sParams, pParams, 512);
    sParams[511] = '\0';

    int iResult = -1;
    if (m_pVR != NULL) {
        int iType = LookUpFactory::instance()->getLookUpType(pLookUpName);
        if (iType >= 0) {
            printf("Type is %d\n", iType);
            double dMin = (m_pVR != NULL)?m_pVR->getMin():dNaN;
            double dMax = (m_pVR != NULL)?m_pVR->getMax():dNaN;

            double adParams[5];
            // first fill with defaults
            int iNumParams = LookUpFactory::instance()->getNumParams(iType);
            for (int i = 0; i < iNumParams; i++) {
                adParams[i] = LookUpFactory::instance()->getParamDefault(iType, i);
            }
            // put min/max where useful
            switch (iType) {
            case LOOKUP_UCHAR:
                adParams[0] = dMax;
                break;
            case LOOKUP_BINVIEW:
            case LOOKUP_SUN:
            case LOOKUP_RAINBOW:
            case LOOKUP_VEG:
            case LOOKUP_DENSITY:
                adParams[0] = dMin;
                adParams[1] = dMax;
                break;
            case LOOKUP_POP:
            case LOOKUP_BIOME:
                adParams[0] = dMax;
                break;
            case LOOKUP_GEO:
                adParams[0] = dMin;
                adParams[1] = dMax;
                adParams[2] = 0;
                break;
            case LOOKUP_THRESH:
                adParams[0] = (dMax - dMin)/2;
                break;
            case LOOKUP_ZEBRA:
                adParams[0] = 16;
                break;
            case LOOKUP_SEGMENT:
                adParams[0] = dMin;
                adParams[1] = dMax;
                adParams[2] = 100;
                adParams[3] =   8;
            }
            // user defined
            iResult = overwriteParams(sParams, adParams, 5);
            if (iResult == 0) {
                LookUp *pLU = setLookUp(iType, adParams);
                if (pLU != NULL) {
                    iResult = 0;
                }
            } else {
                printf("Bad param info\n");
            }
        }
    }
    return iResult;
}


//-----------------------------------------------------------------------------
// setReader
//   set ValReader for file if it is a BMAP-file, and LookUp Params
//
int xbv::setReader(const char *pFileName, const char *pRange) {
    if (m_bDying) {
        //        abort();
    }
     
    int iW = -1;
    int iH = -1;

    printf("inside setReader(%s)\n", pFileName);
    int iResult = -1;
    bool bInterp = false;

    LookUp *pLU=NULL;
    int iLUType;
    double adParams[5];

    m_iVRType = QMapHeader::getQMapType(pFileName);
    printf("Type : %d\n", m_iVRType);
    if (QMAP_TYPE_OK(m_iVRType)) {
        iResult = 0;

        ValReader *pVR = NULL;
        if (pRange == NULL) {
            pVR = QMapUtils::createValReader((char *)pFileName, bInterp); 
        } else {
            pVR = QMapUtils::createValReader((char *)pFileName, pRange, true, bInterp); 
        }
        if (pVR != NULL) {
            pVR->scanValues();
            strcpy(m_sPrevFile, pFileName);
            printf("Data range :[%f,%f]\n", pVR->getMin(),pVR->getMax());
            if (m_pLU == NULL) {
                pLU = NULL;
                if (m_iVRType == QMAP_TYPE_UCHAR) {
                    printf("UCHAR with name [%s], and max %f\n", pVR->getVName(), pVR->getMax());
                    if ((strcmp(pVR->getVName(), "Biome") == 0)) {
                        iLUType = LOOKUP_BIOME;
                        adParams[0] = pVR->getMax();
                    } else {
                        iLUType = LOOKUP_UCHAR;
                        adParams[0] = pVR->getMax();
                    }
                } else if (strcmp(pVR->getVName(), "Alt") == 0) {
                    printf("Have Alt\n");
                    m_bGeoView = true;
                    iLUType = LOOKUP_GEO;
                    adParams[0] = -1000;
                    adParams[1] = pVR->getMax();
                    adParams[2] = 0;
                } else {
                    iLUType = LOOKUP_RAINBOW;
                    adParams[0] = pVR->getMin();
                    adParams[1] = pVR->getMax();
                }
                pLU = setLookUp(iLUType, adParams);
            } else {
                pLU = m_pLU;
            }
            setValReader(pVR, pLU);
            iW = pVR->getNRLon();
            iH = pVR->getNRLat();
        } else {
            iResult = -1;
        }
    } else if (m_iVRType == QMAP_TYPE_NULL) {
        printf("file doesn't exist: [%s]\n", pFileName);
        iResult = -2;
    } else {
        printf("file doesn't exist 2: [%s]\n", pFileName);
        iResult = -3;
    
    }
    
    if (iResult == 0) {
        printf("Resizing to %dx%d\n", iW, iH);
        Glib::RefPtr<Gdk::Screen> scr = Gdk::Screen::get_default();

        if (iW > scr->get_width()-60) {
            iW = scr->get_width()-60;
        }
        if (iH > scr->get_height()-90) {
            iH = scr->get_height()-90;
        }
        set_size_request(iW+20, iH+20);
        resize(iW+20, iH+20);
    }
    
    return iResult;
}

//-----------------------------------------------------------------------------
// setLU
//
void xbv::setLU(int iLUType) {
    bool bChanged = true;
    double adParams[5];
    bzero(adParams, 5*sizeof(double));

    double dMin = dNaN;
    double dMax = dNaN;
    if (m_pVR != NULL) {
        dMin = m_pVR->getMin();
        dMax = m_pVR->getMax();
        
    } else {
        bChanged = false;
    }


    if (m_pVR != NULL) {
        if ((iLUType == LOOKUP_UCHAR) || (iLUType == LOOKUP_BIOME)) {
            adParams[0] = m_pVR->getMax();
        } else {
            adParams[0] = m_pVR->getMin();
            adParams[1] = m_pVR->getMax();
        }
    } else {
        bChanged = false;
    }


    if (bChanged) {
        switch (iLUType) {
        case LOOKUP_UCHAR:
        case LOOKUP_BIOME:
            adParams[0] = dMax;
            break;
        case LOOKUP_ZEBRA:
            adParams[0] = 16;
            break;
        case LOOKUP_THRESH:
            adParams[0] = (dMin+dMax)/2;
            break;
            // fall through
        default:
            adParams[0] = dMin;
            adParams[1] = dMax;
            if (iLUType == LOOKUP_SEGMENT) {
                adParams[2] = (dMin+dMax)/16;
                adParams[3] = 8;
            } else {
                adParams[2] = 0.0;
            }
        }
                

        LookUp *pLU = setLookUp(iLUType, adParams);
        if (m_pLU != NULL) {
            if (m_pLU != pLU) {
                delete m_pLU;
            }
        }
        m_pLU = pLU;
        createImage();
    }
}

//-----------------------------------------------------------------------------
// setReader
//   set ValReader for file, and LookUp Params
//
int xbv::setRawReader(const char *pFileName, int iType, 
                      const char *pLonMin, const char *pLonMax, const char *pDLon, 
                      const char *pLatMin, const char *pLatMax, const char *pDLat) {
    int iResult = -1;
    bool bInterp = false;
    ValReader *pVR = NULL;
    int iHeaderSize = 0;
    char *pEnd;
    double dLonMin;
    double dLonMax;
    double dDLon;
    double dLatMin;
    double dLatMax;
    double dDLat;
    printf("converting params...\n");
    dLonMin = strtof(pLonMin, &pEnd);
    if (*pEnd == '\0') {
        dLonMax = strtof(pLonMax, &pEnd);
        if (*pEnd == '\0') {
            dDLon = strtof(pDLon, &pEnd);
            if (*pEnd == '\0') {
                dLatMin = strtof(pLatMin, &pEnd);
                if (*pEnd == '\0') {
                    dLatMax = strtof(pLatMax, &pEnd);
                    if (*pEnd == '\0') {
                        dDLat = strtof(pDLat, &pEnd);
                        if (*pEnd == '\0') {
                            iResult = 0;
                        }
                    }
                }
            }
        }
    }

    m_iVRType = iType;

    //    fprintf (stderr, "  --> %d (%s)\n", iResult, pEnd);
    if (iResult == 0) {
         printf("creating reader for type %d (%s)...\n", m_iVRType, QMapHeader::getTypeName(m_iVRType));
        switch (m_iVRType) {
        case QMAP_TYPE_UCHAR:
            
            pVR =  new QMapReader<uchar>((char *)pFileName, iHeaderSize, 
                                        dDLon, dLonMin, dLonMax, 
                                        dDLat, dLatMin, dLatMax, bInterp); 
            break;
        case QMAP_TYPE_SHORT:
            pVR =  new QMapReader<short int>((char *)pFileName, iHeaderSize, 
                                        dDLon, dLonMin, dLonMax, 
                                        dDLat, dLatMin, dLatMax, bInterp); 
             break;
        case QMAP_TYPE_INT:
            pVR =  new QMapReader<int>((char *)pFileName, iHeaderSize, 
                                        dDLon, dLonMin, dLonMax, 
                                        dDLat, dLatMin, dLatMax, bInterp); 
            break;
        case QMAP_TYPE_LONG:
            pVR =  new QMapReader<long>((char *)pFileName, iHeaderSize, 
                                        dDLon, dLonMin, dLonMax, 
                                        dDLat, dLatMin, dLatMax, bInterp); 
            break;
        case QMAP_TYPE_FLOAT:
            pVR =  new QMapReader<float>((char *)pFileName, iHeaderSize, 
                                        dDLon, dLonMin, dLonMax, 
                                        dDLat, dLatMin, dLatMax, bInterp); 
            break;
        case QMAP_TYPE_DOUBLE:
            pVR =  new QMapReader<double>((char *)pFileName, iHeaderSize, 
                                        dDLon, dLonMin, dLonMax, 
                                        dDLat, dLatMin, dLatMax, bInterp); 
            break;
        default:
            pVR = NULL;
            iResult = -2;
        }
        //        fprintf(stderr, "  -->%p, %d\n", pVR, iResult);
    }

    if (pVR != NULL) { 
        if (pVR->isOK()) {
            bool bOK  = pVR->extractData();
            if (bOK) {
                pVR->scanValues();
                LookUp *pLU = NULL;
                if (m_iVRType == QMAP_TYPE_UCHAR) {
                    pLU = new UCharLookUp((uchar) pVR->getMax());
                } else {
                    pLU = new BVLookUp(pVR->getMin(), pVR->getMax());
                }

                iResult = 0;
                setValReader(pVR, pLU); 

            } else {
                iResult = -3;
                delete pVR;
                pVR = NULL;
            }       
        } else {
            iResult = -4;
            delete pVR;
            pVR = NULL;
        }
    }

    return iResult;
}


//-----------------------------------------------------------------------------
// doCommands
//   execute the commands in the string
//   (comma-separated)
//
void xbv::doCommands(char *pCommands) {
    char *pCtx;
    printf("Doing command [%s]\n", pCommands);
    if (*pCommands != '\0') {
        char *p = strtok_r(pCommands, ",", &pCtx);
        while (p != NULL) {
            printf("CCCCC [%s]\n", p);
            if (strncasecmp(p, "LU:", 3) == 0) {
                char *p2 = strchr(p+3, ':');
                if (p2 != NULL) {
                    *p2 = 0;
                    printf("CCCCC setting Lookup %s (params %s)\n", p+3, p2+1);
                    setLookUp(p+3, p2+1);
                    printf("CCCCC --> LU is %p\n", m_pLU);
                }
            } else if (strncasecmp(p, "PNG:", 4) == 0) {
                printf("CCCCC writing PNG [%s]\n", p+4);
                createPNG(p+4);
            } else if (strncasecmp(p, "RAW:", 4) == 0) {
                printf("CCCCC writing RAW [%s]\n", p+4);
                createRAW(p+4);
            } else if (strncasecmp(p, "VAL:", 4) == 0) {
                char *pCtx;
                char *p0 = strtok_r(p, ":", &pCtx);
                if (p0 != NULL) {
                    // this is "VAL"
                    printf("Have VAL\n");
                    p0 = strtok_r(NULL, ":", &pCtx);
                    if (p0 != NULL) {
                        char *pFileName = p0;
                        printf("Have Filename %s\n", pFileName);

                        p0 = strtok_r(NULL, ":", &pCtx);
                        if (p0 != NULL) {
                            char *pHeader = p0;
                            printf("Have Header %s\n", pHeader);
                            FILE * fOut = fopen(pFileName, "wt");
                            fprintf(fOut, "%s\t", pHeader);
                            p0 = strtok_r(NULL, ":", &pCtx);
                            fprintf(stderr, "after header [%s]\n", p0);
                            while (p0 != NULL) {
                                double dLon;
                                double dLat;
                                int iRead = sscanf(p0, "%lf|%lf", &dLon, &dLat);
                                printf("read %d\n", iRead);
                                if (iRead == 2) {
                                    if (dLon < -180) {
                                        dLon += 360;
                                    } else if (dLon > 180) {
                                        dLon -= 360;
                                    }
                                    double dVal = dNaN;
                                    if (m_pVR != NULL) {
                                        dVal = m_pVR->getDValue(dLon, dLat);
                                    } else {
                                        printf("No reader\n");
                                    }
                                    fprintf(fOut, "%f\t", dVal);
                                } else {
                                    printf("rad %d intead of 2\n", iRead); 
                                }
                                p0 = strtok_r(NULL, ":", &pCtx);
                            }
                            fprintf(fOut, "\n");
                            fclose(fOut);
                        } else {
                            printf("Expected Header\n");
                        }
                    } else {
                        printf("Expected file name\n");
                    }
                }
            } else {
                showInteractiveCommands();
            }
            p = strtok_r(NULL, "," , &pCtx);
        }
    } else {
        showInteractiveCommands();
    }
}

//-----------------------------------------------------------------------------
// showInteractiveCommands
// 
void xbv::showInteractiveCommands() {
    printf("\n");
    printf("\n");
    printf("Interactive commands\n");
    printf("====================\n");
    printf("  call qmap viewer with argument --command=\"<cmd>[,<cmd>]*\"\n");
    printf("where cmd is one of\n");
    printf("  LU\":\"<LUName>\":\"<LUParam>[\":\"<LUParam>]*\n");
    printf("        set LookUp given by LUName (only makes sense if a png is later created)\n");
    printf("  PNG\":\"<PNGName>\n");
    printf("        create a PNG from QMAP and save in PNGName\n");
    printf("  RAW\":\"<RAWName>\n");
    printf("        create a RAW binary file from QMAP and save in RAWName\n");
    printf("  VAL\":\"<outfile>\":\"<header>\"[:\"<lon>\"|\"<lat>]*\n");
    printf("        create file outfile with given header followed by the \n");
    printf("        QMAP/SNAP-values for the given coords\n");
    printf("        Example: \"VAL:values.dat:values for oink:22|33:9|47\"\n");
    printf("\n");
 }


//-----------------------------------------------------------------------------
// expose_action
//   show vectors if reqiuored
//
bool xbv::expose_action(GdkEventExpose *ev) {
    
    if (m_bShowVectors) {
        Glib::RefPtr<Gdk::Window> window = m_imgMain.get_window();
        if(window) {
            Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
            drawVectors(cr, 1.0);
        }
    }
    return true;
}

//-----------------------------------------------------------------------------
// loadVectors
//   load vector data from a textfile consisting of lines
//   <header> ":" <x1> <y1> <z1>  <x2> <y2> <z2>
//
int  xbv::loadVectors(char *pVectors) {
    int iResult = 0;
    char *pS = strchr(pVectors, ':');
    if (pS != NULL) {
        *pS = '\0';
        pS++;
        char *pEnd;
        float fScale = strtod(pS, &pEnd);
        if (*pEnd=='\0') {
            m_fVecScale = fScale;
        }
    }



    // expect lines of the form "name1 name2 name3 : ax ay az bx by bz"
    LineReader *pLR = LineReader_std::createInstance(pVectors, "rt");
    if (pLR != NULL) {
        while ((iResult == 0) && (!pLR->isEoF())) {
            char *p = pLR->getNextLine();
            char *p0 = p;
            if (p != NULL) {
                p = strchr(p, ':');
                if (p != NULL) {
                    p++;
                    double ax, ay, az;
                    double bx ,by, bz;
                    sscanf(p, "%lf %lf %lf   %lf %lf %lf", &ax, &ay, &az, &bx, &by, &bz);
                    Vec3D A(ax, ay, az);
                    Vec3D B(bx, by, bz);
                    std::pair<Vec3D, Vec3D> P(A, B);

                    double d = B.calcNorm();
                    if (d < 30000) {
                        m_vVecs.push_back(P);
                    } else {
                        printf("Ignoring line (>%f): [%s]\n", d, p0);
                    }
                } else {
                    iResult = -1;
                }
            }
        }
        delete pLR;
    }
    if (iResult == 0) {
        char *p1 = strrchr(pVectors, '.');
        if (p1 != NULL) {
            *p1= '\0';
        }
        p1 = strrchr(pVectors, '/');
        if (p1 != NULL) {
            p1++;
        } else {
            p1 = pVectors;
        }
        strcpy(m_sVecOut, p1);
        strcat(m_sVecOut, "_vec.png");
        printf("vec-output to [%s]\n", m_sVecOut);
    }
    return iResult;
}

//-----------------------------------------------------------------------------
// drawVectors
//   draw vectors in given Cairo-context, scale linewidth by dS
//
void xbv::drawVectors(Cairo::RefPtr<Cairo::Context> &cr, double dS) {
       cr->save();
        double ux=m_dZoom, uy=m_dZoom;
        cr->device_to_user_distance (ux, uy);
        if (ux < uy)
            ux = uy;
        cr->set_line_width (dS*ux);

        cr->set_source_rgba(0.7, 0.2, 0.8, 1.0);

        for (uint i = 0; i < m_vVecs.size(); i++) {
            Vec3D A(&m_vVecs[i].first);
            Vec3D B(&m_vVecs[i].second);
            
            B.scale(m_fVecScale); // 100
            B.add(&A);
            
            //   printf("(%f,%f,%f), (%f, %f, %f)\n", A.m_fX, A.m_fY, A.m_fZ, B.m_fX, B.m_fY, B.m_fZ);

            if (m_pVR != NULL) {
                double iX1 = m_pVR->Lon2X(A.m_fX);
                double iY1 = m_pVR->Lat2Y(A.m_fY);
                double iX2 = m_pVR->Lon2X(B.m_fX);
                double iY2 = m_pVR->Lat2Y(B.m_fY);
                if (m_bFlipped) {
                    iY1 = m_pVR->getNRLat() - iY1;
                    iY2 = m_pVR->getNRLat() - iY2;
                }
                cr->move_to(iX1, iY1);
                cr->line_to(iX2, iY2);
            }            

            
        }
       
        
        cr->stroke();
        
        cr->restore();  // back to opaque black 
}

//-----------------------------------------------------------------------------
// drawVectors
//   draw vectors into window, and into PNG-files with and without background
//
int xbv::drawVectors() {
    ///// draw vectors in window
    Glib::RefPtr<Gdk::Window> window = m_imgMain.get_window();
    if(window) {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
        drawVectors(cr, 1.0);
    }
    
    // save vectors in file

    saveVectors("vectors.png", false);
      



    // save vectors and background in file
    //stride = Cairo::ImageSurface::format_stride_for_width (format, width);

    saveVectors(m_sVecOut, true);


    return 0;
}

//-----------------------------------------------------------------------------
// saveVectors
//   draw vectors into into PNG-files with and without background
//
int xbv::saveVectors(std::string sFileName, bool bWithBackground) {
    
    uchar *pData = NULL;
    int stride = 4;//Cairo::ImageSurface::format_stride_for_width (Cairo::FORMAT_RGB24, (unsigned int)m_pVR->getNRLon()); // doesn' work in this old version
    printf("stride is %d\n", stride);
    if (bWithBackground) {
        

        pData = new uchar[m_pVR->getNRLon()*m_pVR->getNRLat()*stride];
        uchar *p0 = pData;
        for (uint i = 0; i < m_pVR->getNRLat(); i++) {
            uchar *p = p0;
            for (uint j = 0; j < m_pVR->getNRLon(); j++) {
                
                double dValue = m_pVR->getDValue(j, i);
                
                uchar uRed;
                uchar uGreen;
                uchar uBlue;
                uchar uAlpha;
                m_pLU->getColor(dValue, uRed, uGreen, uBlue, uAlpha);
                
                *p++ = (guint8) uBlue;
                *p++ = (guint8) uGreen;
                *p++ = (guint8) uRed;
                *p++ = (guint8) 255;
            }
            p0 += m_pVR->getNRLon()*stride;
        }
    }
    Cairo::RefPtr<Cairo::ImageSurface> surface = bWithBackground ?
        Cairo::ImageSurface::create(pData, Cairo::FORMAT_ARGB32, m_pVR->getNRLon(), m_pVR->getNRLat(), stride*m_pVR->getNRLon()) :
        Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, m_pVR->getNRLon(), m_pVR->getNRLat());

    /*
    Cairo::RefPtr<Cairo::ImageSurface> surface =
        Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, m_pVR->getNRLon(), m_pVR->getNRLat());
    */
    Cairo::RefPtr<Cairo::Context> cr2 = Cairo::Context::create(surface);
    drawVectors(cr2, 1.0);
 
    surface->write_to_png(sFileName);

    if (pData != NULL) {
        delete[] pData;
    }
    return 0;
}



int xbv::loadArcs(char *pArcs) {
    int iResult = -1;
    m_vArcs.clear();
    std::vector<std::pair<double, double> > vArc;
    LineReader *pLR = LineReader_std::createInstance(pArcs, "rt");
    if (pLR != NULL) {
        iResult = 0;
        bool bSearch = true;
        char *p;
        while (bSearch && (iResult == 0) && !pLR->isEoF()) {
            p = pLR->getNextLine();
            if ((p != NULL) && (strstr(p, "ARC") != NULL)) {
                bSearch = false;
            }
        }
        bool bGoOn = true;
        while (bGoOn && (iResult == 0) && !pLR->isEoF()) {
            p = pLR->getNextLine(GNL_IGNORE_BLANKS);
            // ass.: p != NULL
            int iNumCoords = -1;
            int iRead = sscanf(p, "%*d %*d %*d %*d %*d %*d %d", &iNumCoords);
            if (iRead == 1) {
                printf("Number of coords will be %d\n", iNumCoords);
                if (iNumCoords >= 0) {
                    if (iNumCoords > 0) {
                        // read numcoords coordinates
                        int iC = 0;
                        while ((iResult == 0) && (iC < iNumCoords)) {
                            p = pLR->getNextLine(GNL_IGNORE_BLANKS);
                            char *p0 = strtok(p, " \t\n");
                            while ((iResult == 0) && (p0 != NULL)) {
                                double dLon = atof(p0);
                                p0 = strtok(NULL, " \t\n");
                                if (p0 != NULL) {
                                    double dLat = atof(p0);

                                    printf("Read pair (%f, %f)\n", dLon, dLat);
                                    std::pair<double, double> cc(dLon, dLat);
                                    vArc.push_back(cc);
                                    iC++;
                                    p0 = strtok(NULL, " \t\n");
                                } else {
                                    iResult =-2;
                                }
                            }
                        }
                    } else {
                        bGoOn = false;
                    }
                    // save vector
                    // clear vector
                } else {
                    iResult = -1;
                }
            } else {
                printf("Bad subheader [%s]\n", p);
            }
            
            m_vArcs.push_back(vArc);
            vArc.clear();
        }   
        
        printf("loaded %zd arcs\n", m_vArcs.size());
        delete pLR;
    }
    return iResult;
}


int xbv::drawArcs() {
    Glib::RefPtr<Gdk::Window> window = m_imgMain.get_window();
    if(window) {
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
        cr->save();
        double ux=m_dZoom, uy=m_dZoom;
        cr->device_to_user_distance (ux, uy);
        if (ux < uy) {
            ux = uy;
        }
        double dS = 1.0;
        cr->set_line_width (dS*ux);
        
        cr->set_source_rgba(0.75, 0.3, 0.85, 1.0);

        if (m_pVR != NULL) {
        

            for (uint i = 0; i < m_vArcs.size(); i++) {
                std::vector<std::pair<double, double> > vArc = m_vArcs[i];
                
                double dLon0 = vArc[0].first;
                double dLat0 = vArc[0].second;
                
                double iX0 = m_pVR->Lon2X(dLon0);
                double iY0 = m_pVR->Lat2Y(dLat0);
                if (m_bFlipped) {
                    iY0 = m_pVR->getNRLat() - iY0;
                }
                cr->move_to(iX0, iY0);
                
                for (uint j = 1; j < vArc.size(); j++) {
                    double dLon1 = vArc[j].first;
                    double dLat1 = vArc[j].second;
                    double iX1 = m_pVR->Lon2X(dLon1);
                    double iY1 = m_pVR->Lat2Y(dLat1);
                    if (m_bFlipped) {
                        iY1 = m_pVR->getNRLat() - iY1;
                    }
                    cr->line_to(iX1, iY1);
                }            
            }
            cr->stroke();

        }
        
        
    
        cr->restore();  // back to opaque black 
    }
    return 0;
}

