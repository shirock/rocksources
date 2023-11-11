using System.ComponentModel;
using System.Drawing;

public partial class Form1 : Form
{
    public Label Label1;
    public Label Label2;
    public Label Label3;

    public Form1()
    {
        InitializeComponent();
    }

    private void Form1_Shown(object sender, EventArgs e)
    {
    }

    private void Form1_Closing(object sender, CancelEventArgs e)
    {
    }

    private void Form1_KeyDown(object sender, KeyEventArgs e)
    {
        // Ctrl + F1.
        if(e.Control && e.KeyCode == Keys.F1)
        {
            MessageBox.Show("Press Ctrl+F1");
            e.Handled = true;
        }

        Label2.Text = string.Format("KeyDown: {0}; {1}; {2}", e.KeyCode, e.KeyValue, e.KeyData);
    }

    private void Form1_KeyPress(object sender, KeyPressEventArgs e)
    {
        // Ctrl+Back is 0x7F
        if (e.KeyChar == 0x7F)
        {
            MessageBox.Show("Press Ctrl+Back");
            e.Handled = true;
        }

        Label3.Text = string.Format("Press {0}; ASCII Code: 0x{1:x}", e.KeyChar, (int)e.KeyChar);
    }

    private void Label1_Click(object sender, EventArgs e)
    {
    }

    private void Button1_Click(object sender, EventArgs s)
    {
    }
}
