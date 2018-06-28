/*============================================================================
| crc
|
|  CRC16 implementation acording to CCITT standards 
|  
|  Author: Jody Weissmann
\===========================================================================*/ 


/// simple 16-bit crc
unsigned short crc_update(unsigned short crc, unsigned char c);
unsigned short crc_add(unsigned short crc, float *pfData, int iNumFloats);
unsigned short crc_add(unsigned short crc, int i);
unsigned short crc_add(unsigned short crc, float f);
unsigned short crc_add(unsigned short crc, double d);
unsigned short crc_add(unsigned short crc, spcid i);
/*============================================================================
| crc
|
|  CRC16 implementation acording to CCITT standards 
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

