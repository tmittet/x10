using System;
using System.Text;
using log4net;
using log4net.Appender;
using log4net.Config;
using log4net.Core;
using log4net.Layout;

namespace X10ExCom
{
    public class Log
    {
        private readonly ILog _log;

        public Log(Type type, string logFileLocation)
        {
            if (!String.IsNullOrEmpty(logFileLocation) && logFileLocation.Trim().Length > 0)
            {
                FileAppender fileAppender = new FileAppender
                {
                    AppendToFile = true,
                    Encoding = Encoding.UTF8,
                    File = logFileLocation.Trim(),
                    Layout = new PatternLayout("%d{HH:mm:ss,fff} [%t] %-5p %c - %m%n"),
                    Name = "LogFileAppender",
                    Threshold = Level.Debug,
                };
                fileAppender.ActivateOptions();
                BasicConfigurator.Configure(fileAppender);
                _log = LogManager.GetLogger(type);
                Info("File Logging Started.");
            }
            else
            {
                Info("Console Logging Started.");
            }
        }

        public void Debug(string message)
        {
            if (_log != null)
            {
                _log.Debug(message);
            }
            else
            {
                Console.WriteLine("Debug: " + message);
            }
        }

        public void Info(string message)
        {
            if (_log != null)
            {
                _log.Info(message);
            }
            else
            {
                Console.WriteLine("Info: " + message);
            }
        }

        public void Warn(string message)
        {
            if (_log != null)
            {
                _log.Warn(message);
            }
            else
            {
                Console.WriteLine("Warn: " + message);
            }
        }

        public void Error(string message)
        {
            if (_log != null)
            {
                _log.Error(message);
            }
            else
            {
                Console.WriteLine("Error: " + message);
            }
        }
    }
}