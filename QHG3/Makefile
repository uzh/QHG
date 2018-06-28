#why doesn't this work automatically?
#export CSCS=_CSCS_

# essential library stuff
KERNEL    := kernel
#HEADER    := header
COMMON    := common
GEOINFO   := geoinfo
VISUAL    := visual
TILING    := tiling 
EXTERNAL  := external
QMAPS     := qmaps
IMPORT    := import
ICOSA     := icosa
IO        := io
MODULAR   := modular
POPS      := populations

GENES     := genes
TOOLS_IO  := tools_io
TOOLS_ICO := tools_ico
BIOMES    := biomes

ifeq ("X$(SHORT)", "X1")
    POPS    := pop_work
    MODULAR := mod_work
endif

#LIBRARIES := $(EXTERNAL) $(COMMON) $(KERNEL) $(MODULAR) $(GEOINFO) $(QMAPS)  $(ICOSA) $(IO) $(VISUAL) $(POPS)
LIBRARIES := $(EXTERNAL) $(COMMON) $(KERNEL) $(MODULAR) $(GEOINFO) $(QMAPS)  $(ICOSA) $(IO) $(GENES) $(POPS) 


MAINAPP   := app

DEST= x

ifeq "$(HOSTNAME)" ""
# $(warning trying hostname command)
  HOSTNAME:=$(shell hostname)
#  $(warning result: '$(HOSTNAME)')
endif 
#$(warning result: '$(HOSTNAME)')

# set destination to executable directory
ifeq ("$(HOSTNAME)","aim-plankton.uzh.ch")
#	DEST=/home/jody/progs
	DEST=/mnt/data1/neander
#	DEST=/qhg1
else 
  ifeq ("$(HOSTNAME)","aim-triops.uzh.ch")
        DEST=/mnt/data1/neander
  else
    ifeq ("$(HOSTNAME)","enki")
        DEST=./qhgtest
    else
	DEST=./
    endif
  endif
endif
#$(warning Had to remove DataAcquirerAvgAge because it was not in cvs)
#$(warning   Makefiles: kernel, app; SnapDataCollection.cpp)


# set additional compile flags (CADD)
# set ar flags (ARFL)
# set post-processing action (POST) 
ifdef CSCS
  export CADD1=
  POST=
  export CFLAGS=-g -q64 -qstrict  -qstaticinline
  export ARFL=-X64 -rv
  export MPICC=CC
  export GCC=CC
else
  ifdef PGI_VER
    export CADD1=
    POST=
    export CFLAGS=
    export ARFL=-rv
  else 
#export CADD1=-D_GLIBCXX_USE_CXX11_ABI=1
#    ifdef OPT
#      export CADD1= -O3 -Wall
#    else
#      ifdef PROF	
#        export CADD1= -pg -g -Wall 
#      else 
#        ifdef COV
#          export CADD1= -g -Wall  -fprofile-arcs -ftest-coverage 
#        else 
#          export CADD1= -g -Wall 
#        endif
#      endif
#    endif

#POST=./avgage_warning; cp app/TDMain $(DEST)
#      POST1=cp app/TDMain $(DEST)
#      POST2=cp plugagents/*.so $(DEST)/plugins
    export ARFL=-rv
  endif
endif

export WARNINGS=-Wall -Wshadow


ifdef AGSTO
  export CADD=$(CADD1) -DAGSTO
else
  export CADD=$(CADD1)
endif

#export CADD=-pg -Wall
#optimization flags: O1, O2, O3 worse performance
#export CFLAGS=-fomit-frame-pointer -march=pentium4 -dH
export MPI=_MPI_
export DEFS=
#export DEFS=-DSTRONGLOG 
export __USE_ISOC99=1

.PHONY: opti
.PHONY: debi


opti: export CFLAGS=-O3
opti: all

debi: export CFLAGS=-g -Wall
debi: all

.PHONY: all $(MAINAPP) $(LIBRARIES)
all: $(MAINAPP)
	$(POST1)

$(MAINAPP) $(LIBRARIES):
	$(MAKE) --directory=$@

plugs: sos
	$(POST2)

sos:
	$(MAKE) --directory=$(PLUGINS) new; 

$(MAINAPP): $(CONFIGURE) $(LIBRARIES)
$(POPS): $(MODULAR) $(KERNEL) $(COMMON) $(QMAPS) $(ICOSA) 
$(MODULAR):  $(KERNEL) $(GEOINFO) $(COMMON) $(QMAPS) $(ICOSA) 
$(KERNEL):  $(GEOINFO) $(COMMON) $(QMAPS) $(ICOSA) 
$(QMAPS):   $(COMMON) $(EXTERNAL) $(GEOINFO) $(KERNEL)
$(ICOSA): $(COMMON) $(QMAPS) $(EXTERNAL) $(HEADER)
$(IO): $(COMMON) $(QMAPS) $(KERNEL) $(HEADER)
$(GENES): $(COMMON) $(KERNEL)

new: clean 
	$(MAKE) all

newo: clean
	$(MAKE) opti

newd: clean
	$(MAKE) debi

plugins:
	$(MAKE) plugs; 

clean:
	@ echo "--- cleaning ---"; \
	for d in $(MAINAPP) $(LIBRARIES) $(VISUAL); \
        do                               \
          $(MAKE) --directory=$$d clean; \
        done

QHGMain:export DOMP=-DOMP_A
QHGMain: 
	$(MAKE) all && \
	$(MAKE) --directory=$(MAINAPP) QHGMainDirect


tools_n:
	$(MAKE) --directory=$(VISUAL) new && \
	$(MAKE) --directory=$(TOOLS_ICO) new && \
	$(MAKE) --directory=$(TOOLS_IO) new && \
        $(MAKE) --directory=$(IMPORT) new && \
        $(MAKE) --directory=$(GENES) new;

tools_c:
	$(MAKE) --directory=$(VISUAL) clean && \
	$(MAKE) --directory=$(TOOLS_ICO) clean && \
	$(MAKE) --directory=$(TOOLS_IO) clean && \
        $(MAKE) --directory=$(IMPORT) clean && \
        $(MAKE) --directory=$(GENES) clean;

tools:
	$(MAKE) --directory=$(VISUAL) all && \
	$(MAKE) --directory=$(TOOLS_ICO) app && \
	$(MAKE) --directory=$(TOOLS_IO) app && \
        $(MAKE) --directory=$(IMPORT) app && \
        $(MAKE) --directory=$(GENES) app;
