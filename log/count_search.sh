#!/bin/bash

dirName="./"        

echo "Log directory: ${dirName}"
printf 'Total number of keywords searched: '

for file in "$( find $dirName -type f)"
do
    grep -o " : search : " $file | wc -l
done
