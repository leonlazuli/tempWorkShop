#!/bin/bash 

#just a playground

for i in $(ls); do
    ls $i
done

echo "--------------"

for (( i=0; i<5; i=i+1)); do
    echo $i;
done
