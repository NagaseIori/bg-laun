#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <cstdio>

// Print help
void printUsage(const wchar_t* exeName)
{
    std::wcout << L"Usage: " << exeName << L" [options] <command>" << std::endl;
    std::wcout << L"Options:" << std::endl;
    std::wcout << L"  --blocking   Run in blocking mode" << std::endl;
    std::wcout << L"  --console    Allocate a console window" << std::endl;
    std::wcout << L"  --help       Show help" << std::endl;
}

// Parse arguments
bool parseArguments(int argc, wchar_t* argv[], std::wstring &command, bool &blockingMode, bool &showConsole)
{
    blockingMode = false;
    showConsole = false;
    command.clear();

    for (int i = 1; i < argc; ++i)
    {
        std::wstring arg = argv[i];
        if (arg == L"--help")
        {
            printUsage(argv[0]);
            return false;
        }
        else if (arg == L"--blocking")
        {
            blockingMode = true;
        }
        else if (arg == L"--console")
        {
            showConsole = true;
        }
        else if (arg[0] == L'-')
        {
            std::wcerr << L"Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return false;
        }
        else
        {
            if (!command.empty())
                command += L" ";
            command += arg;
        }
    }

    if (command.empty())
    {
        std::wcerr << L"Error: No command specified." << std::endl;
        printUsage(argv[0]);
        return false;
    }
    return true;
}

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
    return FALSE;
}

HANDLE g_hStopEvent = NULL;

void run_cmd_in_background_blocking(const std::wstring& cmd)
{
    HANDLE hJob = CreateJobObject(nullptr, nullptr);
    if (!hJob)
    {
        std::wcerr << L"CreateJobObject failed, error: " << GetLastError() << std::endl;
        return;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
    {
        std::wcerr << L"SetInformationJobObject failed, error: " << GetLastError() << std::endl;
        CloseHandle(hJob);
        return;
    }

    STARTUPINFOW si = { 0 };
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = { 0 };

    std::wstring cmdLine = cmd;
    wchar_t* szCmdLine = &cmdLine[0];

    BOOL success = CreateProcessW(nullptr, szCmdLine, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    if (!success)
    {
        std::wcerr << L"CreateProcess failed, error: " << GetLastError() << std::endl;
        CloseHandle(hJob);
        return;
    }

    if (!AssignProcessToJobObject(hJob, pi.hProcess))
    {
        std::wcerr << L"AssignProcessToJobObject failed, error: " << GetLastError() << std::endl;
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hJob);
        return;
    }

    std::wcout << L"Blocking mode: waiting for child process." << std::endl;
    WaitForSingleObject(pi.hProcess, INFINITE);
    std::wcout << L"Child process exited." << std::endl;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hJob);
}

int run_cmd_nonblocking(const std::wstring& cmd)
{
    STARTUPINFOW si = { 0 };
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = { 0 };

    std::wstring cmdLine = cmd;
    wchar_t* szCmdLine = &cmdLine[0];

    BOOL success = CreateProcessW(nullptr, szCmdLine, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    if (!success)
    {
        std::wcerr << L"CreateProcess failed, error: " << GetLastError() << std::endl;
        return 1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::wcout << L"Non-blocking mode: child launched." << std::endl;
    return 0;
}

int Main(int argc, wchar_t* argv[])
{
    std::wstring command;
    bool blockingMode = false, showConsole = false;

    if (!parseArguments(argc, argv, command, blockingMode, showConsole))
        return 1;

    if (showConsole)
    {
        if (AllocConsole())
        {
            FILE* fp;
            freopen_s(&fp, "CONIN$", "r", stdin);
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONOUT$", "w", stderr);
        }
        else
        {
            std::wcerr << L"AllocConsole failed, error: " << GetLastError() << std::endl;
        }
    }

    if (blockingMode)
    {
        g_hStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!g_hStopEvent)
        {
            std::wcerr << L"CreateEvent failed, error: " << GetLastError() << std::endl;
            return 1;
        }
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);
        std::thread bgThread(run_cmd_in_background_blocking, command);
        bgThread.join();
        CloseHandle(g_hStopEvent);
        return 0;
    }
    else
    {
        return run_cmd_nonblocking(command);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    int result = Main(argc, argv);
    LocalFree(argv);
    return result;
}
