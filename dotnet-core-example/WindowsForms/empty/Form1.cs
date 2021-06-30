using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace Example
{
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

        private void Label1_Click(object sender, EventArgs e)
        {
        }

        private void Button1_Click(object sender, EventArgs s)
        {
            Label1.Text = "button click";
        }
    }
}
