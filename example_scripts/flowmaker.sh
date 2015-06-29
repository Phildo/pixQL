#!/bin/bash

if [ $# -ne 1 ]; then echo "usage: ./flowmaker.sh infile.bmp"; exit 1; fi;
if [ -f flow.gif ]; then echo "delete or move flow.gif before running"; exit 1; fi
if [ -f tmp.pql ]; then echo "delete or move tmp.pql before running"; exit 1; fi
if [ -f tmp_n.pql ]; then echo "delete or move tmp_n.pql before running"; exit 1; fi
for i in {0..59}; do
  if [ -f out_${i}.bmp ]; then echo "delete or move out_${i}.bmp before running"; exit 1; fi
done

touch flow.gif

echo "BLACK; SELECT WHERE 1; OPERATE SET COLOR = RED;" >> tmp.pql
echo "SELECT WHERE (R+X)%60<50; OPERATE SET COLOR = #FF9900FF;" >> tmp.pql
echo "SELECT WHERE (R+X)%60<40; OPERATE SET COLOR = #FFFF00FF;" >> tmp.pql
echo "SELECT WHERE (R+X)%60<30; OPERATE SET COLOR = #00FF00FF;" >> tmp.pql
echo "SELECT WHERE (R+X)%60<20; OPERATE SET COLOR = #0000FFFF;" >> tmp.pql
echo "SELECT WHERE (R+X)%60<10; OPERATE SET COLOR = #FF00FFFF;" >> tmp.pql

for i in {0..59}; do
  sed "s/X/$i/g" tmp.pql > tmp_n.pql
  pixql -i $1 -o out_${i}.bmp -qf tmp_n.pql
done

rm tmp.pql
rm tmp_n.pql

rm flow.gif
convert -delay 10 -loop 0 out_{0..59}.bmp flow.gif
rm out_{0..59}.bmp

