#!/bin/bash
# to generate random Horn clauses
# and to generate A, the relevent set of atoms

# written by Yisong Wang
# 2015.4.23

#
if [ $# -ne 3 ]
then
   echo "$0 K ATOMS CLAUSES"
   exit
fi
length=$1
Atoms=$2
Clause=$3
#sat=minisat
execs=../bin/hksat
genatoms=./genratoms.sh

function gensat3cnf()
{
   if [ $# != 3 ]
   then
     echo "usage: $execs [atoms] [clauses] [ID]"
     exit
   fi
   tmp=`cat /dev/urandom | head -n 10 | md5sum | head -c 10`
   res=`cat /dev/urandom | head -n 10 | md5sum | head -c 10`
   while [ 1 -eq 1 ]
   do 
   $execs $length $1 $2 $3 > $tmp

   $sat $tmp  -verb=0 2>/dev/null > $res
   unsat=`grep "UNSAT" $res | wc -l`
   if [ $unsat -eq 0 ]
   then
     break
   fi
   done 
   cat $tmp
   rm -fr $tmp $res
}

i=1
while [ $i -le 10 ]
do
  j=1
  while [ $j -le 100 ]
  do
     gensat3cnf $Atoms $Clause 0 > $i-$j-1
     gensat3cnf $Atoms $Clause 1 > $i-$j-2
     $genatoms $i $Atoms  > $i-$j
     let j=j+1
  done
  let i=i+1
done

