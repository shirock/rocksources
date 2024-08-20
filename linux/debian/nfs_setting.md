NFS
=====

Server
------

edit /etc/exports:

```text
/home/user/Public  0.0.0.0/24(rw,no_root_squash,no_subtree_check,sync)
```

* no_root_squash: 允許 root 可以直接存取，影響 sudo 之類的指令。

reload:

```term
# exportfs -ar
```

check:

```term
# exportfs -v
```

Client
----------

/home/user/Public 掛載自 NFS

```term
# mount -t nfs -o rsize=32768,wsize=37268 192.168.1.1:/home/user/Public /home/user/Public
```

/etc/fstab:

```text
192.168.1.1:/home/user/Public /home/user/Public  nfs  defaults,_netdev  0  0
```

read size buffer and write size buffer.
default size 1024

actimeo=n
– 將 actimeo 設定為 n 秒相當於將 acdirmax、acdirmin、acregmax 和 acregmin 設定為 n 秒。
– 對於許多 NFS 用戶端經常使用和修改的目錄，設定 actimeo=1 或 actimeo=3。 這可以確保檔案和目錄屬性保持合理的最新狀態，即使它們在不同的用戶端位置經常發生變更。
