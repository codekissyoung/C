#!/bin/bash

# cnt=10;

# for i in 9 8 7 6 5 4 3 2 1; do
#     echo $i;
#     sleep 1;
# done

clear;
for t in "Wake up" "The Matrix has you" "Follow the white rabbit" "Knock, knock";do
clear;
pv -qL10 <<< $'\e[2J'$'\e[32m'$t$'\e[37m';
sleep 5;
done