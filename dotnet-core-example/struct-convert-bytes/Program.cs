/*
Struct <=> Bytes 轉換範例

1. Convert struct to bytes.
2. Convert bytes to struct.
*/
using System;

namespace Example
{
    // example 1
    public struct Packet1
    {
        public Byte Count;
        public Int16 DataLength;
        public Int32 SensorId;
        public Double PositionX;
        public double PositionY;
        public char Mark;

        // avoid Warning CS0649.
        Packet1(byte n = 0)
        {
            Count = n;
            DataLength = 0;
            SensorId = 0;
            PositionX = 0.0;
            PositionY = 0.0;
            Mark = '\0';
        }
    }

    // example 2
    public struct Packet2
    {
        public Byte Count;
        public char Mark;
        public Int16 DataLength;
        public Int32 SensorId;
        public Double PositionX;
        public double PositionY;

        // avoid Warning CS0649.
        Packet2(byte n = 0)
        {
            Count = n;
            DataLength = 0;
            SensorId = 0;
            PositionX = 0.0;
            PositionY = 0.0;
            Mark = '\0';
        }
    }

    class Program
    {
        static void DumpStructBytes(byte[] bs)
        {
            Console.WriteLine($"{bs.Length} bytes:");
            int j = 0;
            for (var i = 0; i < bs.Length; ++i) {
                Console.Write(String.Format("{0:x2} ", bs[i]));
                if (++j >= 8) {
                    j = 0;
                    Console.WriteLine();
                }
            }
        }

        static void Main(string[] args)
        {
            var p1 = new Packet1();
            p1.Count = 1;
            p1.DataLength = 2;
            p1.SensorId = 3;
            p1.PositionX = 4.4;
            p1.PositionY = 5.5;
            p1.Mark = 'Z';

            var p2 = new Packet2() {
                Count = 6,
                DataLength = 7,
                SensorId = 8,
                PositionX = 1.1,
                PositionY = 2.2,
                Mark = 'Y'
            };

            Byte[] bs1 = StructConvert.StructToBytes<Packet1>(p1);
            DumpStructBytes(bs1);

            var bs2 = StructConvert.StructToBytes<Packet2>(p2);
            DumpStructBytes(bs2);
            /*
            使用注意，欄位對齊問題。
            比較 Packet1 和 Packet2 的傾印結果可看出，
            如果 1 bytes 長度的欄位夾在 2 bytes 以上長度的欄位之間，
            那這個欄位會佔 2 bytes ，而不是 1 bytes 。

            如果你的接觸對象送來的封包是這種結構，不要用這個方法轉換。
            */

            // little endian, raw data from Packet2
            var raw_packet = new byte[] {
                1, // => Count
                0x55, // => Mark 'U'
                16,0, // => DataLength
                32,0,0,0, // => SensorId
                0x9a, 0x99, 0x99, 0x99, 0x99, 0x99, 0xf1, 0x3f,
                0x9a, 0x99, 0x99, 0x99, 0x99, 0x99, 0xf1, 0x3f
            };

            Packet2 decoded = StructConvert.BytesToStruct<Packet2>(raw_packet);
            Console.WriteLine($"Count: {decoded.Count}");
            Console.WriteLine($"Mark: {decoded.Mark}");
            Console.WriteLine($"DataLength: {decoded.DataLength}");
        }
    }
}
