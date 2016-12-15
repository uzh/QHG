#include <stdio.h>
#include <glibmm.h>
#include <giomm.h>

#include "strutils.h"
#include "LineReader.h"
#include "FileList.h"
#include "Pattern.h"
#include "IQScene.h"
#include "IQView.h"
#include "IQAnimator.h"

//---------------------------------------------------------------------------
// constructor
//
IQAnimator::IQAnimator(IQScene *pIQS,  Glib::Dispatcher *pDisp) 
    : m_pLR(NULL),
      m_pIQS(pIQS),
      m_pDisp(pDisp),
      m_bRunning(false),
      m_bGoOn(false),
      m_bUseQuats(false),
      m_bUseCol(false) {


}

//---------------------------------------------------------------------------
// launch
//   public function to start thread
//
void IQAnimator::launch(LineReader *pLR,  bool bUseQuats, bool bUseColor, float fInterval) {
    m_pLR = pLR;
    m_fInterval = fInterval;
    m_bUseQuats = bUseQuats;
    m_bUseCol = bUseColor;
    m_pThread = Glib::Thread::create(sigc::mem_fun(*this, &IQAnimator::start_function), true);
}


//---------------------------------------------------------------------------
// start_function
//   the function for starting the thread
//
void IQAnimator::start_function() {
    m_bRunning = true;
    m_bGoOn= true;
    while (m_bGoOn && !m_pLR->isEoF()) { 
        char *p = m_pLR->getNextLine();
        if (p != NULL) {
            printf("Line [%s]\n", p);
            sscanf(p, "%s %s %f %f %f %f", m_sCurFile, m_sPNGFile, &(m_qCur[0]), &(m_qCur[1]), &(m_qCur[2]), &(m_qCur[3]));
            printf("got [%s][%s], %f %f %f %f\n", m_sCurFile, m_sPNGFile, m_qCur[0], m_qCur[1], m_qCur[2], m_qCur[3]);
            while (m_pIQS->m_bLoading) {
                usleep(5000);
            }
            m_pDisp->emit();
            sleep(m_fInterval);
        }
    }
    m_bRunning = false;
    m_pDisp->emit();
    printf("Animator stopped\n");
}


//---------------------------------------------------------------------------
// stop
//
void IQAnimator::stop() { 
    m_bGoOn = false;
    m_bRunning = false;
    m_DirMon = (Glib::RefPtr<Gio::FileMonitor>) NULL;
};

//---------------------------------------------------------------------------
// launch
//   public function to start thread
//
void IQAnimator::launch(const char *pDir, const char *pMask,  bool bUseCol, float fInterval) {
    niceDir(pDir, m_sDir);
    strcpy(m_sMask, pMask);
    m_bUseCol = bUseCol;
    m_fInterval = fInterval;
    printf("Launching 'start_monitoring'\n");
    m_pThread = Glib::Thread::create(sigc::mem_fun(*this, &IQAnimator::start_monitoring), true);

}


//---------------------------------------------------------------------------
// start_dir_monitor
//   the function for starting the thread
//
void IQAnimator::start_monitoring() {
    m_bGoOn = true;
    m_bRunning = true;
    // cleear m_qFiles (has no clear() method
    while (!m_qFiles.empty())   {
        m_qFiles.pop();
    }

    int iNum = collectMatchingFiles(m_sDir, m_sMask);
    printf("collected %d files in [%s] matching [%s]\n", iNum, m_sDir, m_sMask);
    
    printf("Launching 'start_dir_monitor'\n");
    m_pThreadMoni = Glib::Thread::create(sigc::mem_fun(*this, &IQAnimator::start_dir_monitor), true);
    printf("setting timeout to %f\n", m_fInterval*1000);
    Glib::signal_timeout().connect( sigc::mem_fun(*this, &IQAnimator::on_timeout), m_fInterval*1000 );

}

int s_i=0;
//---------------------------------------------------------------------------
// on_timeout
//
bool IQAnimator::on_timeout() {
    char sW[]={'\\', '|', '/', '-'};
    printf("%c\b",sW[s_i]);fflush(stdout);
    s_i=(s_i+1)%4;
    if (!m_pIQS->m_bLoading) {
    if (m_qFiles.size() > 0) {
        strcpy(m_sCurFile, m_qFiles.front().c_str());
        m_qFiles.pop();
        printf("Curfile is [%s]\n", m_sCurFile);

        printf("Waiting for IQScene to stop loading\n");
        while (m_pIQS->m_bLoading) {
            usleep(5000);
        }
        printf("IQScene ready\n");
        m_pDisp->emit();
    }}
    return m_bGoOn;
}

//---------------------------------------------------------------------------
// start_dir_monitor
//
void IQAnimator::start_dir_monitor() {
    Glib::RefPtr<Gio::File> dir = Gio::File::create_for_path(m_sDir);
    m_DirMon = dir->monitor_directory();
    m_DirMon->signal_changed().connect(sigc::mem_fun(*this, &IQAnimator::on_directory_changed));
}

//---------------------------------------------------------------------------
// on_directory_changed
//
void IQAnimator::on_directory_changed(const Glib::RefPtr<Gio::File >&  	file,
                                      const Glib::RefPtr<Gio:: File >&  	other_file,
                                      Gio::FileMonitorEvent  	event_type ) {
    
    //    printf("have directory changed - type %d\n", event_type);
    /*
    const char *pO = (other_file!=NULL)?other_file->get_path().c_str():NULL;
    switch(event_type) {
    case Gio::FILE_MONITOR_EVENT_CHANGED 	:
        printf("CHANGED [%s] [%s]\n", file->get_path ().c_str(), pO);
        break;
    case  Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT: 	
        printf("DONE_HINT [%s] [%s]\n", file->get_path ().c_str(), pO);
        break;
    case Gio:: FILE_MONITOR_EVENT_DELETED 	:
        printf("DELETEDED [%s] [%s]\n", file->get_path ().c_str(), pO);
        break;
    case Gio:: FILE_MONITOR_EVENT_CREATED 	:
        printf("CREATED [%s] [%s]\n", file->get_path ().c_str(), pO);
        break;
    case Gio:: FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED: 	
        printf("ATTRIBUTE_CHANGED [%s] [%s]\n", file->get_path ().c_str(), pO);
        break;
    case  Gio::FILE_MONITOR_EVENT_PRE_UNMOUNT 	:
        printf("PRE_UNMOUNT [%s] [%s]\n", file->get_path ().c_str(), pO);
        break;
    case  Gio::FILE_MONITOR_EVENT_UNMOUNTED 	:
        printf("UNMOUNTED [%s] [%s]\n", file->get_path ().c_str(), pO);
        break;
    case Gio:: FILE_MONITOR_EVENT_MOVED :
        printf("MOVED [%s] [%s]\n", file->get_path ().c_str(), pO);
        break;
    }
        if (event_type == Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT) {
*/
        if (event_type == Gio::FILE_MONITOR_EVENT_CREATED) {

        bool bMatches = false;
        const char *pFileTemp = strrchr(file->get_path ().c_str(), '/');
        const char *pFile = pFileTemp;
        if (pFileTemp != NULL) {
            pFile = pFileTemp+1;
        }
        printf("Found created file [%s]\n", pFile);
        Pattern *pP = new Pattern();
        pP->addPattern(m_sMask);
        VEC_STRINGS *pvCurMatches = new VEC_STRINGS();
        bMatches = pP->matchPattern(0, pFile, *pvCurMatches);
    //@@-->    bool bMatches = match (pFile, m_sMask);
        if (bMatches) {
            printf("Found new matching file [%s]\n", pFile);
            char sFull[512];
            sprintf(sFull, "%s/%s", m_sDir, pFile);
            if (m_qFiles.size() > 0) {
                m_qFiles.push(sFull);
            } else {
                strcpy(m_sCurFile, sFull);
                while (m_pIQS->m_bLoading) {
                    usleep(5000);
                }
                m_pDisp->emit();
            }
        }
    }
}
    

//---------------------------------------------------------------------------
// collectMatchingFiles
//
int IQAnimator::collectMatchingFiles(const char *pDir, const char *pMask) {
    FileList *pFL = new FileList();
    pFL->reset();
    int iNum = pFL->initSearch(pDir, pMask);
    for (int j = 0; j < iNum; j++) {
        const char *pFile = pFL->getNextFile();
        char sPath[MAX_LINE];
        sprintf(sPath, "%s/%s", pDir, pFile);
        m_qFiles.push(sPath);
    }
    return iNum;
}
