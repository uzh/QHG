#ifndef __TILINGSICOLANDTABLE_H__
#define __TILINGSICOLANDTABLE_H__

#include <gtkmm.h>

#include "Observable.h"
#include "TilingTable.h"

class RegionSplitter;


class TilingsIcoLandTable : public TilingTable {
public:
    TilingsIcoLandTable();
    virtual ~TilingsIcoLandTable();

    virtual RegionSplitter *getSplitter();
    virtual void setObject(int iWhat, void *pData) {};

protected:
    RegionSplitter *m_pRS;

    Gtk::Label       m_lblExplanation;
    Gtk::Image       m_imgLand;

};

#endif

