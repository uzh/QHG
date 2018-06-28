#ifdef MPI
#  CC=$(MPICC)
#else
#  CC=$(GCC)
#endif

ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif


ifndef ARFL
  ARFL=-rv
endif


COPT=-c  $<  -o $@ $(CADD) $(CFLAGS)

all: plyfile.o


clean:
	rm -f *.o  *.ti	


plyfile.o: plyfile.c ply.h
	$(CC) $(COPT) -g $(DEFS)
