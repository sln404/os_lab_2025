#!/bin/bash

if [ $# -eq 0 ]; then
    echo "No arguments given."
    exit 1
fi

SUM=0
CNT=$#

for ARG in "$@"; do
    SUM=$(echo "$SUM + $ARG" | bc)
done

AVG=$(echo "scale=2; $SUM / $CNT" | bc)

echo "Number of arguments: $CNT"
echo "Average value: $AVG"
