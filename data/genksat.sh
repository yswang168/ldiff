#!/bin/bash
# to generate random CNF clauses
# and to generate A, the relevent set of atoms

# written by Yisong Wang
# 2015.4.23

#
if [ $# -lt 3 ]
then
   echo "$0 K ATOMS CLAUSES [sat] [|A|=15]"
   exit
fi

# $1: K, the length of clauses
# $2: ATOMS, the number of atoms
# $3: the number of clauses
# $4: sat: the generated random theories must be satfied
# $5: |A|: the number random relevant signatures

length=$1
Atoms=$2
Clause=$3
Root=../../../
sat=$Root/../bin/minisat
execs=$Root/../bin/ksat
genatoms=$Root/genratoms.sh

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

if [ $# -ne 5 ]
then
  lenA=15
else
  lenA=$5
fi
i=$lenA 

while [ $i -le $lenA ]
do
  j=1
  while [ $j -le 100 ]
  do
     if [ $# -eq 4 -a $4=sat ]; then
     	gensat3cnf $Atoms $Clause 0 > $i-$j-1
     	gensat3cnf $Atoms $Clause 1 > $i-$j-2
     else
     	$execs $1 $Atoms $Clause 0 > $i-$j-1
     	$execs $1 $Atoms $Clause 1 > $i-$j-2
     fi
     $genatoms $i $Atoms  > $i-$j
     let j=j+1
  done
  let i=i+1
done

