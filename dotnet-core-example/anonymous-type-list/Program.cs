using System;
using System.Linq;
using System.Diagnostics;

/**
 See http://rocksaying.tw/archives/2019/dotNET-Core-%E7%AD%86%E8%A8%98-anonymous-type-list.html
 */
namespace var
{
    class Program
    {
        static void Main(string[] args)
        {
            // 已知型別串列
            int[] int_list1 = new int[] {1, 3};
            var int_list2 = new int[] {1, 3};
            var int_list3 = new [] {1, 3};

            var list41 = new double[] {1, 3, 5.5};
            var list42 = new [] {1, 3, 5.5};
            Debug.Assert(list41.GetType() == list42.GetType(), 
                "兩者皆應為 double[]");

            // error CS0826: 找不到隱含類型陣列的最佳類型
            //var list5 = new [] {1, 2, "s"};

            var data1 = new {
                name = "a", 
                latitude = 0.0, 
                longitude = 0.0
            };

            var data2 = new {
                name = "b", 
                latitude = 1.0,  // double type
                longitude = 1.0
            };

            var data3 = new {
                name = "c",
                latitude = 1,  // int type
                longitude = 1
            };

            Debug.Assert(data1.GetType() == data2.GetType(),
                "兩者皆應為 {string,double,double}");
            Debug.Assert(data1.GetType() != data3.GetType(),
                "兩者應不同。後者為 {string,int,int}");

            var data_list = new [] {data1, data2}.ToList();

            var data_list2 = new [] { 
                new {
                    name = "", 
                    latitude = 0.0, 
                    longitude = 0.0
                }
            }.ToList();
            data_list2.RemoveAt(0);

            for (var i = 0; i < 10; ++i) {
                data_list2.Add(new {
                    name = $"pos {i}",
                    latitude = 0.0 + i,
                    longitude = 0.0 + i
                });
            }
        }
    }
}
