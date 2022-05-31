using System;
using System.IO;
using System.Reflection;

namespace Example
{
    public class ConfigIni
    {
        public delegate void PrintMethod(string text);

        /**
        如果想要將載入過程列印到其他地方，則委派新的 Print 方法。
        預設是 Console.WriteLine() 。
         */
        public static PrintMethod Print = Console.WriteLine;

        /**
        載入對象的公開欄位，若為 string, int, bool, double 四種類型，
        都可從 iniFilepath 文件指派其值。
        
        @param iniFilepath ini形式的組態文件路徑。
        @param target 目標型別。
        @param obj 操作對象。若省略 obj 引數，則設置靜態公開欄位。
        @param onlyProperties 僅設置屬性?
        @return false 表示檔案不存在。
         */
        public static bool Load(
            string iniFilepath, 
            Type target, 
            Object obj = null, 
            bool onlyProperties = false)
        {
            FieldInfo field;
            PropertyInfo prop;
            string s, k, v;
            string[] kv;
            char[] delimiter = { '=' };
            string section = null;

            var config_file = new FileInfo(iniFilepath);
            if (!config_file.Exists)
            {
                Print($"{config_file.Name} is not available.");
                return false;
            }

            Print($"== Load configuration from {iniFilepath} ==");
            using (var sr = config_file.OpenText())
            {
                while ((s = sr.ReadLine()) != null)
                {
                    if (s.Length < 2 || s[0] == '#' || s[0] == ';')
                        continue;  // skip comment or empty line

                    s = s.TrimEnd();
                    if (s[0] == '[' && s.EndsWith(']'))
                    {
                        section = s.Substring(1, s.Length - 2);
                        continue;
                    }

                    kv = s.Split(delimiter);
                    if (kv.Length < 2)
                        continue;
                    k = kv[0].Trim();
                    v = kv[1].Trim(); 

                    if (section != null)
                        k = $"{section}_{k}";

                    prop = target.GetProperty(k);
                    if (prop != null)
                    {
                        setValue(prop, v, obj);
                    }
                    else if (!onlyProperties)
                    {
                        field = target.GetField(k);
                        if (field != null)
                        {
                            setValue(field, v, obj);
                        }
                    }
                }
            }
            Print($"== End configuration ==");
            return true;
        }

        private static void setValue(FieldInfo field, string value, Object obj)
        {
            // InitOnly means readonly.
            if (field.IsInitOnly)
                return;

            var t = field.FieldType;
            Print($"{field.Name} = {value}");
            if (typeof(string) == t)
            {
                // static 成員 => 類別變數 => 令obj為null
                field.SetValue(obj, value);
            }
            else if (typeof(bool) == t)
            {
                // True|true|Yes|yes|1
                bool bv = ("TtYy1".IndexOf(value[0]) >= 0);
                field.SetValue(obj, bv);
            }
            else if (typeof(int) == t)
            {
                int iv;
                if (Int32.TryParse(value, out iv))
                    field.SetValue(obj, iv);
            }
            else if (typeof(double) == t)
            {
                double dv;
                if (Double.TryParse(value, out dv))
                    field.SetValue(obj, dv);
            }
        }

        private static void setValue(PropertyInfo prop, string value, Object obj)
        {
            // no setter
            if (!prop.CanWrite)
                return;

            var t = prop.PropertyType;
            Print($"{prop.Name} = {value}");
            if (typeof(string) == t)
            {
                prop.SetValue(obj, value);
            }
            else if (typeof(bool) == t)
            {
                bool bv = ("TtYy1".IndexOf(value[0]) >= 0);
                prop.SetValue(obj, bv);
            }
            else if (typeof(int) == t)
            {
                int iv;
                if (Int32.TryParse(value, out iv))
                    prop.SetValue(obj, iv);
            }
            else if (typeof(double) == t)
            {
                double dv;
                if (Double.TryParse(value, out dv))
                    prop.SetValue(obj, dv);
            }
        }
    }
}