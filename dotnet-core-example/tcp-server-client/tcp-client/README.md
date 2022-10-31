TCP Server 的客戶端測試程式提供兩種語言版本。

* C# 版本: tcp-client.csproj, Program.cs
* PHP 版本: tcp-client.php

C# 版本執行範例:

```bash

dotnet run localhost 4321 abc hello world!

```

PHP 版本執行範例:

```bash

php tcp-client.php localhost:4321 abc hello world!

```

它和 [named-pipe-server](https://github.com/shirock/rocksources/tree/master/dotnet-core-example/named-pipe-server) 的 pipe-test.php 只有 socket type 不同。
