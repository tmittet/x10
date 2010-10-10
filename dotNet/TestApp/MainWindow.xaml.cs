/************************************************************************/
/* X10 with Arduino .Net test application, v1.0.                        */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet thomas@mittet.nu October 2010.              */
/************************************************************************/

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using X10ExCom;

namespace TestApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow
    {
        #region Private Fields and Properties

        private Serial _serial;

        private X10House House
        {
            get { return cbxHouse.SelectedIndex == 0 ? X10House.X : (X10House)cbxHouse.SelectedIndex + 64; }
            set { cbxHouse.SelectedIndex = value == X10House.X ? 0 : (byte)value - 64; }
        }

        private byte Scenario
        {
            get
            {
                string value =
                    cbxScenario.SelectedValue != null ?
                    cbxScenario.SelectedValue.ToString() :
                    cbxScenario.Text;
                byte result;
                return byte.TryParse(
                    value,
                    System.Globalization.NumberStyles.AllowHexSpecifier,
                    System.Globalization.CultureInfo.InvariantCulture,
                    out result) ? result : (byte)0;
            }
            set { cbxScenario.Text = value == 0 ? "" : value.ToString("X").PadLeft(2, '0'); }
        }

        private X10Unit Unit
        {
            get { return cbxUnit.SelectedIndex == 0 ? X10Unit.X : (X10Unit)cbxUnit.SelectedIndex - 1; }
            set { cbxUnit.SelectedIndex = value == X10Unit.X ? 0 : (byte)value + 1; }
        }

        private X10Command Command
        {
            get { return cbxCommand.SelectedIndex == 0 ? X10Command.X : (X10Command)cbxCommand.SelectedIndex - 1; }
            set { cbxCommand.SelectedIndex = value == X10Command.X ? 0 : (byte)value + 1; }
        }

        private byte ExtendedCommand
        {
            get
            {
                byte value;
                if (byte.TryParse(
                    cbxExtCommand.Text.Replace("0x", ""),
                    System.Globalization.NumberStyles.AllowHexSpecifier,
                    System.Globalization.CultureInfo.InvariantCulture,
                    out value))
                {
                    return value;
                }
                if (cbxExtCommand.SelectedIndex == 0)
                {
                    return 0x31;
                }
                return 0;
            }
            set
            {
                cbxExtCommand.Text = "0x" + value.ToString("X").PadLeft(2, '0');
            }
        }

        private byte ExtendedData
        {
            get
            {
                byte value;
                if (byte.TryParse(
                    txtExtData.Text.Replace("0x", ""),
                    System.Globalization.NumberStyles.AllowHexSpecifier,
                    System.Globalization.CultureInfo.InvariantCulture,
                    out value))
                {
                    return value;
                }
                return 0;
            }
            set
            {
                txtExtData.Text = "0x" + value.ToString("X").PadLeft(2, '0');
            }
        }

        #endregion

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            for (int i = 1; i <= 255; i++)
            {
                cbxScenario.Items.Add(i.ToString("X").PadLeft(2, '0'));
            }
            cbxExtCommand.Items.Add("0x31 (Pre Set Dim)");
            House = X10House.A;
            Scenario = 1;
            Unit = X10Unit.X;
            Command = X10Command.X;
        }

        #endregion

        #region Serial Port Events

        void _serial_MessageReceived(object source, X10Message message)
        {
            txtReceivedLog.Text += message.SourceString + ": " + message + Environment.NewLine;
            txtReceivedLog.ScrollToEnd();
            txtParsedEventLog.Text +=
                (message.Source + ":").PadRight(13, ' ') +
                message.ToHumanReadableString() +
                Environment.NewLine;
            txtParsedEventLog.ScrollToEnd();

        }

        #endregion

        #region Window/Form Events

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            UpdateLayout();
            // Connect to Arduino running X10ex
            try
            {
                _serial = new Serial(115200, null, new DispatcherWrapper(Dispatcher));
                _serial.MessageReceived += _serial_MessageReceived;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }            
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_serial != null)
            {
                _serial.Dispose();
            }
        }

        #endregion

        #region Button and Misc. Control Events

        private void cbxType_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            switch (cbxType.SelectedIndex)
            {
                case 1: // Scenario Execute
                    lblHouse.Visibility = Visibility.Hidden;
                    cbxHouse.Visibility = Visibility.Hidden;
                    lblScenario.Visibility = Visibility.Visible;
                    cbxScenario.Visibility = Visibility.Visible;
                    lblUnit.Visibility = Visibility.Hidden;
                    cbxUnit.Visibility = Visibility.Hidden;
                    lblCommand.Visibility = Visibility.Hidden;
                    cbxCommand.Visibility = Visibility.Hidden;
                    rabOn.IsEnabled = false;
                    rabOff.IsEnabled = false;
                    sdrBrightness.IsEnabled = false;
                    break;                
                case 2: // Module State Request
                    lblScenario.Visibility = Visibility.Hidden;
                    cbxScenario.Visibility = Visibility.Hidden;
                    lblCommand.Visibility = Visibility.Hidden;
                    cbxCommand.Visibility = Visibility.Hidden;
                    lblHouse.Visibility = Visibility.Visible;
                    cbxHouse.Visibility = Visibility.Visible;
                    lblUnit.Visibility = Visibility.Visible;
                    cbxUnit.Visibility = Visibility.Visible;
                    rabOn.IsEnabled = false;
                    rabOff.IsEnabled = false;
                    sdrBrightness.IsEnabled = false;
                    break;
                case 3: // Module State Wipe
                    lblScenario.Visibility = Visibility.Hidden;
                    cbxScenario.Visibility = Visibility.Hidden;
                    lblUnit.Visibility = Visibility.Hidden;
                    cbxUnit.Visibility = Visibility.Hidden;
                    lblCommand.Visibility = Visibility.Hidden;
                    cbxCommand.Visibility = Visibility.Hidden;
                    lblHouse.Visibility = Visibility.Visible;
                    cbxHouse.Visibility = Visibility.Visible;
                    rabOn.IsEnabled = false;
                    rabOff.IsEnabled = false;
                    sdrBrightness.IsEnabled = false;
                    break;
                default: // X10 Message
                    lblScenario.Visibility = Visibility.Hidden;
                    cbxScenario.Visibility = Visibility.Hidden;
                    lblHouse.Visibility = Visibility.Visible;
                    cbxHouse.Visibility = Visibility.Visible;
                    lblUnit.Visibility = Visibility.Visible;
                    cbxUnit.Visibility = Visibility.Visible;
                    lblCommand.Visibility = Visibility.Visible;
                    cbxCommand.Visibility = Visibility.Visible;
                    rabOn.IsEnabled = true;
                    rabOff.IsEnabled = true;
                    sdrBrightness.IsEnabled = true;
                    break;
            }
            SelectionChangeUpdate();
        }

        private void cbxHouse_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            SelectionChangeUpdate();
        }

        private void cbxScenario_KeyUp(object sender, KeyEventArgs e)
        {
            SelectionChangeUpdate();
        }

        private void cbxScenario_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            SelectionChangeUpdate();
        }

        private void cbxUnit_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            // If selected message type is Standard Message
            if (cbxType.SelectedIndex == 0)
            {
                rabOn.IsEnabled = Unit != X10Unit.X;
                rabOff.IsEnabled = Unit != X10Unit.X;
                sdrBrightness.IsEnabled = Unit != X10Unit.X;
            }
            // Update UI
            SelectionChangeUpdate();
        }
        
        private void cbxCommand_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            bool isNoUnitCommand =
                Command == X10Command.AllLightsOff ||
                Command == X10Command.AllLightsOn ||
                Command == X10Command.AllUnitsOff ||
                Command == X10Command.Bright ||
                Command == X10Command.Dim ||
                Command == X10Command.HailAcknowledge ||
                Command == X10Command.HailRequest;
            if (isNoUnitCommand)
            {
                Unit = X10Unit.X;
            }
            SelectionChangeUpdate();
        }

        private void rabOn_Checked(object sender, RoutedEventArgs e)
        {
            Command = X10Command.On;
            btnSend_Click(sender, e);
        }

        private void rabOff_Checked(object sender, RoutedEventArgs e)
        {
            Command = X10Command.Off;
            btnSend_Click(sender, e);
        }

        private void sdrBrightness_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            Command = X10Command.ExtendedCode;
            cbxExtCommand.Text = "0x31 (Pre Set Dim)";
            cbxExtCommand.SelectedValue = "0x31 (Pre Set Dim)";
            txtExtData.Text = "0x" + Convert.ToByte(Math.Round(sdrBrightness.Value / 100 * 62)).ToString("X").PadLeft(2, '0');
        }

        private void sdrBrightness_PreviewMouseUp(object sender, MouseButtonEventArgs e)
        {
            // Off
            if (sdrBrightness.Value == 0)
            {
                if (rabOff.IsChecked == true)
                {
                    rabOff.IsChecked = null;
                }
                rabOff.IsChecked = true;
            }
            // Pre Set Dim (Extended Code)
            else
            {
                rabOff.IsChecked = false;
                rabOn.Checked -= rabOn_Checked;
                rabOn.IsChecked = true;
                rabOn.Checked += rabOn_Checked;
                btnSend_Click(sender, e);
            }
        }

        private void cbxExtCommand_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            SelectionChangeUpdate();
        }

        private void cbxExtCommand_KeyUp(object sender, KeyEventArgs e)
        {
            SelectionChangeUpdate();
        }

        private void txtExtData_TextChanged(object sender, TextChangedEventArgs e)
        {
            SelectionChangeUpdate();
        }

        private void txtMessage_KeyUp(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.Enter || e.Key == Key.Return)
            {
                btnSend_Click(sender, e);
            }
        }

        private void btnSend_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                X10Message message = X10Message.Parse(txtMessage.Text.Trim());
                _serial.SendMessage(message);
                txtSentLog.Text += message + Environment.NewLine;
                txtSentLog.ScrollToEnd();
            }
            catch (ArgumentException ex)
            {
                MessageBox.Show(ex.Message);
            }
            catch (Exception ex)
            {
                MessageBox.Show("An unexpected parser error occured. " + ex.Message);
            }
        }

        private void btnClearSentLog_Click(object sender, RoutedEventArgs e)
        {
            txtSentLog.Clear();
        }

        private void btnClearReceivedLog_Click(object sender, RoutedEventArgs e)
        {
            txtReceivedLog.Clear();
        }

        private void btnClearParsedEventLog_Click(object sender, RoutedEventArgs e)
        {
            txtParsedEventLog.Clear();
        }

        #endregion

        #region Private Methods

        private void SelectionChangeUpdate()
        {
            Visibility extendedVisible = Visibility.Hidden;
            try
            {
                switch (cbxType.SelectedIndex)
                {
                    case 1: // Scenario Execute
                        txtMessage.Text = new X10ScenarioExecute(Scenario).ToString();
                        break;
                    case 2: // Module State Request
                        txtMessage.Text = new X10ModuleStateRequest(House, Unit).ToString();
                        break;
                    case 3: // Module State Wipe
                        txtMessage.Text = new X10ModuleStateWipe(House).ToString();
                        break;
                    default: // X10 Message
                        if (Command != X10Command.ExtendedCode && Command != X10Command.ExtendedData)
                        {
                            txtMessage.Text = new X10StandardMessage(House, Unit, Command).ToString();
                        }
                        else
                        {
                            extendedVisible = Visibility.Visible;
                            if (ExtendedCommand != 0 || ExtendedData != 0)
                            {
                                txtMessage.Text = new X10ExtendedMessage(House, Unit, Command, ExtendedCommand, ExtendedData).ToString();
                            }
                        }
                        break;
                }
            }
            catch (ArgumentException ex)
            {
                MessageBox.Show(ex.Message);
            }
            lblExtCommand.Visibility = extendedVisible;
            lblExtData.Visibility = extendedVisible;
            cbxExtCommand.Visibility = extendedVisible;
            txtExtData.Visibility = extendedVisible;
        }

        #endregion
    }
}