BASE=~/progs/QHG3/trunk/useful_stuff
DATA_TRIOPS=${BASE}/core_data
DATA_KRAKEN=~/kraken/qhgtests/src/QHG3/useful_stuff/core_data
g0=output/ooa_pop-Sapiens_ooa__070000.qdf

for f in /home/jody/kraken/Simulations/init_2*
do
  g=$f/$g0
  stat $g 2>1 > /dev/null
  if [ $? -eq 0 ]
  then
    echo "processing: $g"

    p2=`echo ${g%%.qdf}.png| gawk -F/ '{ print $NF }'`

    ${BASE}/LinkSGCqdf.py $g ${DATA_TRIOPS}/GridSGC_ieq_256.qdf
    ${BASE}/patagoniacheck.py $g Sapiens_ooa_Number $p2
  
    mv $p2 $f
    display $f/$p2 &

    ${BASE}/LinkSGCqdf.py $g ${DATA_KRAKEN}/GridSGC_ieq_256.qdf
    
  fi
done