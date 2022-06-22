// See https://aka.ms/new-console-template for more information

MyLog.Setup("r:/log/testlog.txt");

MyLog.Log("Hello world");

var t1 = new Thread(()=>{
    Thread.Sleep(10);
    for (var i = 0; i < 10; i++)
    {
        MyLog.Log($"t1 {i}");
        Thread.Sleep(10);
    }
});

var t2 = new Thread(()=>{
    for (var i = 0; i < 10; i++)
    {
        MyLog.Log($"t2 {i}");
        Thread.Sleep(10);
    }
});

t1.Start();
t2.Start();
