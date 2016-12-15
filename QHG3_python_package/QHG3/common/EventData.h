#ifndef __EVENTDATA_H__
#define __EVENTDATA_H__

#include <vector>
#include <string>


class EventData {
public:
    EventData(int iEventType, char *pData);
    static EventData *parseDef(char *pDef);
    ~EventData() {}

    bool equals(EventData *pED);
    int   m_iEventType;
    std::string  m_sData;
    
    void toString(std::string &sCur) const;
    
};

struct EDCompare {
    bool operator() (const EventData *pED1,  EventData *pED2) const {
        bool bComp = false;
        if (pED1->m_iEventType < pED2->m_iEventType) {
            bComp = true;
        } else if (pED1->m_iEventType == pED2->m_iEventType) {
            bComp = pED1->m_sData < pED2->m_sData;
        }
        return bComp;
    }
};



#endif

