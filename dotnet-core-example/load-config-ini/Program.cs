using System;

namespace Example
{
    class Example
    {
        public string 資料路徑 {get; set;}= "";
        public int MaxCount {get; set; } = 0;
        public bool 打開功能 = false;
        public double Scale = 0.0;
    }

    class Program
    {
        public static string 資料路徑 = "";
        public static int  MaxCount = 0;
        public static bool 打開功能 = false;
        public static double Scale = 1.0;
        public static int Abc_Level = 0;

        static void Main(string[] args)
        {
            Console.WriteLine("load test.ini to Program");
            ConfigIni.Print = (s)=>{ Console.WriteLine($"\t{s}"); };
            ConfigIni.Load(@"test.ini", typeof(Program));
            Console.WriteLine(資料路徑);
            Console.WriteLine(MaxCount);
            Console.WriteLine(打開功能);
            Console.WriteLine(Scale);
            Console.WriteLine(Abc_Level);
            
            var em = new Example();
            Console.WriteLine("load test.ini to em");
            ConfigIni.Load("test.ini", typeof(Example), em, true);
            Console.WriteLine(em.資料路徑);
            Console.WriteLine(em.MaxCount);
            Console.WriteLine(em.打開功能);
            Console.WriteLine(em.Scale);
        }
    }
}
