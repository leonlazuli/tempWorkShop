#!/bin/bash -x

#just a playground

read -p "enter a argument > "

case $REPLY in
    *.txt)  
        echo "a txt file name" 
        ;;
    ???)   
        echo "a three character word"
        echo "nothing i just wanna print another line"
        ;;
    *)
        echo "other format word"
        ;;
esac


