/*============================================================================
| crypto 
| 
|  Wrappers for hash algorithms
|  - sha-1
|  - md5
|  - ripemd160
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#define MD5_SIZE  16
#define SHA_SIZE  20
#define RIP_SIZE  20

// pSHAsum must point to buffer of size >= SHA_SIZE
int shasum(const char *pFile, unsigned char *pSHAsum);

// pMD5sum must point to buffer of size >= MD5_SIZE
int md5sum(const char *pFile, unsigned char *pMD5sum);

// pRIPsum must point to buffer of size >= RIP_SIZE
int ripsum(const char *pFile, unsigned char *pRIPsum);



#endif
