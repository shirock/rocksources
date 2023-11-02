磁碟分割
========

默認 SSD 。

分割表
------

| 設備 | 標記  | 掛載點    | 容量      | 檔案系統 | 說明       |
|------|-------|-----------|-----------|----------|------------|
| sda1 | ESP   | /boot/efi | 100 MB    | FAT32    | EFI 保留區 |
| sda2 | Boot  | /boot     | 250~512 MB| Ext2     | 只留最新版 |
| sda3 | Swap  | swap      | 1~4 GB    | swap     | Swap       |
| sda4 | Root  | /         | > 10 GB   | Ext4     | 單用戶系統 |

默認 SSD ，所以不考慮為 home, var 另外配置分割區。

安裝在虛擬機時，虛擬機磁碟映像檔最小應有 *12 GB*。

ESP
----

就算用 BIOS/MBR 開機，也應該保留 ESP 分割區。
日後需要轉換到 UEFI/GPT 時，不用切出新分割區。

ESP (EFI 保留區)，既然 Windows 10 安裝時也只會建 *100 MB*，那就 100MB 。
它的檔案系統格式規定為 *FAT32*。不能放 LVM。

Boot
----

/boot 放置 Linux kernel image。一份 kernel image 大約 50~70 MB。
每當系統更新核心套件時，這裡就會增加一份 kernel image。
若使用者忘了移除舊版本的核心，那麼 /boot 的可用空間就會愈來愈少。
通常只保留最新版核心，要刪除舊的。

/boot 用 Ext2 純粹是因為它幾乎沒有寫入需求。
Debian 引導安裝時，/boot 會配置 *512 MB*。

/boot 單獨配置分割區還有一個原因，方便日後使用 LVM 或 RAID。
在開機管理程式中，大概只有 grub 認得 LVM。

Swap
----

Swap 的使用彈性很大。Debian 引導安裝時，會配置 1 GB。
擺在 /boot 後面，日後可以從 Swap 挪出空間給 /boot。

Root
----

檔案系統格式沒有限制，預設 Ext4。用 Brtfs 或配合 LVM 也行。

Debian 9 ~ 12 安裝下列桌面環境的基本使用空間。

* LXDE, LXQT : 7 GB
* GNOME, KDE : 8 ~ 9 GB

分割區至少要留 25% 可用空間，故 root 分割區最小要 *10 GB*。
Ubuntu 安裝需求再加 20%。

RAID
----

安裝時就決定用 RAID 的話，在選擇分割區這一步時，必須選手動(Manual)，才會看到 Software RAID 項目。
而且安裝時就要準備兩個磁碟。因為安裝程式一定要選兩個分割區才能跑下去。
用同一個磁碟的不同分割區做 RAID 沒意義，不討論。
