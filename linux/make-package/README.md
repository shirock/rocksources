# make-package

套件打包工具。目前只會打包 DEB 套件。

* make-package 配合 Python 3.x 。
  * 套件需求: python3, coreutils, findutils, file, tar, dpkg, python3-setuptools
* make-package2 配合 Python 2.x 。
  * 套件需求: python (>= 2.6), coreutils, findutils, file, tar, dpkg, python-distutils

> distutils 已於 Python 3.12 廢棄。
> Debian 安裝套件 python3-setuptools 取代。

## 用法

```bash
make-package [arguments ...] [commands ...]
```

### 指令 commands

* show-package-name
  顯示套件名稱。套件名稱應記載於 *README* 的 *Package* 欄位。
* show-package-version
  顯示套件版本。套件版本應記載於 *README* 的 *Version* 欄位。
* show-arch
  顯示作業系統架構。amd64 或 i386 。
* show-revision
  若處於版本管理工具管理下，顯示目前的版本 revision 。支援 subversion 和 git 。
* generate
  產生一組用於打包套件的資訊檔案。目前有 2 個檔案：
  1. README: 套件描述資訊，如名稱、版本號碼、依賴套件等。
  2. INSTALL-rule: 打包動作檔。
* debian
  打包 DEB 套件。
* dist
  打包所有可用套件。目前只支援 DEB 套件，所以只會打包 DEB 套件。

make-package 的設計初衷是簡化 DEB 套件打包工作。主要概念繼承 DEB 套件管理規則。DEB 套件管理規則請看 [Debian 新維護人員手冊](https://www.debian.org/doc/manuals/maint-guide/index.zh-tw.html)。

#### 範例

將 generate 和 dist 兩個指令連用，讓 make-package 打包自己。

```bash
make-package generate debian
```

### 參數 arguments

* --directory=where
  指定工作目錄。此目錄下應有 *README* 和 *INSTALL-rule* 。預設是當前目錄。
* --dest=where
  打包好的套件要放置的目錄。預設位置是在工作目錄的 *dist* 子目錄。
* --head=file
  指定套件標頭檔。預設是 *README* 。
* --rule=file
  指定打包動作檔。預設是 *INSTALL-rule* 。

## 套件打包資訊檔案

### INSTALL-rule

你可以把 INSTALL-rule 視為 make-package script 。
它的語法參考 shell script ，但指令(動作)名稱不一樣。

#### 概觀

* 每行開頭使用 `#` 或 `;` 標示的皆視為註解。
* 支援變數置換。例如:  
  WWWDIR=/var/www  
  pack homepage $WWWDIR/service
* 打包 DEB 套件時，若有檔案要複製到套件控制目錄內，請用 *$CONTROLS* 表示控制目錄，而不是直接寫 'DEBIAN' 。例如:  
  copy after-remove.sh $CONTROLS/postrm
* DEB 套件自動處理規則 (你不需要寫):
  * 它會複製 ChangeLog 為 DEBIAN/changelog 。
  * 若是工作目錄、'data' 子目錄、'scripts' 子目錄中包含 conffiles/preinst/prerm/postinst/postrm 這些檔案，它會複製到 DEBIAN 目錄。
  * 它會複製 install.sh 為 DEBIAN/postinst ，複製 uninstall.sh 為 DEBIAN/prerm 。

#### 打包動作

make-package 打包時，會在工作目錄下建立一個 'dist' 子目錄作為打包區，放置需要打包的檔案內容。

所有打包動作的目的地，皆以 'dist' 為根目錄。例如下列動作並不會將檔案複製到系統的 /etc/myprogram.ini ，而是複製到 dist/etc/myprogram.ini 。

```bash
copy etc/settings.ini /etc/myprogram.ini
# it will copy etc/settings.ini to dist/etc/myprogram.ini
```

##### pack directory \[dist_target_directory\]

將目錄內容整個複製到打包區。範例:

```bash
DIST=/usr/local
PROG=my-program
pack docs $DIST/share/doc/$PROG
pack images $DIST/share/images/$PROG
```

##### mkdir directory

在打包區建立目錄，可建立多層目錄。範例:

```bash
mkdir /var/log/my-program
```

##### copy file dist_target \[mode\]

複製檔案到打包區。 mode 是目的檔案屬性，可選。範例:

```bash
copy my-program /usr/bin/ 775
copy data/readme /usr/doc/my-program.txt
copy 
```

##### chmod mode file

設定打包區檔案的屬性。DEB 包會記錄檔案屬性，可執行檔或 shell script 之類的，都應該設定。範例:

```bash
chmod 775 /usr/local/bin/my-script
```

##### remove dist_file

刪除打包區中的指定檔案。

##### replace dist_file \[from\] \[to\]

替換打包區指定檔案的文字內容。不指定 from 和 to 時，它會按照目前的環境變數替換檔案內的文字。範例:

```bash
replace /usr/local/doc/readme @user User
```

##### symlink target link_name

在打包區建立符號連結。

### README

README 的檔案格式與內容，基本上就是參考 [Debian 新維護人員手冊: control](https://www.debian.org/doc/manuals/maint-guide/dreq.zh-tw.html#control)。

#### 重要欄位說明

* Package
  套件名稱。
* Version
  套件版本號碼。內容可以使用 `%r` 代入 revision。例如 `1.0.%r`。
* Description
  套件的簡短說明。
* Architecture
  套件適用機器架構。如果是 Python/shell scripts 之類，可用 *all* 。
  其他情形請省略此欄位。由 make-package 依建置機器的架構決定。
* Depends
  此套件依賴的其他套件清單。可省略。
* Section
  此套件的分類組別。可省略。
* 會自動計算需求空間，並加入 'Installed-Size' 欄位。
