using System;
using System.IO;

namespace Test
{
    class Program
    {
        static void generateElements(string fileName, int size)
        {
            Random random = new Random();
            StreamWriter streamWriter = new StreamWriter(fileName);

            streamWriter.WriteLine(size);

            for (int i = 0; i < size; i++)
            {
                streamWriter.Write((Math.Min(random.Next(-size, size * 2) + random.NextDouble(), Int32.MaxValue)).ToString().Replace(',', '.') + " ");
            }

            streamWriter.Close();
        }

        static void Main(string[] args)
        {
            string fileName = "elements.txt";

            int[] countProcess = new int[] { 1,2,4,8};
            int size = 40_000_000;

            generateElements(fileName, size);

            foreach (int i in countProcess)
            {
                Console.WriteLine("Run testing for size: " + size);

                string command = string.Format("mpiexec -n {0} QuickSort.exe {1} sortElemenets.txt", i, fileName);
                System.Diagnostics.ProcessStartInfo procStartInfo = new System.Diagnostics.ProcessStartInfo("cmd", "/c " + command);
                procStartInfo.RedirectStandardOutput = true;
                procStartInfo.UseShellExecute = false;
                procStartInfo.CreateNoWindow = true;
                System.Diagnostics.Process proc = new System.Diagnostics.Process();
                proc.StartInfo = procStartInfo;
                proc.Start();

                string result = proc.StandardOutput.ReadToEnd();

                Console.WriteLine("Count process: " + i + "\t" + result);


                Console.WriteLine("End testing");
            }

            Console.WriteLine("End all");
            Console.ReadLine();
        }
    }
}
