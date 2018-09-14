using System;
using System.Diagnostics;
using System.Net.Sockets;
using System.Text;

namespace client
{
    class Program
    {
        static void Main(string[] args)
        {
            var endpoint = new UnixDomainSocketEndPoint("server.sock");

            using (Socket client = new Socket(AddressFamily.Unix, SocketType.Stream, ProtocolType.Unspecified))
            {
                Console.WriteLine($"client: connecting to {endpoint.ToString()}");
                client.Connect(endpoint);

                var bytes = new byte[100];
                int received = client.Receive(bytes);

                Console.WriteLine($"received: {received} bytes, {Encoding.UTF8.GetString(bytes)}");

                Console.WriteLine($"client PID: {Process.GetCurrentProcess().Id}");
            }
        }
    }
}
