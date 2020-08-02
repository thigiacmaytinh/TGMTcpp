using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.IO;
using System.Drawing.Drawing2D;
using System.Net;
using System.Resources;
using TGMT;

namespace IPSS
{
    public partial class frmDemo : Form
    {
        Bitmap m_bmp;   

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        void PrintError(string message)
        {
            lblMessage.ForeColor = Color.Red;
            lblMessage.Text = message;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        void PrintSuccess(string message)
        {
            lblMessage.ForeColor = Color.Green;
            lblMessage.Text = message;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        void PrintMessage(string message)
        {
            lblMessage.ForeColor = Color.Black;
            lblMessage.Text = message;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        public frmDemo()
        {
            
            InitializeComponent();

        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        private void frmDemo_Load(object sender, EventArgs e)
        {
            CheckForIllegalCrossThreadCalls = false;
            this.KeyPreview = true;

        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        private void frmDemo_FormClosed(object sender, FormClosedEventArgs e)
        {

        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        private void frmDemo_KeyUp(object sender, KeyEventArgs e)
        {
            if (!e.Control)
                return;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        private void frmDemo_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.F5)
            {
                return;
            }
        }


        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        private void btnConvert_Click(object sender, EventArgs e)
        {
            if(m_bmp == null)
            {
                PrintError("No image");
                return;
            }

            Bitmap bmp = BridgeCLR.ConvertToBlur(m_bmp, trackBar1.Value);
            picResult.Image = bmp;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        private void loadImageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Image files |*.bmp;*.jpg;*.png;*.BMP;*.JPG;*.PNG";
            ofd.ShowDialog();
            string filePath = ofd.FileName;

            if(File.Exists(filePath))
            {
                m_bmp = new Bitmap(filePath);
                picSource.ImageLocation = filePath;
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            PrintMessage("Value: " + trackBar1.Value);

            if (m_bmp == null)
            {
                PrintError("No image");
                return;
            }

            Bitmap bmp = BridgeCLR.ConvertToBlur(m_bmp, trackBar1.Value);
            picResult.Image = bmp;
        }
    }
}
