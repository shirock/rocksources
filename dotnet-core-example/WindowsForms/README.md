# Windows Forms 程式範例

.NET Core 3.0 開始支援 Windows Forms 應用程式開發工作。
此類專案不具跨平台性，只限 Windows 作業系統可用。
基本上， Windows 7 以上版本都可以執行。

```shell

$ dotnet new winforms --name [YourProjectName]

```

cproj 專案檔基本要素：

```xml

<Project Sdk="Microsoft.NET.Sdk">

  <PropertyGroup>
    <OutputType>WinExe</OutputType>
    <TargetFramework>net8.0-windows</TargetFramework>
    <UseWindowsForms>true</UseWindowsForms>
  </PropertyGroup>

</Project>

```

dotnet -f FRAMEWORK 與 專案檔中的 TargetFrameworks:

cproj 專案檔可以使用 TargetFrameworks 元素列出多項可支援架構。
但此時 dotnet 必須加上 -f 參數指定實際執行架構。

注意拼字，此處的 TargetFrameworks 是複數形。

例如:

cproj 專案檔:

```xml

<Project Sdk="Microsoft.NET.Sdk">

  <PropertyGroup>
    <OutputType>WinExe</OutputType>
    <TargetFrameworks>netcore3.1;net6.0-windows;net8.0-windows</TargetFrameworks>
    <UseWindowsForms>true</UseWindowsForms>
  </PropertyGroup>

</Project>

```

dotnet 建置時，要指定 -f 參數，且指定的執行架構必須列在 TargetFrameworks 之中。

```shell

# 可用，目標電腦需具備 .NET Core 3.1 Runtime
dotnet build -f netcore3.1

# 可用，目標電腦需具備 .NET 8 Desktop Runtime
dotnet build -f net8.0-windows

# 不可用，因為 TargetFrameworks 沒有列入 net7.0-windows
dotnet build -f net7.0-windows

```
