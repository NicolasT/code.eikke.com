using System;
using System.Runtime.InteropServices;


public class Test {

        [DllImport ("libtest")] extern static string gettype (string file);

        static int Main (String [] args)
        {
                foreach (String name in args) {
                        String mime =   gettype(name);
                        Console.WriteLine ("{0}\t{1}", name, mime);
                }
                return 0;
        }
}
