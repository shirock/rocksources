#!/bin/sh
# 在桌面環境執行需要 sudo 的 shell script 時，出現 GUI 的密碼輸入視窗。
# 程式執行結果也會用 GUI 訊息視窗顯示。
#
# Required: policykit-1 zenity

pkexec apt-get install xxxx
if [ $? -eq 0 ]; then
    zenity --info --text="$0成功"
else
    zenity --error --text="$0失敗"
fi