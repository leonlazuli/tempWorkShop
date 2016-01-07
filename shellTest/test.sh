#!/bin/bash 

#just a playground

foo=3;

if (( foo > 1 && 3 >= 3 )); then
    $(( foo = foo * 2))
    echo $foo $boo
fi
