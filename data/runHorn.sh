#! /bin/bash

# Build up the Horn theories with given [ATOM] variables in give directory [DIR]

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


rt=`pwd`/$2/$1

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
    
   cd $rt/${clauses[$i]}
   $execdir/jisuan.sh cd $atoms-${clauses[$i]} $lenA&
   $execdir/jisuan.sh pd $atoms-${clauses[$i]} $lenA&
   $execdir/jisuan.sh MIN $atoms-${clauses[$i]} $lenA
   
  let i=i+1
done 
