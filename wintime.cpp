/*
 * wintime: A "Linux-time"-like program for Windows platform.
 *
 * Author:     Jun Zhang
 * Bug Report: zhangjunqcc@gmail.com
 *
 * To compile: 
 *   g++ -O3 -fno-exceptions -static-libstdc++ -static-libgcc -static wintime.cpp -o wintime.exe
 *
 * Usage: 
 *   wintime [program and arguments]
 *
 */

#include <windows.h> 
#include <cstring>
#include <cstdio> 
#include <ctime>
#include <csignal>
#include <sys/time.h>

using namespace std;

class WinTime {
public:
    WinTime(void);
    ~WinTime(void);

    static void Start(void);
    static void End(void);
    static void SetUSRTime(const FILETIME& KernelTime, const FILETIME& UserTime);
    static void Report(void);

private:    
    static timeval earlyt;
    static timeval latet;
    static double usertime;
    static double systime;
    static double realtime;
    static double walltime;
    
    static double SYSTEMTIME2Second(const SYSTEMTIME& st);
};

timeval WinTime::earlyt;
timeval WinTime::latet;
double WinTime::usertime = 0.;
double WinTime::systime = 0.;
double WinTime::realtime = 0.;
double WinTime::walltime = 0.;

WinTime::WinTime(void) {}
WinTime::~WinTime(void) {}
void WinTime::Start(void) { gettimeofday(&earlyt, NULL); }
void WinTime::End(void) 
{
    gettimeofday(&latet, NULL);    
    walltime = (latet.tv_sec-earlyt.tv_sec)+(double)(latet.tv_usec-earlyt.tv_usec)/1000000;        
}
double WinTime::SYSTEMTIME2Second(const SYSTEMTIME& st) 
{
    return st.wHour*3600+st.wMinute*60+st.wSecond+(double)st.wMilliseconds/1000; 
}
void WinTime::SetUSRTime(const FILETIME& KernelTime, const FILETIME& UserTime)
{
    SYSTEMTIME kt;
    SYSTEMTIME ut;    
    FileTimeToSystemTime(&KernelTime, &kt);
    FileTimeToSystemTime(&UserTime, &ut);
    usertime = SYSTEMTIME2Second(ut);
    systime = SYSTEMTIME2Second(kt);
    realtime = usertime+systime;
}
void WinTime::Report(void)
{        
    printf("\n");    
    printf("%-6s %.3f s\n", "wall", walltime);    
    printf("%-6s %.3f s\n", "real", realtime);
    printf("%-6s %.3f s\n", "user", usertime);
    printf("%-6s %.3f s\n", "sys ", systime);
}

void BlockTermSignal(int signal)
{
    /* Nothing to do */
}

void ProcessSignal(void)
{
    signal(SIGINT, BlockTermSignal);
    signal(SIGILL, BlockTermSignal);
    signal(SIGFPE, BlockTermSignal);
    signal(SIGSEGV, BlockTermSignal);
    signal(SIGTERM, BlockTermSignal);
    signal(SIGABRT, BlockTermSignal);
}

void PrintHelp(void)
{
    printf("wintime: A \"Linux-time\"-like program for Windows.\n");
    printf("Author:  Jun Zhang (zhangjunqcc@gmail.com)\n");
    printf("Usage:   wintime [program arguments]\n");
}

int main(int argc, char** argv)
{
    if(argc == 1)
    {
        PrintHelp();
        return 0;
    }
    ProcessSignal();
    
    const int CMDLINELENGTH = 4096;
    char cmdline[CMDLINELENGTH] = {0};
    for(int i = 1; i < argc; ++i)
    {
        bool writequote = false;
        if(strchr(argv[i], ' ') != NULL) writequote = true; // To put "string" in the arguments
        if(writequote) strcat(cmdline, "\"");
        strcat(cmdline, argv[i]);
        if(writequote) strcat(cmdline, "\"");        
        strcat(cmdline, " ");
    }

    // printf("%s\n", cmdline);    
    
    WinTime::Start();

    STARTUPINFO si= { 0 };
    PROCESS_INFORMATION pi= { 0 };
    if(CreateProcess(NULL, cmdline, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi) == 0)
    {
        printf("Cannot run the command [ %s ]\n", cmdline);
        return -1;
    }    
    WaitForSingleObject(pi.hProcess, INFINITE);

    FILETIME CreationTime;
    FILETIME ExitTime;
    FILETIME KernelTime;
    FILETIME UserTime;    
    GetProcessTimes(pi.hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime);
    
    // Now we get system time
    WinTime::SetUSRTime(KernelTime, UserTime);

    // Now we get wall time
    WinTime::End();
    
    // Now we report the result
    WinTime::Report();

    return 0;
}
