using System;
using System.Runtime.InteropServices;
using System.Text;

class DesktopAnalyzer {
    [DllImport("user32.dll")]
    static extern IntPtr FindWindow(string className, string windowName);
    
    [DllImport("user32.dll")]
    static extern IntPtr FindWindowEx(IntPtr parent, IntPtr child, string className, string windowName);
    
    [DllImport("user32.dll")]
    static extern int GetClassName(IntPtr hWnd, StringBuilder className, int maxCount);
    
    [DllImport("user32.dll")]
    static extern IntPtr SendMessageTimeout(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam, uint flags, uint timeout, out IntPtr result);
    
    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);
    
    delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    
    static void Main() {
        Console.WriteLine("=== Desktop Window Structure Analysis ===\n");
        
        // Find Progman
        IntPtr progman = FindWindow("Progman", null);
        Console.WriteLine("Progman: 0x{0:X}", progman.ToInt64());
        
        // Check SHELLDLL_DefView under Progman
        IntPtr defViewInProgman = FindWindowEx(progman, IntPtr.Zero, "SHELLDLL_DefView", null);
        Console.WriteLine("SHELLDLL_DefView in Progman: 0x{0:X}", defViewInProgman.ToInt64());
        
        // Count WorkerW windows
        int workerWCount = 0;
        IntPtr workerWWithDefView = IntPtr.Zero;
        
        EnumWindows((hWnd, lParam) => {
            StringBuilder className = new StringBuilder(256);
            GetClassName(hWnd, className, 256);
            
            if (className.ToString() == "WorkerW") {
                workerWCount++;
                IntPtr defView = FindWindowEx(hWnd, IntPtr.Zero, "SHELLDLL_DefView", null);
                if (defView != IntPtr.Zero) {
                    workerWWithDefView = hWnd;
                    Console.WriteLine("WorkerW #{0} with SHELLDLL_DefView: 0x{1:X}", workerWCount, hWnd.ToInt64());
                }
            }
            return true;
        }, IntPtr.Zero);
        
        Console.WriteLine("\nTotal WorkerW windows: {0}", workerWCount);
        Console.WriteLine("WorkerW with SHELLDLL_DefView: 0x{0:X}", workerWWithDefView.ToInt64());
        
        // Try sending 0x052C with different parameters
        Console.WriteLine("\n=== Testing different 0x052C parameters ===\n");
        
        uint SMTO_NORMAL = 0x0000;
        uint SMTO_ABORTIFHUNG = 0x0002;
        
        // Test 1: wParam=0, lParam=0 (current implementation)
        IntPtr result1;
        IntPtr ret1 = SendMessageTimeout(progman, 0x052C, IntPtr.Zero, IntPtr.Zero, SMTO_NORMAL, 1000, out result1);
        Console.WriteLine("Test 1 (wParam=0, lParam=0): ret={0}, result={1}", ret1.ToInt64(), result1.ToInt64());
        
        System.Threading.Thread.Sleep(500);
        
        // Test 2: wParam=0xD, lParam=0 (some implementations use this)
        IntPtr result2;
        IntPtr ret2 = SendMessageTimeout(progman, 0x052C, new IntPtr(0xD), IntPtr.Zero, SMTO_NORMAL, 1000, out result2);
        Console.WriteLine("Test 2 (wParam=0xD, lParam=0): ret={0}, result={1}", ret2.ToInt64(), result2.ToInt64());
        
        System.Threading.Thread.Sleep(500);
        
        // Test 3: wParam=0xD, lParam=1 (another variant)
        IntPtr result3;
        IntPtr ret3 = SendMessageTimeout(progman, 0x052C, new IntPtr(0xD), new IntPtr(1), SMTO_NORMAL, 1000, out result3);
        Console.WriteLine("Test 3 (wParam=0xD, lParam=1): ret={0}, result={1}", ret3.ToInt64(), result3.ToInt64());
        
        System.Threading.Thread.Sleep(500);
        
        // Check if WorkerW structure changed
        Console.WriteLine("\n=== After sending messages ===\n");
        defViewInProgman = FindWindowEx(progman, IntPtr.Zero, "SHELLDLL_DefView", null);
        Console.WriteLine("SHELLDLL_DefView still in Progman: {0}", defViewInProgman != IntPtr.Zero ? "YES" : "NO");
        
        workerWWithDefView = IntPtr.Zero;
        EnumWindows((hWnd, lParam) => {
            StringBuilder className = new StringBuilder(256);
            GetClassName(hWnd, className, 256);
            
            if (className.ToString() == "WorkerW") {
                IntPtr defView = FindWindowEx(hWnd, IntPtr.Zero, "SHELLDLL_DefView", null);
                if (defView != IntPtr.Zero) {
                    workerWWithDefView = hWnd;
                }
            }
            return true;
        }, IntPtr.Zero);
        
        Console.WriteLine("WorkerW with SHELLDLL_DefView found: {0}", workerWWithDefView != IntPtr.Zero ? "YES (0x" + workerWWithDefView.ToString("X") + ")" : "NO");
    }
}
