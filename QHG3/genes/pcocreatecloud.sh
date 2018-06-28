
function setvars {
    mode=$1
    QHG=
  # some constants
    if [ ${mode} == cloud ] 
    then
        QHG=/home/centos/qhg_data/QHG3
    elif [ ${mode} == kraken ] 
    then
        QHG=/data/jw_simulations/qhgtests/src/QHG3 
    elif [ ${mode} == triops ] 
    then
        QHG=/home/jody/progs/QHG3/trunk
    fi

    locfile=${QHG}/genes/LocationListGridsRed.txt
    gridfile=${QHG}/resources/grids/EmptyGridSGCV_256.qdf
    popprefix=ooa_pop-Sapiens_ooa_
    statprefix=ooa_SGCVNM
}


function showvars {
  echo "QHG:  ${QHG}"
  echo "loc:  ${locfile}"
  echo "grid: ${gridfile}"
  echo "popprefix:  ${popprefix}"
  echo "statprefix: ${statprefix}"
}


#-----------------------------------------------------------------------------
# checkExistence
#   expects params
#     maindir
#     times  (';'-separated three digit numbers, e.g. "000;010;020"
#
function checkExistence {
  loc_bExistOK=0
  maindir=$1
  times=$2
  times=( ${times//;/  } )
  last=`echo ${times[@]} | gawk '{ print $NF }'`
#  >&2 echo "checkExistence in [$maindir]: last=[$last]"
#  >&2 echo "checkExistence atimes: [${times[@]}]"

  if [ -d ${maindir} ]
  then
    popfile=${maindir}/${popprefix}
    statfile=${maindir}/${statprefix}
    
    if [ $bZip -eq 1 ]
    then
      if [ -f ${statfile}_${last}000.qdf.gz ]
      then
        loc_bExistOK=1  
        for t in ${times[@]}
        do
          if [ ! -f  ${popfile}_${t}000.qdf.gz ]
          then
            loc_bExistOK=0
            >&2 echo "gzipped popfile [${popfile}_${t}000.qdf.gz] does not exist"
          fi
        done
      else
        >&2 echo "gzipped statfile [${statfile}_${last}000.qdf.gz] does not exist"
      fi
    else
      if [ -f ${statfile}_${last}000.qdf ]
      then
        loc_bExistOK=1  
        for t in ${times[@]}
        do
          if [ ! -f  ${popfile}_${t}000.qdf ]
          then
            loc_bExistOK=0
            >&2 echo "gzipped popfile [${popfile}_${t}000.qdf] does not exist"
          fi
        done
      else
        >&2 echo "gzipped statfile [${statfile}_${last}000.qdf] does not exist"
      fi
    fi
  else
    >&2 echo "directory [${maindir}] does not exist"
  fi
  echo $loc_bExistOK
}  


#-----------------------------------------------------------------------------
# gunzipFiles
#   expects params
#     maindir
#     times  (';'-separated three digit numbers, e.g. "000;010;020"
#
function gunzipFiles {
  maindir=$1
  times=$2
  atimes=( ${times//;/  } )
  last=`echo ${atimes[@]} | gawk '{ print $NF }'`
#  >&2 echo "gunzip atimes: [${atimes[@]}]"

  popfile=${maindir}/${popprefix}
  statfile=${maindir}/${statprefix}

#  >&2 echo "gunzipping [${statfile}_${last}000.qdf.gz]"
  gunzip ${statfile}_${last}000.qdf.gz
  for t in ${atimes[@]}
  do
#     >&2 echo "gunzipping [${popfile}_${t}000.qdf.gz]"
     gunzip ${popfile}_${t}000.qdf.gz
  done
}


#-----------------------------------------------------------------------------
# gzipFiles
#   expects params
#     maindir
#     times  (';'-separated three digit numbers, e.g. "000;010;020"
#
function gzipFiles {
  maindir=$1
  times=$2
  atimes=( ${times//;/  } )
  last=`echo ${atimes[@]} | gawk '{ print $NF }'`

#  >&2 echo "gzip atimes: [${atimes[@]}]"
  popfile=${maindir}/${popprefix}
  statfile=${maindir}/${statprefix}

  gzip ${statfile}_${last}000.qdf
  for t in ${atimes[@]}
  do
     gzip ${popfile}_${t}000.qdf
  done
}



#-----------------------------------------------------------------------------
# main
#   expects params
#     mode
#     maindir
#     outbody
#     [outname] (default: "analysis")
#     times  
#

if [ $# -ge 4 ]
then
  mode=$1
  setvars $mode
  if [ $# -ge 5 ]
  then 
    outname=$4
    stimes=$5
  else
    outname=analysis
    stimes=$4
  fi

  times=${stimes// /; }
  atimes=( ${stimes} )

  last=`echo ${atimes[@]} | gawk '{ print $NF }'`
 
  u=(${2//:/ })
  maindir=${u[0]}

  if [ x${u[1]} == xz ]
  then
    bZip=1
  else
    bZip=0
  fi

  if [ ${mode} == cloud ] 
  then
    logdir=${maindir/output//}  
  else
    logdir=${QHG}/app
  fi
  echo "logdir: ${logdir}"
  popfile=${maindir}/${popprefix}
  statfile=${maindir}/${statprefix}
  outtop=$3
  outdir=`echo ${maindir} | gawk -F/ '{ print $NF }'`
  outbody=${outtop}/${outdir}/${outname}

  #----------------------------------
  # check existence of files
  #
  bExistOK=`checkExistence ${maindir} "${times}"`
  #echo "exist: [$bExistOK]"  
  ok=0

  #----------------------------------
  # gunzip files
  #
  if [ $bExistOK -eq 1 ]
  then
    ok=1
    if [ $bZip -eq 1 ]
    then
      echo "existence ok - gunzipping..."
      `gunzipFiles ${maindir} "${times}"`
    else
      echo "existence ok - no gunzipping..."
    fi
  fi

  #----------------------------------
  # create output directory
  #
  if [ $ok -eq 1 ]
  then 
    if [ ! -d ${outtop} ]
    then
      echo "making [${outtop}]"
      mkdir ${outtop}
    fi
    if [ ! -d ${outtop}/${outdir} ]
    then
      echo "making [${outtop}/${outdir}]"
      mkdir ${outtop}/${outdir}
      touch ${outbody}.all.bin
    fi
  fi 


  #----------------------------------
  # QDFSampler loop
  #
  if [ $ok -eq 1 ]
  then 
    echo "QDFSampler"
    for t in ${atimes[@]}
    do
      ${QHG}/genes/QDFSampler -i ${popfile}_${t}000.qdf  -s Sapiens_ooa -o ${outbody}_${t} -f bin:num --location-file=${locfile} -g ${gridfile} -q
      if [ $? -ne 0 ]
      then
        echo "--- QDFSampler failed for $t ----"
        ok=0
      fi
    done
  fi

  #----------------------------------
  # BingGeneMerge
  #
  if [ $ok -eq 1 ]
  then 
    echo "BingGeneMerge"
    ${QHG}/genes/BinGeneMerge2  ${outbody}.all.bin ${outbody}_*.bin
  
    if [ $? -ne 0 ]
    then
      echo "--- BingGeneMerge2 failed ----"
      ok=0
    fi
  fi

  #----------------------------------
  # GeneDist
  #
  if [ $ok -eq 1 ]
  then 
    echo "GeneDist"
    ${QHG}/genes/GeneDist -g ${outbody}.all.bin  -o ${outbody} -m ${statfile}_${last}000.qdf -G ${gridfile}

    if [ $? -ne 0 ]
    then
      echo "--- GeneDist failed ----"
      ok=0
    fi
  else
    ok=1
  fi

  #----------------------------------
  # CalcPCO
  #
  if [ $ok -eq 1 ]
  then 
    echo "CalcPCO"
    lines=`wc -l  ${outbody}.full.mat | gawk '{ print $1 }'`

    # matlab
    # ${QHG}/genes/calcpco_matlab.sh ${outbody}.full.mat ${outbody}.full.pco

    # R
    ${QHG}/genes/calcpco_R.sh ${outbody}.full.mat ${outbody}.full.pco

    # hand made
    # ${QHG}/genes/CalcPCO $lines ${outbody}.full.mat ${outbody}.full.pco
    if [ $? -ne 0 ]
    then
      echo "--- CalcPCO failed ----"
      ok=0
    fi
  fi
 
  #----------------------------------
  # ColumnMerge
  #
  if [ $ok -eq 1 ]
  then 
    echo "ColumnMerge"
    python ${QHG}/genes/ColumnMerger.py ${outbody}.tagdists all ${outbody}.full.pco 1-20 ${outbody}.tab
  fi


  #----------------------------------
  # copy log files
  #
  if [ $ok -eq 1 ]
  then 
    echo "copying logs "
    cp ${maindir}/*.out ${maindir}/*.out ${outtop}/${outdir}
  fi

  #----------------------------------
  # attributes
  #
  if [ $ok -eq 1 ]
  then 
    echo "creating attribute list"
    ${QHG}/useful_stuff/show_attributes all  ${popfile}_${last}000.qdf > ${outbody}.attr
  fi

  #----------------------------------
  # arrival times
  #
  if [ $ok -eq 1 ]
  then 
    echo "arrival times"
    ${QHG}/genes/ArrivalCheck -g ${statfile}_${last}000.qdf -l ${locfile} -n > ${outbody}.arr
  fi


  #----------------------------------
  # gzipping
  #    irrespective of what happened, gzip the files again (if they were gunzipped)
  #
  if [ $bExistOK -eq 1 ] && [ $bZip -eq 1 ]
  then
    echo "gzipping..."
    `gzipFiles ${maindir} "${stimes}"`
  fi


  if [ $ok -eq 1 ]
  then
      echo "+++ success +++"
  else
      echo "--- failure ---"
  fi

else
  if [ $# -ge 1 ]
  then
    mode=$1
    setvars ${mode}
    showvars
    echo "------------------"
  fi
  #----------------------------------
  # usage
  #
  echo "$0 <mode> <maindir>[:z] <outdir> [<outname>] <times>"
  echo "where"
  echo " mode:     machine name: 'triops' | 'kraken' | 'cloud'"
  echo " maindir:  directory containing the [gzipped] pop and stat files"
  echo "           if ':z' is appended, gzipped file are expected"
  echo " outdir:   output directory"
  echo " outname:  output filr prefix (defualt: \"analysis\")"
  echo " times:    quoted list of  3-digit times"
  echo ""
  echo "Examples:"
  echo "$0 ./ all_zombies  \"050 100 150\""
  echo "  apply QDFSampler to pop files of times 50000,100000,150000, then merge the results and apply GeneDist"
  
fi
 
