using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PJSIP
{
    public partial class Form1 : Form
    {
        private PjSipServer m_SipServer;
        private int callID=-1;

        public Form1()
        {
            InitializeComponent();
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            m_SipServer = new PjSipServer();

            m_SipServer.IncomingCall += (args) =>
            {
                this.Invoke((MethodInvoker)delegate()
                {
                    lblIncomingCall.Text = "Incoming Call!";
                    //lblIncomingCall.Text = args.CallID.ToString();
                    callID = args.CallID;
                });
            };

            m_SipServer.DtmfReceived += (args) =>
            {
                if(args.DTMF=="*")
                {
                    //creates a file only if the recorder is on
                    if (m_SipServer.CheckRecorder(callID))
                    {
                        m_SipServer.StopRecording(callID);
                    }
                }
                this.Invoke((MethodInvoker)delegate()
                {
                    lblDtmf.Text += args.DTMF;
                });
            };

            m_SipServer.CallDisconnected += (args) =>
            {
                this.Invoke((MethodInvoker)delegate()
                {
                    // I think if call disconnects in the middle i can write some code to make call_id and callID sync

                    lblIncomingCall.Text = "Call Disconnected!";
                });
            };

            m_SipServer.Start();

        }

        private void btnStop_Click(object sender, EventArgs e)
        {
            
            
        }

        private void btnAnswer_Click(object sender, EventArgs e)
        {
            m_SipServer.Answer(callID);
        }

        private void btnHangUp_Click(object sender, EventArgs e)
        {
            int statusCode=699;
            if (cmbStatusCodes.SelectedItem != null)
            {
                statusCode = int.Parse(cmbStatusCodes.SelectedItem.ToString());
            }

            m_SipServer.HangUp(callID, statusCode);
            //since we are hanging up we need to decrement callID
            callID--;
        }

        private void btnPlay_Click(object sender, EventArgs e)
        {
            m_SipServer.PlayPrompt(callID);
        }

        private void btnDial_Click(object sender, EventArgs e)
        {
            // since its a new call we have to increment callID
            callID++;
            m_SipServer.MakeCall(callID);
        }

        private void btnRecordCall_Click(object sender, EventArgs e)
        {
            m_SipServer.RecordCall(callID);
        }

        private void btnCreateConference_Click(object sender, EventArgs e)
        {
            //As we are creating new conference we have to increment the callID
            callID++;
            m_SipServer.CreateConference(callID);
        }

        private void btnRemoveConference_Click(object sender, EventArgs e)
        {
            m_SipServer.RemoveConference(callID);
            //since we are removing calls from conference we have to decrement callID 
            callID--;
        }

        private void btnStopRecording_Click(object sender, EventArgs e)
        {
            //creates a file only if the recorder is on
            if (m_SipServer.CheckRecorder(callID))
            {
                m_SipServer.StopRecording(callID);
            }
        }
    }
}
