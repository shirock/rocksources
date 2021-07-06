/*
See also: https://docs.microsoft.com/zh-tw/dotnet/desktop/winforms/controls/how-to-play-a-sound-from-a-windows-form
*/
using System;
using System.ComponentModel;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Media;

namespace Example
{
    public partial class Form1 : Form
    {
        public SoundPlayer[] players = new []
            {
                // Only support WAV file.
                new SoundPlayer(@"c:\Windows\Media\chimes.wav"),
                new SoundPlayer(@"c:\Windows\Media\Alarm01.wav"),
                new SoundPlayer(@"c:\Windows\Media\Ring01.wav")
            };

        public Button Btn1;
        public Button Btn2;
        public Button Btn3;

        public Form1()
        {
            InitializeComponent();
            // SoundPlayer player1 = new SoundPlayer();
            // player1.SoundLocation = @"c:\Windows\Media\chimes.wav";
        }

        private void Form1_Shown(object sender, System.EventArgs e)
        {
            players[0].Play();
        }

        private void BtnPlay_Click(object sender, EventArgs s)
        {
            // Button btn = (Button) sender;
            var btn = sender as Button;
            
            var i = Int32.Parse(btn.Text.Split(' ')[1]) - 1;
            players[i].Play();
        }
    }
}
