此為「[Raspberry Pi 使用 Grove LoRa 無線模組負擔 LoRa Gateway 角色](http://rocksaying.tw/archives/2018/Raspberry_Pi_use_Grove_LoRa_Radio.html)」的完整範例程式。

本案例於 Raspberry Pi 使用 Grove - LoRa Radio 模組，實作基本的 LoRa 無線應用。 Raspberry Pi 的角色是 LoRa Gateway 。它負責收集其他 LoRa 設備的訊號，再透過 Internet 發佈到 MQTT 頻道上。

除了主程式外，還包含下列工具:

* configure 指令稿: 檢查其他需求套件是否已安裝。
* install.sh 指令稿: 將使用者 pi 加入 dialout 群組。安裝 paho-mqtt 套件。
* pyrfm: Python library for Grove LoRa Radio 。我修改過的 pyrfm ，增加了 `setHeaderTo()` 和 `setHeaderFrom()` 方法。這是 Grove LoRa Radio Arduino 套件已實作，但 pyrfm 原作者未實作的方法。我加以補全。
* lora-sender.py: 示範用 LoRa 發訊端。此發訊端範例使用我修改的 pyrfm 。

主程式 rf95_server.py 可以 root 身份執行，亦可以一般使用者 (pi) 的身份執行。不同使用者身份執行時，其預設的資料目錄也不相同。

root 身份:

* Configuration: /etc/rf95_server.conf
* Log file: /var/log/rf95_server.log
* PID file: /var/run/rf95_server.pid

一般使用者身份:

* Configuration: $HOME/etc/rf95_server.conf
* Log file: $HOME/log/rf95_server.log
* PID file: $HOME/log/rf95_server.pid

rf95_server.conf 內容格式如下:

```
[LoRa]
id = 255
port = /dev/serial0

[MQTT]
host = localhost
topic_base = tw/rocksaying/lora
username = guest
# username should not be empty
password =

[Log]
debug = no
log_dir = /var/log
pid_dir = /var/run
```
