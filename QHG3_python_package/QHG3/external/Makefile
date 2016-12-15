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

all:  libNR.a plyfile.o

libNR.a: nrutil.o splint.o spline.o splin2.o splie2.o polint.o polin2.o gaussj.o eigsrt.o jacobi.o svdcmp.o pythag.o betai.o betacf.o gammln.o
	ar $(ARFL) $@ $?

clean:
	rm -f *.o libGPC.a libNR.a *.ti	

nrutil.o: nrutil.c nrutil.h
	$(CC) $(COPT) $(DEFS)

splint.o: splint.c nrutil.h
	$(CC) $(COPT) $(DEFS)

spline.o: spline.c nrutil.h
	$(CC) $(COPT) $(DEFS)

splin2.o: splin2.c nrutil.h
	$(CC) $(COPT) $(DEFS)

splie2.o: splie2.c nrutil.h
	$(CC) $(COPT) $(DEFS)

polint.o: polint.c nrutil.h
	$(CC) $(COPT) $(DEFS)

gaussj.o: gaussj.c nrutil.h
	$(CC) $(COPT) $(DEFS)

polin2.o: polin2.c nrutil.h
	$(CC) $(COPT) $(DEFS)

betacf.o: betacf.c nrutil.h
	$(CC) $(COPT) $(DEFS)

betai.o: betai.c nrutil.h
	$(CC) $(COPT) $(DEFS)

gammln.o: gammln.c nrutil.h
	$(CC) $(COPT) $(DEFS)

plyfile.o: plyfile.c ply.h
	$(CC) $(COPT) -g $(DEFS)
