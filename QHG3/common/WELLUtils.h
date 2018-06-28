#ifndef __WELLUTILS_H__
#define __WELLUTILS_H__

int stringToSeed(char *sSequence, std::vector<uint32_t> &vulState);
int phraseToSeed(const char *pPhrase, std::vector<uint32_t> &vulState);
int randomSeed(int iSeed, std::vector<uint32_t> &vulState);

WELL512 *createWELL(const char *pPhrase);

#endif
