#!/bin/sh

if [ $# -ge 2 ]
then
    mat_file=$1
    out_file=$2
    echo "applying 'R cmdscale' to $mat_file; writing result to $out_file"

    echo "options(echo = FALSE)"             > temp.R 
    echo "mydata=read.table(\"$mat_file\")" >> temp.R
    echo "x=cmdscale(mydata, k=10)"         >> temp.R
    echo "xa=t(x)"                          >> temp.R
    echo "write(xa, file=\"$out_file\", ncolumns=10)" >> temp.R
    

    R CMD BATCH temp.R temp.out
    
    # rm temp.R temp.out
else
    echo "$0 <mat-file> <out-file>"
fi

