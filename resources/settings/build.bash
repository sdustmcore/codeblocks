#!/bin/bash

test -d output || mkdir output

echo "cleaning output directory..."
rm output/*.png
cd png

#for i in `ls | grep .png`; do convert -strip $i $i ; done

echo " "
echo "stripping images..."
for i in `ls | grep .png`; do echo -n "["$i"] " ; convert -strip $i "../output/"$i ; done

cd ../output

echo " "
echo " "
echo "generating desaturated versions..."

for i in `ls | grep .png`; do echo -n "["$i"] " ; convert -strip -antialias -fx "(r+b+g)/2.9" -gaussian 0x0.8 $i `echo $i | sed -e "s/\.png/-off\.png/"`; done

