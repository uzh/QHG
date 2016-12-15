if [ -z $PROG_DIR  ] 
then
  PROG_DIR=.
fi

if [ -z $DATA_DIR  ]
then
  DATA_DIR=../resources
fi

if [ -z $UTIL_DIR ]
then
  UTIL_DIR=../import
fi

if [ -z $QMAP_DIR ]
then
  QMAP_DIR=../qmaps
fi

if [ -z $OUT_DIR  ]
then
  OUT_DIR=testout
fi
rm -rf $OUT_DIR
mkdir $OUT_DIR


red='\e[31;48m'
green='\e[32;48m'
blue='\e[34;48m'
#black='\e[30;48m'
black='\e[39;49;00m'

echo progdir: $PROG_DIR
echo datadir: $DATA_DIR
echo utildir: $UTIL_DIR
echo qmapdir: $QMAP_DIR

GRID_DIR=$DATA_DIR/grids
CLIM_DIR=$DATA_DIR/climate
BIOM_DIR=$DATA_DIR/biome

#europe: 
#GRID=$GRID_DIR/eu_2m_f.qmap
#world
GRID=$GRID_DIR/wwwf8A_2.qmap

#now:
TEMP=$CLIM_DIR/cai_temp2.clim
RAIN=$CLIM_DIR/cai_precip2.clim

temptodaymap=$CLIM_DIR/temp_ann_001k.qmap 
raintodaymap=$CLIM_DIR/rain_ann_001k.qmap 
calcclim=1
#calcclim=1

  
REQ_TOOL="$QMAP_DIR/QSmooth $PROG_DIR/TempRainBiomizer $PROG_DIR/BiomassDistribution $PROG_DIR/TempRainCapacity $PROG_DIR/QGrowthTest"
REQ_DATA="$GRID $TEMP $RAIN $BIOM_DIR/lholdag_f.qmap $BIOM_DIR/qhg_biomes.qmap $BIOM_DIR/qhg_perc_all.txt $BIOM_DIR/GCEPMasses.txt $BIOM_DIR/trans_LH2QHG.txt"

#----------------------------------------------------------------------------
# checking for required files
#----------------------------------------------------------------------------
echo -e  "${blue}>>> checking for required files${black}"
ok=1
for f in $REQ_TOOL
do
  echo -n "checking $f: " 
  if [ -x $f ]
  then
      echo -e "${green}ok${black}"
  else 
      ff=`echo $f | gawk -F/ '{ print $NF }'`
      dd=`echo $f | sed "s#$ff##g"` 
      echo -e "${red}ERR: $ff not in [$dd]${black}"
      ok=0;
  fi
done
for f in $REQ_DATA
do
  echo -n "checking $f: " 
  if [ -r $f ]
  then
      echo -e "${green}ok${black}"
  else
      ff=`echo $f | gawk -F/ '{ print $NF }'`
      dd=`echo $f | sed "s#$ff##g"` 
      echo -e "${red}ERR: $ff not in [$dd]${black}"
      ok=0;
  fi
done

if [ $ok -eq 0 ]
then 
  echo -e "${red}Can't continue because of missing tools or files${black}"
  exit 1
fi

echo -e "${green}All required files found${black}"

#names of vegetation items
NAMES=( Grass Bush Tree )

# standard growth times for 25% of capacity at 15°C
TIMES=( 60 120 360 )

#----------------------------------------------------------------------------
# prepare temp & rain files
#----------------------------------------------------------------------------
# only do lengthy stuff if required 
if [ ! -f qtoday_trb.qmap ]
then

  if [ $calcclim -ne 0 ]
  then
    echo -e  "${blue}>>> create an altitude corrected temperature map for the world${black}"
    $UTIL_DIR/UDel2QMap $TEMP $OUT_DIR/temp_today.qmap -p 7c
    $PROG_DIR/TopoTemper -ag $GRID $OUT_DIR/temp_today.qmap $OUT_DIR/tempcorr_today.qmap
    temptodaymap=$OUT_DIR/tempcorr_today.qmap

    if [ $? != 0 ]
    then
      exit
    fi

    echo
    echo -e "${blue}>>> create a rain map with same detail as temperature map${black}"
    $UTIL_DIR/UDel2QMap $RAIN $OUT_DIR/precip_today.qmap -p 7c
    $QMAP_DIR/QScale $OUT_DIR/precip_today.qmap $GRID $OUT_DIR/precipcorr_today.qmap
    raintodaymap=$OUT_DIR/precipcorr_today.qmap   

    if [ $? != 0 ]
    then
      exit
    fi
  fi
#----------------------------------------------------------------------------
#  create  trb table
#----------------------------------------------------------------------------
  echo
  echo -e "${blue}>>> create a trb table from global data for qhg biomes (non-corrected temp & rain)${black}"
  $PROG_DIR/TempRainBiomizer -t -r --temp-file=$temptodaymap \
                           --prec-file=$raintodaymap        \
                           --biom-file=$BIOM_DIR/lholdag_f.qmap    \
                           --temp-bin=0.125 --prec-bin=1           \
                           --trans-file=$BIOM_DIR/trans_LH2QHG.txt \
                           -o $OUT_DIR/qtoday -v  

  if [ $? == 0 ]
  then
    echo -e "${green}trb table successfully created${black}"
  else
    echo -e "${red}TempRainBiomzer failed${black}"
    exit
  fi
fi


#----------------------------------------------------------------------------
#  create biomes for today
#----------------------------------------------------------------------------
echo
echo -e "${blue}>>> recreate biomes for today:${black}"
 
$PROG_DIR/TempRainBiomizer  -b --trb-table=$OUT_DIR/qtoday_trb.qmap  \
                       --temp-file=$temptodaymap   \
                       --prec-file=$raintodaymap   \
                       -o $OUT_DIR/qbiomes_today -v  


  if [ $? == 0 ]
  then
    echo -e "${green}successfully created biomes for today${black}"
  else
    echo -e "${red}TempRainBiomzer failed${black}"
    exit
  fi

#----------------------------------------------------------------------------
#  create biomass densities for today
#----------------------------------------------------------------------------
echo
echo -e "${blue}>>> ++ create biomasses for current day biomes:${black}"
echo -e "${blue}>>> ++  uses a global biome map and total masses to calculate specific masses${black}"
echo -e "${blue}>>> ++  uses a local biome map to calculate masses for that area${black}"
$PROG_DIR/BiomassDistribution -b $BIOM_DIR/qhg_biomes.qmap           \
                      -o $OUT_DIR/dens_today -p $BIOM_DIR/qhg_perc_all.txt    \
                      -f None -g $BIOM_DIR/GCEPMasses.txt            \
                      -t $OUT_DIR/qbiomes_today_bio.qmap -T density


  if [ $? == 0 ]
  then
    echo -e "${green}successfully created biomasses for today${black}"
  else
    echo -e "${red}BiomassDistribution failed${black}"
    exit
  fi

#----------------------------------------------------------------------------
#  smoothing density maps
#----------------------------------------------------------------------------
echo
echo -e "${blue}>>>++ smooth the maps a bit:${black}"
for f in $OUT_DIR/dens_today* ; do   $QMAP_DIR/QSmooth -i $f  -o ${f%%.qmap}S.qmap -k 3 -s 9x9 -r 1; done

#++ single ones:
# $QMAP_DIR/QSmooth -i mass_Grass.qmap  -o mass_GrassS.qmap -k 3 -s 9x9 -r 1

if [ $? != 0 ]
then
  exit
fi

#----------------------------------------------------------------------------
#  create temp-rain-capacity map
#----------------------------------------------------------------------------
echo
echo -e "${blue}>>>++ create temp-rain-capacity maps:${black}"
jobs=""
for f in $OUT_DIR/dens_today*S.qmap ; 
  do  
  echo "processing $f"
  $PROG_DIR/TempRainCapacity -t $temptodaymap \
      -p $raintodaymap -o ${f%%.qmap} -c $f \
      --temp-bin=0.1 --prec-bin=10 -r 5 -k 1.1 
  if [ $? != 0 ]
  then
    echo -e "${red}TempRainCapacity failed${black}"
    exit
  fi

done

echo -e "${green}successfully created biomasses for today${black}"

if [ $? == 0 ]
then
  echo -e "${green}successfully created temp-rain-capacity maps${black}"
fi

#----------------------------------------------------------------------------
#  smoothing averages & std devs
#----------------------------------------------------------------------------
echo
echo -e "${blue}>>>++ smoothe averages:${black}"
for f in $OUT_DIR/dens_today*avgc.qmap ; 
  do 
  $QMAP_DIR/QSmooth -i $f -o ${f%%.qmap}S.qmap -k 3 -s 11x11 -r 1; 

  if [ $? != 0 ]
  then
    echo -e "${red}smoothing averages failed${black}"
    exit
  fi
done

if [ $? == 0 ]
then
  echo -e "${green}successfully smoothed averages${black}"
fi

echo
echo -e "${blue}>>>++ smooth standard devs:${black}"
for f in $OUT_DIR/dens_today*stdc.qmap ; 
  do 
  $QMAP_DIR/QSmooth -i $f -o ${f%%.qmap}S.qmap -k 3 -s 21x21 -r 1; 

  if [ $? != 0 ]
  then
    echo -e "${red}smoothing standard devs failed${black}"
    exit
  fi

done

if [ $? == 0 ]
then
  echo -e "${green}successfully smoothed standard devs${black}"
fi


#----------------------------------------------------------------------------
#  create biomasses for current day
#----------------------------------------------------------------------------
# the following lines are used to calculate average growth rates for the given
# calculated masses
echo
echo -e "${blue}>>> ++ create biomasses for current day biomes:${black}"
echo -e "${blue}>>> ++  uses a global biome map and total masses to calculate specific masses${black}"
echo -e "${blue}>>> ++  uses a local biome map to calculate masses for that area${black}"
$PROG_DIR/BiomassDistribution -b $BIOM_DIR/qhg_biomes.qmap           \
                      -o $OUT_DIR/mass_today -p $BIOM_DIR/qhg_perc_all.txt    \
                      -f None -g $BIOM_DIR/GCEPMasses.txt            \
                      -t $OUT_DIR/qbiomes_today_bio.qmap -T mass


if [ $? == 0 ]
then
  echo -e "${green}successfully created biomasses for current day biomes${black}"
else
  echo -e "${red}BiomassDistribution failed${black}"
  exit
fi  
echo ">>>++ calc growth rates"

#----------------------------------------------------------------------------
#  create growth files
#----------------------------------------------------------------------------
echo
echo -e "${blue}>>> ++ create growth files${black}"
i=-1
while [ $? == 0 ] && [ $i -lt 2 ]
do
  let i=$i+1
  $PROG_DIR/QGrowthTest $OUT_DIR/mass_today_${NAMES[$i]}.qmap $temptodaymap $OUT_DIR/g${NAMES[$i]}.qmap ${TIMES[$i]}
done

if [ $i -lt 2 ]
then 
  exit
fi

echo "# growth rates" > $OUT_DIR/GrowthRates.txt
for h in ${NAMES[@]}
do
  a=`$QMAP_DIR/QStat -s $OUT_DIR/g$h.qmap | gawk '{ print $2 }'`; 
  b=`$QMAP_DIR/QStat -s $OUT_DIR/area.qmap | gawk '{ print $2 }'`; 
  c=`$QMAP_DIR/div $a $b  86400`; 
  echo $h: $c;
  echo $h: $c >> $OUT_DIR/GrowthRates.txt;
done



if [ $? == 0 ]
then
  echo -e "${green}+++ complete success +++${black}"
else
  echo -e "${red}--- terminating with a fault ---${black}"
fi

