#!/bin/bash
# Yahav Zarfati
#Check if input is valid
if [ $# -lt 3 ]
then
	echo "Not enough parameters"
	exit
fi
cd $1
#Find in path suitable strings in files
find . -maxdepth 1 -name "*.$2" -exec grep -iw $3 {} \;
