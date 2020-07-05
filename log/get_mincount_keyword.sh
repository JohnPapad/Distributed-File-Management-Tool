#!/bin/bash

dirName="./"    
    
echo "Log directory: ${dirName}"
printf 'Keyword least frequently found: '

for file in "$( find $dirName -type f)"
do

    cut -d' ' -f7- $file | awk '$1 == "search"' | awk '!a[$0]++' | cut -d' ' -f3- | awk '$NF!="[WORD_NOT_FOUND]"' | awk -F':' '$0=$1 " "NF' | awk 'BEGIN { FS=OFS=" " } {print $1,$2-1}' | awk '{a[$1]+=$2}END{for(i in a) print i,a[i]}' | sort -k2 | head -1 | awk '{print $1,"["$2"]"}'
   
done
