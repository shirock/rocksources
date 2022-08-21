using System;
using System.IO;
using System.Threading;

class MyLog
{
    private static string logFile = "";
    /*
    ReaderWriterLockSlim 或 ReaderWriterLock 僅為同程序之多執行緒的資源共用鎖 (multi-threading only)。
    不能用於不同程序之間的資源共用情形 (not for multi-processing)。
    要在不同程序間共用資源，需要使用具名機制，例如 named system semaphore 或 named mutex.
    https://docs.microsoft.com/en-us/dotnet/api/system.threading.semaphore
    */
    private static ReaderWriterLockSlim fileLock = new ReaderWriterLockSlim();
    // private static ReaderWriterLock fileLock = new ReaderWriterLock();
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
        // fileLock.AcquireWriterLock(3000); // ReaderWriterLock
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
            // fileLock.ReleaseWriterLock(); // ReaderWriterLock
        }
    }
}
