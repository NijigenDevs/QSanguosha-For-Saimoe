#include "UpdateChecker.h"

#include <QObject>
#include <QMessageBox>

#ifdef Q_OS_WIN
void UpdateChecker::GetModuleDirectory(LPTSTR szPath)
{
    GetModuleFileName(NULL, szPath, MAX_PATH);

    LPTSTR pSlash = _tcsrchr(szPath, '\\');

    if (pSlash == 0)
        szPath[2] = 0;
    else
        *pSlash = 0;
}

bool UpdateChecker::UpdateAvailable(bool notifyIfNewest)
{
    // Get the real path to wyUpdate.exe
    // (assumes it's in the same directory as this app)
    TCHAR szPath[MAX_PATH];
    szPath[0] = '\"';
    GetModuleDirectory(&szPath[1]);
    PathAppend(szPath, _T("wyUpdate.exe\" /quickcheck /justcheck"));


    STARTUPINFO si = { 0 }; si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };

    // start wyUpdate
    if (!CreateProcess(NULL, szPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        auto msg = QObject::tr("wyUpdate Doesn't Exist").utf16();
        auto title = QMessageBox::tr("Warning").utf16();
        MessageBox(0, reinterpret_cast<LPCWSTR>(msg), reinterpret_cast<LPCWSTR>(title), MB_OK);
        return false;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get the exit code
    DWORD exitcode = 0;
    GetExitCodeProcess(pi.hProcess, &exitcode);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // exitcode of 2 means update available
    if (exitcode == 2)
    {
        return true;
    }
    else if (exitcode == 0 && notifyIfNewest)
    {
        auto msg = QObject::tr("Your Version is Newest").utf16();
        auto title = QMessageBox::tr("Congratulation").utf16();
        MessageBox(0, reinterpret_cast<LPCWSTR>(msg), reinterpret_cast<LPCWSTR>(title), MB_OK);
        return false;
    }
    return false;
}
#endif

void UpdateChecker::CheckUpdate(bool notifyIfNewest)
{
#ifdef Q_OS_WIN
    if (UpdateAvailable(notifyIfNewest))
    {
        auto msg = QObject::tr("Do you want to update now?").utf16();
        auto title = QObject::tr("Update Available").utf16();
        if (MessageBox(0, reinterpret_cast<LPCWSTR>(msg), reinterpret_cast<LPCWSTR>(title), MB_OKCANCEL | MB_ICONINFORMATION | MB_TOPMOST) == IDOK)
        {
            TCHAR szPath[MAX_PATH];

            GetModuleDirectory(szPath);
            PathAppend(szPath, _T("wyUpdate.exe"));

            // Start the wyUpdate and Quit
            ShellExecute(NULL, NULL, szPath, NULL, NULL, SW_SHOWNORMAL);

            exit(1);
        }
    }
#else
    // not implemented
#endif
}