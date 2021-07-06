# 加入 DLL 資源參考

這裡說的 DLL 資源是指用 .NET Framework 開發的項目。

現在大多數 .NET 程式開發資源，會打包成 nuget 套件供程式開發人員使用。
但總有一些程式開發資源只提供了 DLL 檔案，而沒有正式打包 nuget 套件。

*dotnet* 指令工具沒有對應的命令項目可用於加入 DLL 資源。
你得要自己編輯專案檔(.csproj) ，手動加入下列參考項目。

```xml

  <ItemGroup>
    <Reference Include="????">
      <HintPath>path-to-????.dll</HintPath>
    </Reference>
  </ItemGroup>

```

若這項 DLL 資源還參考其他套件，也要加入專案檔。

例如本範例要加入 XP3620Library.dll 。假設 XP3620Library.dll 放在專案子目錄 dlls 中。
那專案檔就加入下列項目:

```xml

  <ItemGroup>
    <Reference Include="XP3620Library">
      <HintPath>dlls\XP3620Library.dll</HintPath>
    </Reference>
  </ItemGroup>

```

且 XP3620Library 還參考了 System.IO.Ports 套件 (.NET 並不內含此套件)，
那就要再執行 `dotnet add package System.IO.Ports` 。

dotnet build 時，會自動把 XP3620Library.dll 也複製到執行檔目錄內。
