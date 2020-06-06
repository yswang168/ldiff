#! /bin/bash

# Build up the Horn theories with 50 variables

if [ $# != 1 ]
then
  echo "usage: $0 ATOMS"
  exit
fi
atoms=$1
# compute the number of clauses

declare -a clauses
# comment the following block
:<<!
MAX=40
for (( i=1; i <= MAX; i++ ))
do
   let clauses[$i]=$1*$i/5
done
echo ${clauses[*]} > clauses
!


rt=`pwd`/3Horn
clauses=(`head -1 $rt/clauses`) 

execdir=./
i=0
while [ $i -lt ${#clauses[@]}  ]
do
#   
#   rm  $rt/${clauses[$i]} -fr
#   if [ ! -d $rt/${clauses[$i]} ]
#   then
#       mkdir $rt/${clauses[$i]}
#   fi
#!   
   cd $rt/${clauses[$i]}
#   $execdir/genhksat.sh 3 $atoms ${clauses[$i]}
   $execdir/jisuan.sh $atoms-${clauses[$i]} &
   $execdir/jisuan.sh pd $atoms-${clauses[$i]} &
   $execdir/jisuan.sh MIN $atoms-${clauses[$i]} 
   
  let i=i+1
done 
