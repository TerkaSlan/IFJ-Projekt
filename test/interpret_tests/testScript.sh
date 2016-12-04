#!/usr/bin/env bash
# FILES="`pwd`/test/interpret_tests/*/*.code"
FILES="*.code"
i=0
j=0
for f in $FILES
do
	echo "======================================="
	FILENAME=`echo $f | sed -E 's/^.*\///'`
	RETVAL=`echo $FILENAME | cut -c 1`
	./../../interpret $f > log/$FILENAME.out 2> log/$FILENAME.stderr
	REALRETVAL=`echo $?`
	if [ "$RETVAL" -eq "$REALRETVAL" ];then
		DIFF=`diff log/$FILENAME.out $FILENAME.out`
		if [ "$DIFF" != "" ]
			then
			echo "FAIL"
			head -1 $f
			echo $FILENAME
			echo "Differences in output:"
			# echo $DIFF
			# cat $FILENAME.out
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
		echo "Our retval: `echo $?`"
		i=$((i+1))
	fi	

	# ./../../interpret $f >> $f.out 2>> $f.stderr
	# REALRETVAL=0
	# REALRETVAL=`echo $?`
	# echo "---------------------------"

	# if [ "$RETVAL" -eq "$REALRETVAL" ];then
	# 	echo "SUCCESS"
	# 	echo $f | sed -E 's/^.*\///'
	# else
	# 	echo "FAIL"
	# 	head -1 $f
	# 	echo $f | sed -E 's/^.*\///'
	# 	echo "Our retval: `echo $?`"
	# fi
done

echo "=================================================="
echo "=================================================="
echo "Count of fails: $i"
echo "Count of successes: $j"