#!/bin/bash
#Omer Arad 314096389
#echo "$0 $1 $2 $3"

# check for valid input
if [ -z "$1" ] || [ -z "$2" ]
then
    echo "Not enough parameters"
    exit
fi

# delete compiled files in dir
find "$1" -maxdepth 1 -name '*.out' -delete

# if given flag -r then recurse on all subdirs
if [ "$3" = "-r" ]
then
    for file in "$1"/*
    do
        if [ -d "$file" ]
	then
	    #echo "recurse on $file with $0 $file $2 $3"
	    $($0 "$file" "$2" "-r")
        fi
    done
fi

# compile all files in current folder ($1) that contain given word ($2)
for i in "$1"/*.c
do
    if grep -q -w -i "$2" "$i"; then
        #echo "gcc -g3 -o3 -w $i -o ${i%.c}"
        gcc -g3 -o3 -w "$i" -o "${i%.c}.out"
    fi
done
