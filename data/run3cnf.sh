#! /bin/bash


if [ $# != 2 ]
then
  echo "usage: $0 ATOMS DIR"
  exit
fi
atoms=$1
# compute the number of clauses

declare -a clauses
#:<<!
MAX=40
i=1
while (( i <= 40 ))
do
   clauses[$i]=`(echo "scale=2"; echo $1*$i/5)|bc`
   let clauses[$i]=$1*$i/5
   let i=i+1
done
echo ${clauses[*]} > clauses
#!

if [ ! -d $2 ] 
then
   mkdir $2
fi

rt=`pwd`/$2
#clauses=(`head -1 $rt/clauses`) 

execdir=./
i=0
while [ $i -lt ${#clauses[@]}  ]
do
#:<<!
   rm  $rt/${clauses[$i]} -fr
   if [ ! -d $rt/${clauses[$i]} ]
   then
       mkdir $rt/${clauses[$i]}
   fi
#!    

   cd $rt/${clauses[$i]}
   $execdir/genksat.sh 3 $atoms ${clauses[$i]}
   $execdir/jisuan.sh $atoms-${clauses[$i]} &
   $execdir/jisuan.sh pd $atoms-${clauses[$i]} &
   $execdir/jisuan.sh MIN $atoms-${clauses[$i]} 
   
  let i=i+1
done 
