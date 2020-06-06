#! /usr/bin/bash

# generate the random relevant sigantures
# $1: the number of random relevant propositional variables
# $2: the total number atoms 

declare -a rand
i=0
while [ $i -lt $1 ]
do
    rand[$i]=`expr $RANDOM % $2 + 1`
    j=0
    while [ $j -lt $i ]
    do
	if [ ${rand[$i]} -eq ${rand[$j]} ] 
	then
	   rand[$i]=`expr $RANDOM % $2 + 1`
	   j=0
	   continue
	fi     
	let j=j+1
    done
    let i=i+1
done

echo ${rand[*]}
