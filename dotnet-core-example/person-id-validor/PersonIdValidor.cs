using System.Collections.Generic;
using System.Text;

/**
<summary>
中華民國身分證號有效性驗證器。
</summary>
 */
public class PersonIdValidor
{
    // https://web.fg.tp.edu.tw/~anny/idtest.htm?fbclid=IwAR0pI0ymRmanb3sx2noxsfYO1UgzSBVyh3VIM-U3Sl_TaGg6ZrezNT5srwg
    static Dictionary<char, int> LOCALE_CODE_TBL = new Dictionary<char, int>()
    {
        {'A',  1},
        {'B', 10},
        {'C', 19},
        {'D', 28},
        {'E', 37},
        {'F', 46},
        {'G', 55},
        {'H', 64},
        {'I', 39},
        {'J', 73},
        {'K', 82},
        {'L',  2},
        {'M', 11},
        {'N', 20},
        {'O', 48},
        {'P', 29},
        {'Q', 38},
        {'R', 47},
        {'S', 56},
        {'T', 65},
        {'U', 74},
        {'V', 83},
        {'W', 21},
        {'X',  3},
        {'Y', 12},
        {'Z', 30}
    };

    /**
    <summary>
    中華民國身分證號有效性驗證。無視英文字母大小寫。
    </summary>
    <returns>True if Id is valid; otherwise, false.</returns>
     */
    public static bool IsValid(string Id)
    {
        if (Id.Length != 10)
        {
            // Console.WriteLine("1 not pass");
            return false;
        }

        var lc = Id.Substring(0, 1).ToUpper()[0];
        if (lc < 'A' || lc > 'Z')
        {
            return false;
        }

        var id = Encoding.ASCII.GetBytes(Id);
        int sum = LOCALE_CODE_TBL[lc];

        int d = 0;
        for (int i = 1, j = 8; j > 0; i++, j--)
        {
            d = id[i] - 48;
            if (d < 0 || d > 9)
            {
                // Console.WriteLine("2 not pass");
                return false;
            }
            // Console.WriteLine($"add {d} * {j}");
            sum += (d * j);
        }

        sum += (id[9] - 48);
        #if DEBUG
        Console.WriteLine(sum);
        #endif

        return (sum % 10) == 0;
    }
}
