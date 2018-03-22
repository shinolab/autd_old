using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace simple
{
    class Program
    {
        [DllImport("autd3-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int AUTDOpenController(string location, out IntPtr handle);
        [DllImport("autd3-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void AUTDFreeController(IntPtr handle);
        [DllImport("autd3-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void AUTDAddDevice(IntPtr handle, float x, float y, float z, float rz1, float ry, float rz2);
        [DllImport("autd3-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int AUTDSetGain(IntPtr handle, int deviceIndex, int transIndex, int amp, int phase);
        [DllImport("autd3-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int AUTDSetFocalpoint(IntPtr handle, float x, float y, float z, int amp);

        static void Main(string[] args)
        {
            IntPtr handle = new IntPtr();
            AUTDOpenController("", out handle);
            AUTDAddDevice(handle, 0, 0, 0, 0, 0, 0);
            //AUTDSetGain(handle, 0,0,255,0);
            AUTDSetFocalpoint(handle, 0, 0, 200, 255);
            Console.WriteLine("Press any key to stop...");
            Console.ReadKey();
            AUTDFreeController(handle);
        }
    }
}
