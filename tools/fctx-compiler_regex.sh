#!/bin/bash
# Generate regex for fctx-compiler from languages.c data
# fctx-compiler file -r [,.0-9:\;A-Za-z일ÁăầÂÅäÄĄČÇéÉĚĒĞÍİÑÓÖØŘŚŠŞÚÛÜŪýŹžŽאבגדהוטילמנספץצקרשΑΒβΓΔδΕεΙΪΚΛΜΝΟΠΡΣΤΥΦАБВГдДеЕжЖиИІЙКЛМнНОПРСТУФЧЮЯ一三二五六周四土日月木水火週金]
# fctx-compiler file -r [0-9:\ ]
# Version 0.1 - 29/12/2017

input_file=$1
tmp_file="/tmp/$$.tmp"
output_file="./output.txt"

cat $input_file | while read line ; do
    r=1
    tmp=$(echo $line | awk '{print substr($0,'"$r"',1)}')


    while [ "$r" -le "${#line}" ]; do
        echo "$tmp" >> $tmp_file
        r=$((r+1))
        tmp=$(echo $line | awk '{print substr($0,'"$r"',1)}')
    done
done
sort -u $tmp_file | tr -d '\n' > $output_file
echo  >> $output_file
rm $tmp_file
