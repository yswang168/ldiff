#!/bin/sh
EXEC=../../../../bin/ldiff
if [ $# -ne 3 ]
then
   echo "$0 cd|pd|MIN|min CLAUSES lenA"
   exit
fi

parm=-$1
resdir=$1
clause=$2

i=$3
rm $resdir -fr
mkdir $resdir

while [ $i -le $3 ]
do
  j=1
  while [ $j -le 100 ]
  do
     echo "$clause-$i-$j-1 $clause-$i-$j-2 $i-$clause-$j" >> $resdir/$i.out
     $EXEC -embed $parm $i-$j-1  $i-$j-2 $i-$j >> $resdir/$i.out 
     let j=j+1
  done
  let i=i+1
done
