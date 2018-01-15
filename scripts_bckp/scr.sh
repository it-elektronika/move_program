#!bin/bash

threshold=$(du -b "/home/pi/touch.txt" | cut -f 1)
bool=true

while $bool;do

  actualsize=$(du -b "/home/pi/touch.txt" | cut -f 1)
  echo $actualsize

  if [ $actualsize -gt $threshold ]; then
    echo "condition fullfiled"
    #pkill -f evtest
    #rm touch.txt
    bool=false
  fi
done

if ! $bool;then
  pkill -f evtest
  rm touch.txt
  cd move_program
  ./move_prog
  #rm /home/pi/touch.txt
fi

