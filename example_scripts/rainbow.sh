#!/bin/bash
if [ $# -ne 1 ]; then echo "usage: ./rainbow.sh infile.bmp"; exit 1; fi;
if [ -f bow.bmp ]; then echo "delete or move bow.bmp before running"; exit 1; fi
if [ -f tmp.pql ]; then echo "delete or move tmp.pql before running"; exit 1; fi

echo "SELECT WHERE 1; OPERATE SET R = R/2; OPERATE SET G = G/2; OPERATE SET B = B/2;" >> tmp.pql
echo "SELECT WHERE row >= HEIGHT/6*0 AND row < HEIGHT/6*1; OPERATE SET R = OUT.R+(255/2);" >> tmp.pql
echo "SELECT WHERE row >= HEIGHT/6*1 AND row < HEIGHT/6*2; OPERATE SET R = OUT.R+(255/2); OPERATE SET G = OUT.G+(255/4);" >> tmp.pql
echo "SELECT WHERE row >= HEIGHT/6*2 AND row < HEIGHT/6*3; OPERATE SET R = OUT.R+(255/2); OPERATE SET G = OUT.G+(255/2);" >> tmp.pql
echo "SELECT WHERE row >= HEIGHT/6*3 AND row < HEIGHT/6*4; OPERATE SET G = OUT.G+(255/2);" >> tmp.pql
echo "SELECT WHERE row >= HEIGHT/6*4 AND row < HEIGHT/6*5; OPERATE SET B = OUT.B+(255/2);" >> tmp.pql
echo "SELECT WHERE row >= HEIGHT/6*5 AND row < HEIGHT/6*6; OPERATE SET R = OUT.R+(255/2); OPERATE SET B = OUT.B+(255/2);" >> tmp.pql

pixql -i $1 -o bow.bmp -qf tmp.pql

rm tmp.pql

