if [ $# -ge 2 ]
then
    mat_file=$1
    out_file=$2
    echo "applying 'cmdscale' to $mat_file; writing result to $out_file"
    unset DISPLAY
    matlab >> matlob.log 2>&1 << EOF

      m=dlmread('$mat_file');
      Y=cmdscale(m);
      dlmwrite('$out_file', Y, ' ');
      exit 
EOF
else
    echo "$0 <mat-file> <out-file>"
fi

