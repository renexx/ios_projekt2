#!/bin/bash

# Based on: https://raw.githubusercontent.com/nechutny/IOS/master/proj2/clean.sh
# Improved by: xbolva00

ME="rene"

PROJECT="project2"
if [ ! -z "$1" ];
then
   PROJECT="$1"
fi

IPCS_S=`ipcs -s | egrep "0x[0-9a-f]+ [0-9]+" | grep $ME | cut -f2 -d" "`
IPCS_M=`ipcs -m | egrep "0x[0-9a-f]+ [0-9]+" | grep $ME | cut -f2 -d" "`
IPCS_Q=`ipcs -q | egrep "0x[0-9a-f]+ [0-9]+" | grep $ME | cut -f2 -d" "`

IPCS_S_COUNT=`echo -n "$IPCS_S" | wc -l`
IPCS_M_COUNT=`echo -n "$IPCS_M" | wc -l`
IPCS_Q_COUNT=`echo -n "$IPCS_Q" | wc -l`

PROCESSES_COUNT=`ps aux | grep $ME | grep "^./$PROJECT" | wc -l`


for id in $IPCS_S; do
  ipcrm -s $id;
done

echo "Semaphores ($IPCS_S_COUNT) cleaned..."

for id in $IPCS_M; do
  ipcrm -m $id;
done

echo "Shared ($IPCS_M_COUNT) memory cleaned..."


for id in $IPCS_Q; do
  ipcrm -q $id;
done

echo "Message queues ($IPCS_M_COUNT) cleaned..."

pkill -f "./$PROJECT" 2>/dev/null

echo "Processes ($PROCESSES_COUNT) killed..."
