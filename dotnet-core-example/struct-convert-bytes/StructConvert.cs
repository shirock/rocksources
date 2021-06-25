using System;
using System.Runtime.InteropServices;

namespace Example
{
    /*
    .net framework 4.5.1 淘汰非泛型方法，用泛型方法改寫。
    */
    public class StructConvert
    {
        /*
        Convert struct to bytes.

        淘汰非泛型方法 StructureToPtr()
        改用泛型方法 StructureToPtr<T>()
        */
        public static Byte[] StructToBytes<T>(T obj)
        {
            Int32 size = Marshal.SizeOf<T>();
            IntPtr buffer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr<T>(obj, buffer, false);
                Byte[] bytes = new Byte[size];
                Marshal.Copy(buffer, bytes, 0, size);
                return bytes;
            }
            finally
            {
                Marshal.FreeHGlobal(buffer);
            }
        }

        /*
        Convert bytes to struct.

        淘汰非泛型方法 PtrToStructure(IntPtr, Type)
        改用泛型方法 PtrToStructure<T>(IntPtr)
        */
        public static T BytesToStruct<T>(Byte[] bytes)
        {
            Int32 size = Marshal.SizeOf<T>();
            IntPtr buffer = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.Copy(bytes, 0, buffer, size);
                return Marshal.PtrToStructure<T>(buffer);
            }
            finally
            {
                Marshal.FreeHGlobal(buffer);
            }
        }
    }
}