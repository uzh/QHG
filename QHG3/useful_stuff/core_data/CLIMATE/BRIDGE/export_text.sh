#!/bin/bash

name=$1
outname=${name%".nc"}.txt
ncdump -l 8192 -v longitude,latitude,temp_mm_srf,precip_mm_srf,snowdepth_mm_srf $name | sed -e '1,/data:/d' -e '$d'  > $outname
