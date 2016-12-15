#ifndef __IGTILINGPANEL_H__
#define __IGTILINGPANEL_H__ 

#include <gtkmm.h>
#include <vector>

#include "Observer.h"
#include "Observable.h"
#include "TilingTable.h"
#include "RegionSplitter.h"

class GridProjection;

class IGTilingPanel : public Gtk::VBox, public Observer, public Observable {
public:
    IGTilingPanel();
    virtual ~IGTilingPanel();

    void switchToTable(bool bFlat, int iType);
    void realize_action();

    RegionSplitter *getSplitter();
    // from Observer
    void notify(Observable *pObs, int iType,  const void *pCom);

protected:
    int m_iPage;

    void hideAll();
    std::vector<TilingTable*> m_vtTilingTables;

    
    

};

#endif
