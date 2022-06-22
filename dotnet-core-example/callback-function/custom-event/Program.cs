// See https://aka.ms/new-console-template for more information
Console.WriteLine("Custom Event Example!");
var kb = new Keyboard();

void HandleInputted(object sender, CustomEventArgs e)
{
    Console.WriteLine($"subscriber1 收到資料: {e.Text}");
}

kb.Inputted += HandleInputted;

kb.Inputted += (_, e) => {
    Console.WriteLine($"subscriber2 收到資料: {e.Text}");
};

kb.Loop();

public class CustomEventArgs: EventArgs
{
    public CustomEventArgs(string text)
    {
        this.text = text;
    }

    private string text;

    public string Text
    {
        get => text;
    }
}

public class Keyboard
{
    public event EventHandler<CustomEventArgs>? Inputted = null;

    // https://docs.microsoft.com/zh-tw/dotnet/csharp/programming-guide/events/how-to-raise-base-class-events-in-derived-classes
    public void OnInputted(CustomEventArgs e)
    {
        // Safely raise the event for all subscribers
        Inputted?.Invoke(this, e);
    }

    public Keyboard()
    {
    }

    public void Loop()
    {
        Console.WriteLine("Ctrl+C 中止");
        while (true)
        {
            Console.Write("輸入一行> ");
            var text = Console.ReadLine();
            if (!string.IsNullOrEmpty(text))
            {
                OnInputted(new CustomEventArgs(text));
            }
        }
    }
}
