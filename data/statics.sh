#! /bin/bash

if [ $# -ne 3 ]
then
   echo "usage $0 [ATOMS] [2CNF|2CNF-2|3CNF-1|3CNF-2|3Horn] lenA"
   exit
fi
# $1: the number of atoms 
# $2: the directory where the date located in
# $3: the size of relevant signature

exec=`pwd`/state-all.sh
#cd $2/$1

for op1 in MIN cd pd 
do
   for op2 in difference time Memory
   do
      $exec $1 $op1 $op2 $3 $2 > $2/$1/$op2-$op1
   done
   if [ $op1 = MIN ]
   then
      $exec $1 $op1 mindiff $3 $2 > $2/$1/mindiff-MIN
   fi
done
