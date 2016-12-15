#ifndef __MOVESTATWRITER_H__
#define __MOVESTATWRITER_H__

class MoveStats;

class MoveStatWriter {
public:
    MoveStatWriter(MoveStats *pMS);
    int writeToQDF(const char *pFileName, float fTime);
    int write(hid_t hFile);

protected:
    MoveStats *m_pMS;
    int writeMStatAttributes(hid_t hMStatGroup);
};

#endif
