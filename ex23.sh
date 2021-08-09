#!/bin/bash
# Yahav Zarfati
#Declare flags
HOST=0
SYSTEM=0

STATIC=0
ICON=0
MACHINE=0
BOOT=0
VIRTUALIZATION=0
KERNEL=0
ARCHITECTURE=0

NAME=0
VERSION=0
PRETTY_NAME=0
HOME=0
SUPPORT=0
#Check for invalid input
if [ $# == 0 ]; then
	echo "Invalid input"
	exit
fi

#Check if host argument
if [ $1 == "host" ]; then
	HOST=1
	#Check if inserted only 1 argument
	if [ $# == 1 ]; then
		cat hostnamectl
	else
		#Switch case for each flag
		for arg in "$@"
		do
			case $arg in
				--static_hostname)
				#Check if flag is already used
				if [ $STATIC -eq 0 ]; then
					grep -w "Static hostname" hostnamectl | cut -d ":" -f2 | cut -d " " -f2
					STATIC=1
				else
					continue
				fi
				shift
				;;
				--icon_name|--name)
				#Check if flag is already used
				if [ $ICON -eq 0 ]; then
					grep -w "Icon name" hostnamectl | cut -d ":" -f2 | cut -d " " -f2
					ICON=1
				else
					continue
				fi
				shift
				;;
				--machine_id)
				#Check if flag is already used
				if [ $MACHINE -eq 0 ]; then
					grep -w "Machine ID" hostnamectl | cut -d ":" -f2 | cut -d " " -f2
					MACHINE=1
				else
					continue
				fi
				shift
				;;
				--boot_id)
				#Check if flag is already used
				if [ $BOOT -eq 0 ]; then
					grep -w "Boot ID" hostnamectl | cut -d ":" -f2 | cut -d " " -f2
					BOOT=1
				else
					continue
				fi
				shift
				;;
				--virtualization)
				#Check if flag is already used
				if [ $VIRTUALIZATION -eq 0 ]; then
					grep -w "Virtualization" hostnamectl | cut -d ":" -f2 | cut -d " " -f2
					VIRTUALIZATION=1
				else
					continue
				fi
				shift
				;;
				--kernel)
				#Check if flag is already used
				if [ $KERNEL -eq 0 ]; then
					grep -w "Kernel" hostnamectl | cut -d ":" -f2 | sed 's/ //'
					KERNEL=1
				else
					continue
				fi
				shift
				;;
				--architecture)
				#Check if flag is already used
				if [ $ARCHITECTURE -eq 0 ]; then
					grep -w "Architecture" hostnamectl | cut -d ":" -f2 | cut -d " " -f2
					ARCHITECTURE=1
				else
					continue
				fi
				shift
				;;
			esac
		done
		#Check if inserted invalid flags
		if [ $HOST -eq 1 ] && [ $STATIC -eq 0 ] && [ $ICON -eq 0 ] && [ $MACHINE -eq 0 ] && [ $BOOT -eq 0 ] && [ $VIRTUALIZATION -eq 0 ] && [ $KERNEL -eq 0 ] && [ $ARCHITECTURE -eq 0 ]; then
			cat hostnamectl
		fi
	fi
	exit
fi

#Check if system argument
if [ $1 == "system" ]; then
	SYSTEM=1
	#Check if inserted only 1 argument
	if [ $# == 1 ]; then
		cat os-release
	else
		#Switch case for each flag
		for arg in "$@"
		do
			case $arg in
				--name)
				#Check if flag is already used
				if [ $NAME -eq 0 ]; then
					grep -w "NAME" os-release | cut -d "=" -f2 | sed 's/"//g'
					NAME=1
				else
					continue
				fi
				shift
				;;
				--version)
				#Check if flag is already used
				if [ $VERSION -eq 0 ]; then
					grep -w "VERSION" os-release | cut -d "=" -f2 | sed 's/"//g'
					VERSION=1
				else
					continue
				fi
				shift
				;;
				--pretty_name)
				#Check if flag is already used
				if [ $PRETTY_NAME -eq 0 ]; then
					grep -w "PRETTY_NAME" os-release | cut -d "=" -f2 | sed 's/"//g'
					PRETTY_NAME=1
				else
					continue
				fi
				shift
				;;
				--home_url)
				#Check if flag is already used
				if [ $HOME -eq 0 ]; then
					grep -w "HOME_URL" os-release | cut -d "=" -f2 | sed 's/"//g'
					HOME=1
				else
					continue
				fi
				shift
				;;
				--support_url)
				#Check if flag is already used
				if [ $SUPPORT -eq 0 ]; then
					grep -w "SUPPORT_URL" os-release | cut -d "=" -f2 | sed 's/"//g'
					SUPPORT=1
				else
					continue
				fi
				shift
				;;
			esac
		done
		#Check if inserted invalid flags
		if [ $SYSTEM -eq 1 ] && [ $NAME -eq 0 ] && [ $VERSION -eq 0 ] && [ $PRETTY_NAME -eq 0 ] && [ $HOME -eq 0 ] && [ $SUPPORT -eq 0 ]; then
			cat os-release
		fi
	fi
	exit
fi

echo "Invalid input"
