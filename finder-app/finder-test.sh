#!/bin/sh
# Tester script for Assignment 4
# Author: Siddhant Jajoo, modified for Assignment 4

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data
READDIR=/etc/finder-app
OUTPUTFILE=/tmp/assignment4-result.txt
writer=/usr/bin/writer
finder=/usr/bin/finder.sh

# 檢查配置文件
if [ ! -f "${READDIR}/conf/username.txt" ]; then
    echo "Error: ${READDIR}/conf/username.txt not found"
    exit 1
fi
username=$(cat ${READDIR}/conf/username.txt)
if [ -z "$username" ]; then
    echo "Error: username.txt is empty"
    exit 1
fi

# 處理參數
if [ $# -lt 3 ]; then
    echo "Using default value ${WRITESTR} for string to write"
    if [ $# -lt 1 ]; then
        echo "Using default value ${NUMFILES} for number of files to write"
    else
        NUMFILES=$1
    fi    
else
    NUMFILES=$1
    WRITESTR=$2
    WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

# 創建寫入目錄
rm -rf "${WRITEDIR}"
mkdir -p "$WRITEDIR"
if [ ! -d "$WRITEDIR" ]; then
    echo "Failed to create $WRITEDIR"
    exit 1
fi

# 寫入檔案
for i in $(seq 1 $NUMFILES); do
    echo "Writing $WRITESTR to file ${WRITEDIR}/${username}$i.txt"
    $writer "${WRITEDIR}/${username}$i.txt" "$WRITESTR"
done

# 執行 finder.sh 並保存輸出
OUTPUTSTRING=$($finder "$WRITEDIR" "$WRITESTR")
echo "${OUTPUTSTRING}" > "${OUTPUTFILE}"

# 驗證輸出
set +e
echo "${OUTPUTSTRING}" | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
    echo "success"
    rm -rf "${WRITEDIR}"
    exit 0
else
    echo "failed: expected ${MATCHSTR} in ${OUTPUTSTRING} but instead found"
    rm -rf "${WRITEDIR}"
    exit 1
fi
