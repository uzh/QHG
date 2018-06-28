#include <stdio.h>
#include <string.h>

const unsigned int  BITSINBLOCK = 8*sizeof(unsigned long int);
const unsigned int  NUCSINBLOCK = BITSINBLOCK/2;

const static char s_asNuc[] = {'A', 'C', 'G', 'T'};
const char* p_asNuc = s_asNuc;

char* blockToNucStr(unsigned long int lBlock, char *pNuc) {
    pNuc[NUCSINBLOCK] = '\0';
    for (int i = 0; i < NUCSINBLOCK; i++) {
        pNuc[i] = p_asNuc[lBlock & 0x3];
        lBlock >>= 2;
    }
    return pNuc;
}

int main(int argc, char *argv[]) {

  if (argc != 1) {
    fprintf(stderr, "%s < <input> > <output>\n", argv[0]);
    return -1;
  }
  
//  FILE* FIN = fopen(argv[1], 'r');

//	fprintf(stderr,"files opened\n");

  char* name1 = new char[128];
  char* name2 = new char[128];
  char* name3 = new char[128];
  char* name4 = new char[128];
  char* name5 = new char[128];
  char* name6 = new char[128];
  
  char** adtp = new char*[4];
  for (int i=0; i<4; i++) {
    adtp[i] = new char[NUCSINBLOCK+1];
  }

  unsigned long int* genes = new unsigned long int[4];
  
  while (scanf("%s %s %s %s %s %s %lu %lu %lu %lu", 
		name1, name2, name3, name4, name5, name6, &genes[0], &genes[1], &genes[2], &genes[3]) == 10) {
    
    printf("%s %s %s %s %s %s ", name1, name2, name3, name4, name5, name6);
    for (int i = 0; i < 4; i++) {
      blockToNucStr(genes[i], adtp[i]);
    }
    for (int i = 0; i < NUCSINBLOCK; i++) {
      printf("%c %c ", adtp[0][i], adtp[2][i]);
    }
    for (int i = 0; i < NUCSINBLOCK; i++) {
      printf("%c %c ", adtp[1][i], adtp[3][i]);
    }
    printf("\n");
  }
  
  
  delete[] name1;
  delete[] name2;
  delete[] name3;

  for (int i = 0; i<4; i++) {
    delete adtp[i];
  }
  delete[] adtp;
  
//  fclose(FIN);
  
  return 0;
}

