* edit /etc/default/grub
* 修改後需執行: `updage-grub`

### 開機強制 check disk.

```text

GRUB_CMDLINE_LINUX="fsck.mode=force"

```

### 取消網路設備重命令行為

保持 eth? 的設備名稱。

```text

GRUB_CMDLINE_LINUX="net.ifnames=0 biosdevname=0"

```
