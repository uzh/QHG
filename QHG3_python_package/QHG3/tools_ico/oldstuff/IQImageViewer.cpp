#include <gtkmm.h>


#include "ProjInfo.h"
#include "IQImageViewer.h"
#include "notification_codes.h"



//----------------------------------------------------------------------------
// constructor
//
IQImageViewer::IQImageViewer(ProjInfo *pPI, int iW, int iH) 
    : /*Gtk::Alignment(Gtk::ALIGN_END, Gtk::ALIGN_END),*/
      m_pPI(pPI),
      m_iW(iW),
      m_iH(iH), 
      m_iPW(iW),
      m_iPH(iH), 
      m_pImgData(NULL),
      m_imgPreView(),
      m_chkLive("Live"), 
      m_VBox(false, 0), 
      m_bInterp(true),
      m_bUseQuat(false),
      m_bLive(false),
      m_bFresh(false) {

    m_pPI->addObserver(this);

    m_pImgData  = new guint8[m_iW*m_iH*3];
    m_chkLive.set_active(false);
    m_chkLive.signal_clicked().connect(sigc::mem_fun(*this, &IQImageViewer::live_action) );

    for (int i = 0; i < m_iW*m_iH*3; i+=3) {
        m_pImgData[i] = 255;
        m_pImgData[i+1] = 0;
        m_pImgData[i+2] = 0;
    }
    m_imgPreView.set_size_request(m_iW, m_iH);

    m_VBox.pack_start(m_chkLive, Gtk::PACK_SHRINK, 0);
    m_VBox.pack_start(m_imgPreView, Gtk::PACK_SHRINK, 0);
    add(m_VBox);
}


//----------------------------------------------------------------------------
// destructor
//
IQImageViewer::~IQImageViewer() {
    if (m_pImgData != NULL) {
        delete[] m_pImgData;
    }
}

//----------------------------------------------------------------------------
// createImage
//
void IQImageViewer::createImage(bool bInterp, bool bUseQuat, int iPW, int iPH) {
    // make sure VBox doesn't change size
    int iBW;
    int iBH;
    m_VBox.get_size_request(iBW, iBH);
    m_VBox.set_size_request(m_iW,iBH);

    // now do the work
    m_bInterp = bInterp;
    m_bUseQuat = bUseQuat;
    m_iPW = iPW;
    m_iPH = iPH;
    m_imgPreView.set_size_request(m_iPW, m_iPH);
    updateImage();
}

//----------------------------------------------------------------------------
// createImage
//
void IQImageViewer::updateImage() {
    if (m_bInterp) {
        if (m_bUseQuat) {
            m_pPI->fillRGBInterpolatedQ(m_iPW, m_iPH, m_pImgData);
        } else {
            m_pPI->fillRGBInterpolated(m_iPW, m_iPH, m_pImgData);
        }
    } else {
        m_pPI->fillRGBPoints(m_iPW, m_iPH, m_pImgData);
    }
    // paint the image
    printf("Painting\n");

    Glib::RefPtr<Gdk::Pixbuf> pP1 = 
        Gdk::Pixbuf::create_from_data(m_pImgData,
                                      Gdk::COLORSPACE_RGB,
                                      false, 
                                      8, 
                                      m_iPW, 
                                      m_iPH,
                                      m_iPW*3);
    m_imgPreView.set(pP1);
    m_imgPreView.queue_draw();
}

//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQImageViewer::notify(Observable *pObs, int iType, const void *pCom) {
    // printf("[IQScene::notify] received notification [%d] from %p\n", iCom , pObs);
    if (iType == NOTIFY_LU_CHANGE) {
        if (m_bLive) {
            updateImage();
        }
    } else if (iType == 0) {
        long iCom = (long) pCom;
        switch (iCom) {
        case NOTIFY_NEW_DATA:
            m_bFresh = true;
            break;

        case NOTIFY_MODEL_LOADED:
            //            printf("Model loaded L(%d) Q:(%d) F(%d)...............\n", m_bLive, m_bUseQuat, m_bFresh);
            if (m_bLive && (m_bUseQuat || m_bFresh)) {
                updateImage();
                m_bFresh = false;
            }
            break;
        default:
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// notify
//   from Observer
//
void IQImageViewer::live_action() {
    m_bLive = m_chkLive.get_active();
    if (m_bLive) {
        updateImage();
    }
}
