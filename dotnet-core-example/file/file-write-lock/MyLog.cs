using System;
using System.IO;
using System.Threading;

class MyLog
{
    private static string logFile = "";
    private static ReaderWriterLockSlim fileLock = new ReaderWriterLockSlim();
    // https://docs.microsoft.com/en-us/dotnet/api/system.threading.readerwriterlockslim

    public static void Setup(string filepath)
    {
        logFile = filepath;
        var logDir = Path.GetDirectoryName(filepath);
        if (logDir != null && !Directory.Exists(logDir))
        {
            Directory.CreateDirectory(logDir);
        }
    }

    public static void Log(string msg)
    {
        Console.WriteLine(msg);

        // File.AppendAllText 內部會以排他模式開啟。
        // 當多緒同時開啟檔案時，後來者會擲出 IOException。
        // 若不想處理 IOException，則應使用 ReaderWriterLockSlim 保證不漏內容。
        fileLock.EnterWriteLock(); // 後來者將在此排隊等待
        try
        {
            File.AppendAllText(logFile, string.Format("{0} {1}\n", DateTime.Now.ToString("s"), msg));
        }
        catch (IOException e)
        { 
            // 如果未使用 ReaderWriterLockSlim ，則此處有可能擲出 IOException
            Console.WriteLine($"failed to append: {e.Message}");
        }
        finally
        {
            fileLock.ExitWriteLock();
        }
    }
}
