#why doesn't this work automatically?
#export CSCS=_CSCS_

# essential library stuff
KERNEL    := kernel
HEADER    := header
COMMON    := common
GEOINFO   := geoinfo
AGENTS    := agents
PLUGINS   := plugagents
VISUAL    := visual
TILING    := tiling 
EXTERNAL  := external
QMAPS     := qmaps
IMPORT    := import
CONV      := conv
ICOSA     := icosa
IO        := io
MODULAR   := modular
POPS      := populations

TOOLS_IO  := tools_io
TOOLS_ICO := tools_ico
BIOMES    := biomes

LIBRARIES := $(EXTERNAL) $(COMMON) $(KERNEL) $(MODULAR) $(HEADER) $(GEOINFO) $(QMAPS) $(ICOSA) $(IO) $(VISUAL) $(POPS)


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
  ifeq ("$(HOSTNAME)","aim-triops")
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
    ifeq ("$(HOSTNAME)","aim-triops.uzh.ch")
	  export MPICC=~/utils/cmpicc
	  export GCC=~/utils/cgcc
    else
	  export MPICC=mpic++
	  export GCC=g++
    endif

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
$(ICOSA): $(COMMON) $(QMAPS) $(EXTERNAL) $(VISUAL) $(HEADER)
$(IO): $(COMMON) $(QMAPS) $(KERNEL) $(HEADER)


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
	for d in $(MAINAPP) $(LIBRARIES); \
        do                               \
          $(MAKE) --directory=$$d clean; \
        done

tools_n:
	$(MAKE) --directory=$(TOOLS_ICO) new && \
        $(MAKE) --directory=$(TOOLS_IO) new && \
        $(MAKE) --directory=$(BIOMES) new;

tools_c:
	$(MAKE) --directory=$(TOOLS_ICO) clean && \
        $(MAKE) --directory=$(TOOLS_IO) clean && \
        $(MAKE) --directory=$(BIOMES) clean;

tools:
	$(MAKE) --directory=$(TOOLS_ICO) app && \
        $(MAKE) --directory=$(TOOLS_IO) app && \
        $(MAKE) --directory=$(BIOMES) app;
