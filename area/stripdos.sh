#! /bin/sh
ls *.are *.txt | while read line
do
echo $line
cat $line > /tmp/stripdos.tmp
fromdos < /tmp/stripdos.tmp > $line
done
