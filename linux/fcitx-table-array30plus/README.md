fcitx 行列30字根表添加一級簡碼
==============================

fcitx-table-array30 所採用的字根表中，包含行列30特別碼以及1996年新增特別碼。但不含一、二級簡碼和 w 特殊符號。

一級簡碼的使用率實在很高，沒它很難用。所以我將以前設計 TouchIME 時整理的行列30簡碼表拿出來，與 fcitx-table-array30 的字根合併。
**All the data file is now in public domain.**

不過我在合併過程中，發現一些問題，所以我最後只加入一級簡碼，放棄二級簡碼。

使用說明
--------

合併程式：[https://github.com/shirock/rocksources/tree/master/linux/fcitx-table-array30plus](https://github.com/shirock/rocksources/tree/master/linux/fcitx-table-array30plus)。

先執行 merge.py 將簡碼表和字根表合併，產生 array30plus.txt 。
再執行 fcitx-tools 的 txt2mb 轉換字根表。

安裝方式，參考 [在Linux Mint上安裝自家輸入法，從cin格式轉到fcitx的mb碼表](https://blog.fat-nerds.com/dot-nerd/linux-mint-fcitx-chinese-customized-input-table-cin-mb/) 。我將轉換後的字根表放在 ~/.config/fcitx/table/ 。順便移除了 fcitx-table-array30 套件。

```shell
mkdir ~/.config/fcitx/table
cp array30plus.conf ~/.config/fcitx/table/
cp array30plus.mb ~/.config/fcitx/table/
```

字根表來源
----------

* [array30.txt 取自 fcitx-table-extra](https://github.com/fcitx/fcitx-table-extra/tree/master/tables)
* [行列30簡碼表 ar30_hw.txt ，取自 TouchIME](https://github.com/shirock/rocksources/tree/master/web/touch-ime/array30)
* [行列小站](http://www.array30.com/)

放棄二級簡碼的原因
------------------

因為二級簡碼收錄的字都有特別碼，重複了。而且二級簡碼會把沒用到的位置用 '□' 佔位。
當 fcitx 轉換字根表時，會將同碼的重複字清除(包含'□')，也就不能保證這些字在候選字清單的順序，破壞了二級簡碼規則。

二級簡碼是兩個字根再加數字鍵選字。而特別碼是兩個字根再加空白鍵(轉換鍵)。
在輸入效率上，特別碼比較理想，所以放棄加入二級簡碼。

捨棄一級簡碼的'隨'
------------------

一級簡碼的'隨'和 特殊碼'的'的位置重複了。取'的'捨'隨'。

作為踢出一級簡碼位置的補償，我把'隨'加到它首二碼的候選字清單。

基本上，每個一級簡碼的第一個候選字都剛好是單字根字。唯有'隨'字例外。
而 fcitx 的預設出字規則是轉換鍵後，預設出第一個候選字。
按這個規則，'的'特別碼剛好和'隨'的一級簡碼出字順序相同，都是 T 加空白鍵。
