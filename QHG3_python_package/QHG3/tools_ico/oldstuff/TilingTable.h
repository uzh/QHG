#ifndef __TILINGTABLE_H__
#define __TILINGTABLE_H__

#include <gtkmm.h>

#include "Observable.h"
class RegionSplitter;

class TilingTable : public Gtk::Table, public Observable {
public:
    virtual RegionSplitter *getSplitter()=0;
    virtual void setObject(int iWhat, void *pData)=0;
};



#endif 
