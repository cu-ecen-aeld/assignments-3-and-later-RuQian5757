#!/bin/sh

#Tester script for assignment 1, 2, and 4

#Modified for Buildroot environment

set -e set -u

NUMFILES=10 WRITESTR=AELD_IS_FUN WRITEDIR=/tmp/aeld-data WRITER=/usr/bin/writer FINDER=/usr/bin/finder.sh CONF_DIR=/etc/finder-app/conf USERNAME=$(cat $CONF_DIR/username.txt)

if [ $# -lt 3 ] then echo "Using default value ${WRITESTR} for string to write" if [ $# -lt 1 ] then echo "Using default value ${NUMFILES} for number of files to write" else NUMFILES=$1 fi else NUMFILES=$1 WRITESTR=$2 WRITEDIR=/tmp/aeld-data/$3 fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

#檢查必要文件和目錄

if [ ! -x "$WRITER" ]; then echo "Error: writer executable not found at $WRITER" exit 1 fi

if [ ! -x "$FINDER" ]; then echo "Error: finder.sh not found at $FINDER" exit 1 fi

if [ ! -f "$CONF_DIR/username.txt" ]; then echo "Error: username.txt not found at $CONF_DIR/username.txt" exit 1 fi

#創建 WRITEDIR

rm -rf "${WRITEDIR}" mkdir -p "$WRITEDIR"

if [ -d "$WRITEDIR" ] then echo "$WRITEDIR created" else echo "Error: Failed to create $WRITEDIR" exit 1 fi

#寫入測試文件

for i in $( seq 1 $NUMFILES ) do $WRITER "$WRITEDIR/${USERNAME}$i.txt" "$WRITESTR" done

#運行 finder.sh 並將輸出寫入 /tmp/assignment4-result.txt

OUTPUTSTRING=$($FINDER "$WRITEDIR" "$WRITESTR") echo "$OUTPUTSTRING" > /tmp/assignment4-result.txt


#檢查輸出文件是否生成
if [ ! -f /tmp/assignment4-result.txt ]; then echo "Error: Failed to write output to /tmp/assignment4-result.txt" exit 1 fi


#驗證 finder.sh 輸出
set +e echo "$OUTPUTSTRING" | grep "${MATCHSTR}" if [ $? -eq 0 ]; then echo "Success: Finder output matches expected string" exit 0 else echo "Failed: Expected '${MATCHSTR}' in output but found '$OUTPUTSTRING'" exit 1 fi
