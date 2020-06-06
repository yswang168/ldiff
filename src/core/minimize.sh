#!/bin/bash

# To compute the minimal difference of $1 from a tautology cnf

if [ $# != 1 ]; then
  echo "usage: $0 KB1";
  exit;
fi

runsat="minisat -verb=0 "
suffix=".cnf"
bminiSAT="SAT"
bminiUNSAT="UNSATISFIABLE"

i=1
lines=`wc -l $1|cut -f1 -d" "`
while [ $i -le $lines ]; 
do
  line=`head -$i $1 | tail -1`
  cnffile=xxx$i$suffix
  cp $1 $cnffile -fr
  (echo "$i c"; echo "c $line"; echo "."; echo "wq") | ed -s $cnffile

  for lit in $line
  do
    if [ $lit -ne 0 ]; then
       let nlit=-1*$lit
       echo "$nlit 0 " >> $cnffile
    fi
  done 

  bsat=`$runsat $cnffile 2>/dev/null`
  if [[ $bsat == $bminiUNSAT ]]; then
     (echo $i c; echo "c $line"; echo "."; echo "wq") | ed -s $1
     echo $i: $line
  fi
  echo "c $bsat" >> $cnffile
  let i=i+1
done
