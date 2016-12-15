#ifndef __IQOVERLAY_H__
#define __IQOVERLAY_H__

#include <vector>
#include <set>

#include "types.h"
#include "icoutil.h"
#include "Observable.h"

class IQOverlay : public Observable {
public:
    IQOverlay();
    ~IQOverlay();

    void clear();
    bool contains(gridtype lNode);
    void addData(std::vector<gridtype> vOverlays);
    bool hasData();
    void show();
protected:
    std::set<gridtype> m_sNodes;
};

#endif
