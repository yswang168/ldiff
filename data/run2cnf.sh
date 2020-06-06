#! /bin/bash

# Build up the 2CNF theories with given [ATOM] variables in give directory [DIR]

# $1: the number of atoms
# $2: the directory
# $3: the size of relevant signature A

if [ $# -lt 2 ]
then
  echo "usage: $0 ATOMS DIR [|A|=15]"
  exit
fi
atoms=$1
# compute the number of clauses

declare -a clauses
MAX=30
for (( i=1; i <= MAX; i++ ))
do
   #clauses[$i]=`(echo "scale=2"; echo $1*$i/5)|bc`
   let clauses[$i]=$1*$i/10
done


rt=`pwd`/$2/$1
if [ ! -d $2 ]; then
	mkdir $2
fi
if [ ! -d $2/$1 ]; then
	mkdir $2/$1
fi
echo ${clauses[*]} > $rt/clauses

if [ $# -ne 3 ]
then
  lenA=15
else
  lenA=$3
fi

clauses=(`head -1 $rt/clauses`) 

execdir=`pwd`
i=0
while [ $i -lt ${#clauses[@]}  ]
do
   rm  $rt/${clauses[$i]} -fr
   if [ ! -d $rt/${clauses[$i]} ]
   then
       mkdir $rt/${clauses[$i]}
   fi
    
   cd $rt/${clauses[$i]}
   $execdir/genksat.sh 2 $atoms ${clauses[$i]} UN $lenA
   $execdir/jisuan.sh cd $atoms-${clauses[$i]} $lenA&
   $execdir/jisuan.sh pd $atoms-${clauses[$i]} $lenA&
   $execdir/jisuan.sh MIN $atoms-${clauses[$i]} $lenA
   
  let i=i+1
done 
