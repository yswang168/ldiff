#! /bin/bash

# Build up the Horn theories with 50 variables
if [ $# -ne 5 ]
then 
   echo "usage: $0 [atoms] [cd|pd|MIN] [mindiff|difference|Memory|time] [LenA] [dir]"
   exit
fi
atoms=$1
lenA=$4
datadir=$5

declare -a clauses
clauses=(`head -1 $datadir/$1/clauses`)
i=0
rt=`pwd`
while [ $i -lt ${#clauses[@]}  ]
do
   #echo atoms: $1  clauses: ${clauses[$i]} 
   cd $rt/$datadir/$atoms/${clauses[$i]}/$2
   $rt/hstat.sh $atoms ${clauses[$i]}  $3 $2 $lenA
  let i=i+1
done 
