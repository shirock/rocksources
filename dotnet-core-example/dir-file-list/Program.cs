using System;
using System.Collections.Generic;
using System.IO; // Directory

namespace dir_file_list
{
    class Program
    {
        static void Main(string[] args)
        {
            /* Microsoft Docs
            The EnumerateFiles and GetFiles methods differ as follows: 
            When you use EnumerateFiles, you can start enumerating the collection of names 
            before the whole collection is returned; when you use GetFiles, you must wait for 
            the whole array of names to be returned before you can access the array. 
            Therefore, when you are working with many files and directories, EnumerateFiles 
            can be more efficient.
             */
            Console.WriteLine("EnumerateFiles:"); // deferred execution
            IEnumerable<string> files1 = Directory.EnumerateFiles("obj", "*.*");
            foreach (var filename in files1) {
                Console.WriteLine($"+ {filename}"); // include path
            }

            Console.WriteLine("GetFiles:"); // immediately execution
            string[] files2 = Directory.GetFiles("obj", "*.*");
            foreach (var filename in files2) {
                Console.WriteLine($"+ {filename}"); // include path
            }

            Console.WriteLine("DirectoryInfo EnumerateFiles:"); // deferred execution
            DirectoryInfo dirInfo = new DirectoryInfo("obj");
            IEnumerable<FileInfo> files3 = dirInfo.EnumerateFiles("*.*");
            foreach (var fileinfo in files3) {
                Console.WriteLine($"+ filepath: {fileinfo.DirectoryName}/{fileinfo.Name}");
                Console.WriteLine(string.Format("  {0} bytes. {1}",
                    fileinfo.Length,
                    fileinfo.IsReadOnly ? "read-only" : "read-write"));
            }

            Console.WriteLine(string.Format("full path of obj {0}", Path.GetFullPath("obj")));
            string filepath = "/test/111.txt";
            Console.WriteLine(string.Format("{0}: \n+ filename:{1} \n+ extension:{2} \n+ dir:{3}", 
                filepath, 
                Path.GetFileName(filepath),
                Path.GetExtension(filepath),
                Path.GetDirectoryName(filepath)));
        }
    }
}
