#!/bin/bash

# NOTE- REQUIRES ImageMagick
# Bands of color traverse the image filling in their portion

if [ $# -ne 1 ]; then echo "usage: ./filler.sh infile.bmp"; exit 1; fi;
if [ -f fill.gif ]; then echo "delete or move fill.gif before running"; exit 1; fi
if [ -f tmp.pql ]; then echo "delete or move tmp.pql before running"; exit 1; fi
if [ -f tmp_n.pql ]; then echo "delete or move tmp_n.pql before running"; exit 1; fi
for i in {0..63}; do
  if [ -f out_${i}.bmp ]; then echo "delete or move out_${i}.bmp before running"; exit 1; fi
done

touch fill.gif

echo "BLACK;" >> tmp.pql
echo "SELECT WHERE R <= (4*X)+3; OPERATE SET R = R;" >> tmp.pql
echo "SELECT WHERE G <= (4*X)+3; OPERATE SET G = G;" >> tmp.pql
echo "SELECT WHERE B <= (4*X)+3; OPERATE SET B = B;" >> tmp.pql
echo "SELECT WHERE R >= (4*X) AND R <= (4*X)+3; OPERATE SET R = 255;" >> tmp.pql
echo "SELECT WHERE G >= (4*X) AND G <= (4*X)+3; OPERATE SET G = 255;" >> tmp.pql
echo "SELECT WHERE B >= (4*X) AND B <= (4*X)+3; OPERATE SET B = 255;" >> tmp.pql

for i in {0..63}; do
  sed "s/X/$i/g" tmp.pql > tmp_n.pql
  pixql -i $1 -o out_${i}.bmp -qf tmp_n.pql
done

rm tmp.pql
rm tmp_n.pql

rm fill.gif
convert -delay 10 -loop 0 out_{0..63}.bmp fill.gif
rm out_{0..63}.bmp

