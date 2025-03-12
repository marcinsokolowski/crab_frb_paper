#!/bin/bash

template="*.ar"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

for arfile in `ls ${template}`
do
   dt=`echo $arfile | cut -b 1-10`
   txtfile=${arfile%%ar}txt
   basefile=${arfile%%.ar}
   
   mkdir -p ${dt}
   cd ${dt}
   
   echo "ln -s ../${arfile}"
   ln -s ../${arfile}

   echo "pdmp -dr 0.05 -ds 0.01 -bf -g /png ${arfile}  > $txtfile"
   pdmp -dr 0.05 -ds 0.01 -bf -g /png ${arfile}  > $txtfile

   echo "mv pgplot.png ${basefile}.png"
   mv pgplot.png ${basefile}.png
   
   echo "mv pdmp.best ${basefile}.best"
   mv pdmp.best ${basefile}.best
   cd ..
done
