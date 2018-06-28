#!/bin/bash

clim=CLIMATE
tool=./
icogrid=${HOME}/QHG3/resources/grids/GridSG_ieq_128.qdf
basegrid=WorldMap_ieq_128.qdf
waelbroeck=waelbroeck2002_table.txt
outdir=./grids

if [ ! -f ${basegrid} ]; then
    cp ${icogrid} ${basegrid}
    ${tool}/bedrock2grid.py ETOPO1_Bed_g_gmt4.grd ${basegrid}
fi

if [ ! -d $outdir ]; then
    mkdir $outdir
fi

i=120
while (( $i > -1 )); do
    echo $i
    qdf=world_${i}kya.qdf
    if [ ! -f ${outdir}/${qdf} ]; then 
        cp $basegrid ${outdir}/${qdf}
        cd ${outdir}
        ${tool}/set_sea_level.py ../$waelbroeck $i ${qdf}
        if (( $i >= 27 )); then
            ${tool}/add_ice_peltier.py ../CLIMATE/peltier_ice5g_122_0_ka/ice5g_v1.2_122_27.nc $i ${qdf}
        else
            ${tool}/add_ice_peltier.py ../CLIMATE/peltier_ice5g_122_0_ka/ice5g_v1.2_26_0.nc $i ${qdf}
        fi
        ${tool}/kernel_interp.py ../CLIMATE/BRIDGE/${i}ka.ann.nc ${qdf}
        cd ..
        if (( $i < 17 )); then
            # extra 0.5 step for ice maps
            j=`awk -v N=$i 'BEGIN { print N+0.5 }'`
            echo $j
            qdf=world_${j}kya.qdf
            if [ ! -f ${outdir}/${qdf} ]; then
                cp $basegrid ${outdir}/${qdf}
                cd ${outdir}
                ${tool}/set_sea_level.py ../$waelbroeck $j ${qdf} # use j^th sea level
                ${tool}/add_ice_peltier.py ../CLIMATE/peltier_ice5g_122_0_ka/ice5g_v1.2_26_0.nc $j ${qdf}  # use j^th ice map
                ${tool}/kernel_interp.py ../CLIMATE/BRIDGE/${i}ka.ann.nc ${qdf}  # still use i^th climate (no 0.5 steps in BRIDGE)
                cd ..
            fi
        fi
    fi
    if (( $i > 80 )); then
        i=`expr $i - 4`
    elif (( $i > 22 )); then 
        i=`expr $i - 2`
    else 
        i=`expr $i - 1`
    fi

done


