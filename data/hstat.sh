#! /bin/bash 
if [ $# -ne 5 ]; then
echo "usage: $0 [atoms] [clauses]  [time|Memory|difference] [cd|pd|MIN] [LenA]"
exit
fi

num=100   # groups
# total=`(echo 'scale=2'; echo "$2/$1")|bc`
# The below is for Horn theories with variable-length-model
total=`(echo 'scale=2'; echo "$2/(2^$1)")|bc`
a=0
t1=""
t2=""

function CheckLines() # with the metric: Memory, time, difference and mindiff
{
	line=`grep $2 $1.out | wc -l `
	if [ $line -ne $num ]; then
   		echo "The number of $1.out contains $2: $line line, while the expected is $num"
   		exit
	fi
}
start=$5
signum=$5
# statics the metrics of time, memory, difference of logical difference
# for each length $i of relevant signature
case $3 in
Memory)
	for(( i=$start; i<=$signum; i++)) 
	do
   		CheckLines $i $3
		grep Memory $i.out > mem
		cut -d: -f2 mem > tmem
# (echo '1,$s/MB//g'; echo "wq")|ed -s tmem
  		a=0
 	 	while read cur
  		do
     		a=`(echo scale=2; echo $a + $cur)|bc`
  		done < tmem
  		total="$total \t&`(echo 'scale=2'; echo "$a/$num")|bc`"
	done
	echo -e $total
	;;

time)
	for(( i=$start; i<=$signum; i++)) 
	do
		CheckLines $i $3
		grep time $i.out > time
		cut -d: -f2 time > ttime
  		a=0
  		while read cur
  		do
     			a=`(echo 'scale=2'; echo $a+$cur)|bc`
  		done < ttime
  		total="$total \t&`(echo 'scale=2'; echo "$a/$num")|bc`"
	done
	echo -e $total
	;;
difference)
	for(( i=$start; i<=$signum; i++)) 
	do
		#CheckLines $i length
		declare -a num_diff
	    	grep length $i.out  | cut -f2-20 -d":" > tnd
       		for (( j=0; j<=$signum; j++ ))
		do
			let k=$j+2
			cut -d" " -f$k tnd | cut -d: -f2 > xtnd
			a=0
  			while read cur 
  			do
    	 			a=`expr $a + $cur`
			done < xtnd
			num_diff[$j]=`(echo scale=4; echo $a/$num)|bc`
       		#total="$total \t&`(echo 'scale=1'; echo "$a/$num")|bc`"
     			t1="$t1 %8.4f "
       			t2="$t2 ${num_diff[$j]} "
		done
		CheckLines $i diff	
		# compute difference and removed minimal clauses
		grep difference $i.out > xx
		cut -d: -f2 xx > tnd
		a=0
		while read cur
		do
		   	a=`expr $a + $cur`	
		done < tnd
		let j=j+1
		num_diff[$j]=`(echo scale=4; echo $a/$num)|bc`
     		t1="$t1 %8.4f "
       		t2="$t2 ${num_diff[$j]} "
		# for removed minimal clauses	
		if [ $4 = MIN ]; then
			grep difference xx |grep \( | cut -d: -f3 > tnd
  			(echo "1,\$s/(//g"; echo "1,\$s/)//g"; echo "wq")|ed -s tnd 2 >& /dev/null
			a=0
			while read cur
			do
		   		a=`expr $a + $cur`	
			done < tnd
			let j=j+1
			num_diff[$j]=`(echo scale=4; echo $a/$num)|bc`
     			t1="$t1 %8.4f "
       			t2="$t2 ${num_diff[$j]} "
		fi
  	done
	printf $total
  	printf "$t1\n" $t2
	;;
esac

# remove all the immediate files
rm -fr nd xx tnd xtnd mem tmem time ttime

