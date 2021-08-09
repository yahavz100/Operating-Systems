#!/bin/bash
# Yahav Zarfati
#Check if input invalid
export LC_ALL=C
if [ $# -lt 4 ]
then
	echo "Not enough parameters"
	exit
fi
#For each directory in path
for i in $(find . -type d | sort)
do	
	#Use ex21 to check strings in directories
	./ex21.sh $i $2 $3 | while read -r line; do
		#Check if output is greater then length
		if [ $(echo $line | wc -w) -ge $4 ]; then
			echo $line
		fi
	done
done
