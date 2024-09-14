Openbox 訂製環境
================

為了提供用戶登入畫面，需要安裝一種顯示管理器 (Display Manager)，例如 lightdm, sddm 或 lxdm 。

基本安裝 openbox obconf obmenu 。

以 Openbox 作為 LxQt, KDE 等 Qt 基礎桌面環境的視窗管理程式時，推薦加裝 obconf-qt 。

安裝 pkexec polkitd，不用裝 policykit-1 。
如果只安裝 Openbox 而未安裝其他任何桌面環境時，可能不會一併安裝 Policy Kit。
這影響需要系統權限才能工作的 GUI 程式，例如 synaptic 。

針對中文使用環境: https://www.rocksaying.tw/archives/31556973.html 。

* 個人用戶組態: ~/.config/openbox/rc.xml
* 系統組態: /etc/xdg/openbox/rc.xml

參考資料:

* https://wiki.archlinux.org/title/Openbox
* https://openbox.org/help/Contents

桌面工具程式
------------

非必要套件。依同類功能條列。多選一安裝即可。

可以從 Xfce, LXDE, LXQt, Mate 這些桌面環境中挑選偏好的工具程式。

* xterm, pterm, stterm, zutty, terminus, lxterminal, qterminal, mate-terminal, konsole, gnome-terminal
* tint2, lxpanel
  工作條 (taskbar)。
* thunar, pcmanfm, pcmanfm-qt, caja, tuxcmd, gnome-commander, krusader
  檔案管理員。
* mousepad, geany, featherpad, pluma, tea, gedit, kate
  純文字編輯器。
* xarchiver, lxqt-archiver, engrampa, ark, file-roller
  壓縮檔工具。
* xfce4-taskmanager, lxtask, qps, mate-system-monitor
  工作管理員。
* synaptic
  系統套件管理員。
* xscreensaver, light-locker, i3lock, mate-screensaver
  桌面鎖定程式。
* network-manager-gnome, cmst
  網路組態管理員 (GUI前端)。依據你偏好的網路組態管理員而定，network-manager 或 connman。
* x11-utils, suckless-tools
  工具 xwininfo, xporp, etc.
* xfonts-utils
* hsetroot, nitrogen
  桌面背景產生器。
* compton
  視窗特效合成器。

開始功能表 (menu)
-----------------

Openbox 的任何空白區域，按下滑鼠右鍵叫出開始功能表 (應用程式選單)。

* 系統的開始功能表: /etc/xdg/openbox/menu.xml
* 個人用戶的開始功能表: ~/.config/openbox/menu.xml

通常不用個人用戶的開始功能表。按 XDG 規範行為，透過套件安裝 GUI 程式之後，就會加進系統的開始功能表。
但個人用戶的開始功能表不會變更，需要手動修改。

滑鼠操作視窗
------------

滑鼠指標放在程式視窗任何位置皆可，不用按在視窗標題列。

* 按住 Alt+滑鼠左鍵，移動滑鼠: 拖曳程式視窗位置。
* 按住 Alt+滑鼠右鍵，移動滑鼠: 改變程式視窗大小。

自動啟動 (Autostart)
--------------------

自動啟動，寫在 script: ~/.config/openbox/autostart 。
執行程式時，列尾記得加 & 符號 (背景執行)。
範例:

~~~sh
# 迷你工作列 (如果安裝了)
tint2 &
~~~

系統範圍: /etc/xdg/openbox/autostart 。

鍵盤快捷鍵 (keyboard shortcut)
------------------------------

Debian 的 Openbox 套件預設快捷鍵，常用:

1. Window+D: 叫出桌面。
   未使用桌面管理員的情形，此動作會隱藏所有程式視窗，露出 Openbox 空白畫面。接著，可以按滑鼠右鍵叫出開始功能表。
2. Alt+Tab, Alt+Shift+Tab: 切換程式視窗。
3. Alt+F4: 關閉目前程式視窗。
4. Alt+Space: 顯示目前程式的視窗功能選單。
5. Print: 擷取螢幕畫面 (執行 scrot)。

自訂快捷鍵的方式，編輯 ~/.config/openbox/rc.xml 的 keyboard 區段，增加或修改 keybind 項目。

範例: 離開 Openbox (logout)
----

若是透過顯示管理器啟動，此行為等同登出。

按鍵 Ctrl+Alt+Escape 登出:

~~~xml
   <keybind key="C-A-Escape">
     <action name="Exit">
       <prompt>yes</prompt>
     </action>
   </keybind>
~~~

或者綁命令列指令 `openbox --exit`，但這種方法不會出現確認畫面:

~~~xml
   <keybind key="C-A-Escape">
     <action name="Execute">
       <command>openbox --exit</command>
     </action>
   </keybind>
~~~

範例: 執行終端機程式
----

按鍵 Ctrl+Alt+T 執行終端機程式:

~~~xml
   <keybind key="C-A-t">
     <action name="Execute">
       <command>x-terminal-emulator</command>
     </action>
   </keybind>
~~~
