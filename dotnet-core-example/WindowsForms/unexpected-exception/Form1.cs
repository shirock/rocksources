using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

    public partial class Form1 : Form
    {
        public Label Label1;
        public Button Button1;

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

        private void Button1_Click(object sender, EventArgs s)
        {
            // 故意觸發未預期例外。
            var t = "a123";
            var i = Int32.Parse(t);

            Label1.Text = $"button click {i}";
        }
    }
