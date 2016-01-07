#!/bin/bash -x

#just a playground

DELAY=3;

while true; do
    clear;
    cat <<-EOF
        Please Select:
        1.display system info 
        2.display disk space
        3.display home space utilization
        0.quit
EOF
    echo "there"
    read -p "enter selection [0-3] > "
    if [[ $REPLY =~ ^[0-3]$ ]]; then
        if [[ $REPLY == 1 ]]; then
            echo "Hostname: $HOSTNAME"
            uptime
        elif [[ $REPLY == 2 ]]; then
            df -h
        elif [[ $REPLY == 3 ]]; then
            du -sh $HOME
        else
            break;
        fi
    else
        echo "Invalid entry."
    fi
    
    sleep $DELAY
        
    
done
