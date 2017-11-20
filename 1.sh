#!/bin/bash
directory="/home/zyyin/splash/"

for file in `ls $directory`
do
        if [ -d $directory/$file ]
        then
                echo $file is dir
        else
                echo $file
		./png2yuv $directory$file
fi
done
