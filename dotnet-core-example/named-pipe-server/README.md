注意，自 .NET6 起，使用 unix domain socket 實作 NamedPipe 類別，而不是用 FIFO 。

這個程式需要示範 .NET 的 NamedPipe 類別在不同程式語言之間的互通能力，
所以提供了一個 PHP 的測試用客戶端程式。

但因為 PHP 的 Windows 版本並不支援 unix domain socket ，
故這個 PHP 客戶端程式只能在 Linux 下使用。

在 Linux/MacOS 平台，建立 unix domain socket 時的讀寫權限，預設是 644 (srwx-r-xr-x)。
表示不同 UID 的程序不能開啟這個 socket 。
可以用 chmod 修改 socket 的讀寫權限，讓其他使用者的程序也能用。

.NET6 還沒有可跨平台改變 unix socket 讀寫權限的方法。
