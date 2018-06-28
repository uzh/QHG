#ifndef __IQANIMATOR_H__
#define __IQANIMATOR_H__

#include <queue>
#include <string>

class IQScene;
class LineReader;

class IQAnimator {
public:
    IQAnimator(IQScene *pIQS, Glib::Dispatcher *pDisp);
    void launch(LineReader *pLR,  bool bUseQuats,  bool bUseCol, float fInterval=0);
    void launch(const char *pDir, const char *pMask,  bool bUseCol, float fInterval=0);
    bool isRunning() { return m_bRunning;};
    char *getCurFile() { return m_sCurFile;};
    char *getPNGFile() { return m_sPNGFile;};
    float *getCurQuat() { return m_bUseQuats?m_qCur:NULL;};
    void stop();
    int collectMatchingFiles(const char *pDir, const char *pMask);
    bool useCol() { return m_bUseCol;};
protected:
    void start_function();
    void start_monitoring();
    void start_dir_monitor();
    bool on_timeout();
    void on_directory_changed(const Glib::RefPtr< Gio::File >&  	file,
                              const Glib::RefPtr< Gio::File >&  	other_file,
                              Gio::FileMonitorEvent  	event_type );

    LineReader *m_pLR;
    IQScene          *m_pIQS;
    Glib::Dispatcher *m_pDisp;
    Glib::Thread     *m_pThread;
    bool              m_bRunning;
    char              m_sCurFile[1024];
    char              m_sPNGFile[1024];
    float             m_fInterval;
    bool              m_bGoOn;
    float             m_qCur[4];
    bool              m_bUseQuats;
    bool              m_bUseCol;

    char m_sDir[512];
    char m_sMask[512];
    std::queue<std::string> m_qFiles;        
    Glib::RefPtr<Gio::FileMonitor> m_DirMon; 
    Glib::Thread     *m_pThreadMoni;
};


#endif
