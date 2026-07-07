using System;
using System.Threading;
using WixToolset.BootstrapperApplicationApi;

namespace Fabulor.Setup;

public static class Program
{
    public static void Main()
    {
        Exception? failure = null;
        var thread = new Thread(() =>
        {
            try
            {
                ManagedBootstrapperApplication.Run(new FabulorBootstrapperApplication());
            }
            catch (Exception ex)
            {
                failure = ex;
            }
        });

        thread.SetApartmentState(ApartmentState.MTA);
        thread.Start();
        thread.Join();

        if (failure != null)
        {
            throw failure;
        }
    }
}
