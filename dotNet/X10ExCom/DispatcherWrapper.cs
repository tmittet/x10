using System;
using System.ComponentModel;
using System.Threading;
using System.Windows.Threading;

namespace X10ExCom
{
    public class DispatcherWrapper : ISynchronizeInvoke
    {
        private readonly Dispatcher _dispatcher;

        public DispatcherWrapper(Dispatcher dispatcher)
        {
            _dispatcher = dispatcher;
        }

        #region ISynchronizeInvoke Members
  
        public IAsyncResult BeginInvoke(Delegate method, object[] args) 
        {
            DispatcherOperation op = _dispatcher.BeginInvoke(method, args);
            return new DispatcherOperationWrapper(op);
        }

        public object EndInvoke(IAsyncResult result)
        {
            DispatcherOperationWrapper wrapper = result as DispatcherOperationWrapper;
            if (wrapper != null)
            {
                wrapper.Operation.Wait();
                return wrapper.Operation.Result;
            }
            throw new ArgumentException("Result does not wrap a DispatchOperation");
        }

        public object Invoke(Delegate method, object[] args)
        {
            return _dispatcher.Invoke(method, args);
        }

        public bool InvokeRequired
        {
            get { return _dispatcher.CheckAccess(); }
        }

        #endregion

        private class DispatcherOperationWrapper : IAsyncResult  
        {  
            private DispatcherOperationWaitHandle _waitHandle;
            private readonly DispatcherOperation _operation;
            private readonly object _state;

            public DispatcherOperationWrapper(DispatcherOperation operation)
            {
                _operation = operation;
            }

            public DispatcherOperationWrapper(DispatcherOperation operation, object state)
                : this(operation)
            {
                _state = state;
            }

            public DispatcherOperation Operation
            {
                get { return _operation; }
            }

            #region IAsyncResult Members  
 
            public object AsyncState
            {
                get { return _state; }
            }

            public WaitHandle AsyncWaitHandle
            {
                get { return _waitHandle ?? (_waitHandle = new DispatcherOperationWaitHandle(_operation)); }
            }

            public bool CompletedSynchronously
            {
                get { return false; }
            }

            public bool IsCompleted
            {
                get { return (_operation.Status == DispatcherOperationStatus.Completed); }
            }

            #endregion

            private class DispatcherOperationWaitHandle : WaitHandle
            {
                private readonly DispatcherOperation _operation;
                public DispatcherOperationWaitHandle(DispatcherOperation operation)
                {
                    _operation = operation;
                }

                public override bool WaitOne()
                {
                    DispatcherOperationStatus status = _operation.Wait();
                    return (status == DispatcherOperationStatus.Completed);
                }

                public override bool WaitOne(int milliseconds)
                {
                    return WaitOne(new TimeSpan(0, 0, 0, 0, milliseconds));
                }

                public override bool WaitOne(int milliseconds, bool exitContext)
                {
                    return WaitOne(milliseconds);
                }

                public override bool WaitOne(TimeSpan timeout)
                {
                    DispatcherOperationStatus status = _operation.Wait(timeout);
                    return (status == DispatcherOperationStatus.Completed);
                }

                public override bool WaitOne(TimeSpan timeout, bool exitContext)
                {
                    return WaitOne(timeout);
                }
            }
        }
    }
}
