#!/usr/bin/env bash

FILES="tests/*.code"
i=0
j=0

if [ ! -d "tests/log" ]; then
  mkdir tests/log
fi

for f in $FILES
do
	echo "======================================="
	FILENAME=`echo $f | sed -E 's/^.*\///'`
	RETVAL=`echo $FILENAME | cut -c 1`
	./interpret $f > tests/log/$FILENAME.out 2> tests/log/$FILENAME.stderr
	REALRETVAL=`echo $?`
	if [ "$RETVAL" -eq "$REALRETVAL" ];then
		DIFF=`diff tests/log/$FILENAME.out tests/$FILENAME.out`
		if [ "$DIFF" != "" ]
			then
			echo "FAIL"
			head -1 $f
			echo $FILENAME
			echo "Differences in output:"
			echo $DIFF
			i=$((i+1))
		else
			echo "SUCCESS"
			echo $FILENAME
			j=$((j+1))
		fi
	else
		echo "FAIL"
		head -1 $f
		echo $FILENAME
		echo "Our retval: `echo $REALRETVAL`"
		i=$((i+1))
	fi	
done

echo "=================================================="
echo "=================================================="
echo "Count of fails: $i"
echo "Count of successes: $j"
