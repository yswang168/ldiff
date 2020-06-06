# compute the satisfiability of the theories in $1/$2, the file is named in $3-$i-1 (i\in [1,00])
#! /bin/bash
# $1 Directory
# $2 Atoms
# $3 LenA

if [[ $# != 3 ]]; then
	echo "Usage $0 Director Atoms LenA"
	exit
fi
declare -a clauses

rt=$1/$2
clauses=(`head -1 $rt/clauses`)
mc="2^$2"  #$((2**$2))
SAT=minisat-2.2
i=0
while [ $i -lt ${#clauses[@]}  ]
do
  cdir=$rt/${clauses[$i]}
  if [ -f $cdir/sat ]; then
	  rm $cdir/sat
  fi
  touch $cdir/sat
  for ((j=1; j<101; j++))
  do
    echo $3-$j-1 >> $cdir/sat
    $SAT -verb=0 $cdir/$3-$j-1 >> $cdir/sat 
    $SAT -verb=0 $cdir/$3-$j-2 >> $cdir/sat 
  done
  # statics the number of SAT instances
  msat=`grep ^SAT $cdir/sat | wc -l`
  ration=`(echo scale=2; echo $msat/200) | bc`
  echo `(echo scale=2; echo ${clauses[$i]}/$mc)|bc`  ${clauses[$i]}  $msat  $ration
  let i=$i+1
done
