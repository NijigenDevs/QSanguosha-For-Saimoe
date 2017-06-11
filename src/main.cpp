#include <QFile>
#include <QCoreApplication>
#include <QApplication>
#include <QTranslator>
#include <QDateTime>
#include <QSplashScreen>
#include <QMessageBox>

#include "server.h"
#include "settings.h"
#include "engine.h"
#include "mainwindow.h"
#include "audio.h"
#include "stylehelper.h"

#ifndef WINDOWS
#include <QDir>
#endif

#ifdef USE_BREAKPAD
#include <client/windows/handler/exception_handler.h>
#include <QProcess>

using namespace google_breakpad;
#endif

#ifdef Q_OS_WIN
#ifndef _UNICODE
#define _UNICODE
#endif
#include "Shlwapi.h"
#include "tchar.h"
#include "string.h"
#include <iostream>
#include <thread>
#pragma comment (lib, "Shlwapi.lib")

void GetModuleDirectory(LPTSTR szPath)
{
    GetModuleFileName(NULL, szPath, MAX_PATH);

    LPTSTR pSlash = _tcsrchr(szPath, '\\');

    if (pSlash == 0)
        szPath[2] = 0;
    else
        *pSlash = 0;
}

bool UpdateAvailable()
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
    return exitcode == 2;
}

void checkUpdate()
{
    if (UpdateAvailable())
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
}
#endif

int main(int argc, char *argv[])
{
    bool noGui = argc > 1 && strcmp(argv[1], "-server") == 0;

    if (noGui)
    {
        new QCoreApplication(argc, argv);
    }
    else
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
        new QApplication(argc, argv);
    }

#if defined(Q_OS_MAC) || defined(Q_OS_ANDROID)
#define showSplashMessage(message)
#define SPLASH_DISABLED
#else
    QSplashScreen *splash = NULL;
    if (!noGui) {
        QPixmap logo;
        QDate currentDate = QDate::currentDate();
        logo.load("image/system/developers/logo.png");

        splash = new QSplashScreen(logo);
        splash->show();
        qApp->processEvents();
    }
#define showSplashMessage(message) \
    if (splash == NULL) {\
        puts(message.toUtf8().constData());\
        } else {\
        splash->showMessage(message, Qt::AlignBottom | Qt::AlignHCenter, Qt::white);\
        qApp->processEvents();\
        }
#endif

#ifdef USE_BREAKPAD
    showSplashMessage(QSplashScreen::tr("Loading BreakPad..."));
    ExceptionHandler eh(L"./dmp", NULL, NULL, NULL, ExceptionHandler::HANDLER_ALL);
#endif

#if defined(Q_OS_MAC) && defined(QT_NO_DEBUG)
    showSplashMessage(QSplashScreen::tr("Setting game path..."));
    QDir::setCurrent(qApp->applicationDirPath());
#endif

#ifdef Q_OS_LINUX
    showSplashMessage(QSplashScreen::tr("Checking game path..."));
    QDir dir(QString("lua"));
    if (dir.exists() && (dir.exists(QString("config.lua")))) {
        // things look good and use current dir
    } else {
        showSplashMessage(QSplashScreen::tr("Setting game path..."));
#ifndef Q_OS_ANDROID
        QDir::setCurrent(qApp->applicationFilePath().replace("games", "share"));
#else
        bool found = false;
        QDir storageDir("/storage");
        QStringList sdcards = storageDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const QString &sdcard, sdcards) {
            QDir root(QString("/storage/%1/Android/data/org.nijigendevs.saimoe").arg(sdcard));
            if (root.exists("lua/config.lua")) {
                QDir::setCurrent(root.absolutePath());
                found = true;
                break;
            }
        }
        if (!found) {
            QDir root("/sdcard/Android/data/org.nijigendevs.saimoe");
            if (root.exists("lua/config.lua")) {
                QDir::setCurrent(root.absolutePath());
                found = true;
            }
        }


        if (!found) {
            QString m = QObject::tr("Game data not found, please download QSanguosha-For-Hegemony PC version, and put the files and folders into /sdcard/Android/data/org.nijigendevs.saimoe");
            if (!noGui)
                QMessageBox::critical(NULL, QObject::tr("Error"), m);
            else
                puts(m.toLatin1().constData());

            return -2;
        }
#endif
    }
#endif

    // initialize random seed for later use
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    // load the main translation file first for we need to translate messages of splash.
    QTranslator translator;
    translator.load("sanguosha.qm");
    qApp->installTranslator(&translator);

    showSplashMessage(QSplashScreen::tr("Loading translation..."));
    QTranslator qt_translator;
    qt_translator.load("qt_zh_CN.qm");
    qApp->installTranslator(&qt_translator);

    showSplashMessage(QSplashScreen::tr("Initializing game engine..."));
    new Settings;
    Sanguosha = new Engine;

    showSplashMessage(QSplashScreen::tr("Loading user's configurations..."));
    Config.init();
    if (!noGui) {
        QFont f = Config.AppFont;
#ifdef Q_OS_ANDROID
        f.setPointSize(12);
#endif
        qApp->setFont(f);
    }

    if (qApp->arguments().contains("-server")) {
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if (server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    }

    showSplashMessage(QSplashScreen::tr("Loading style sheet..."));
    QFile file("style-sheet/sanguosha.qss");
    QString styleSheet;
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        styleSheet = stream.readAll();
    }

#ifdef Q_OS_WIN
    QFile winFile("style-sheet/windows-extra.qss");
    if (winFile.open(QIODevice::ReadOnly)) {
        QTextStream winStream(&winFile);
        styleSheet += winStream.readAll();
    }
#endif

    qApp->setStyleSheet(styleSheet + StyleHelper::styleSheetOfTooltip());

#ifdef AUDIO_SUPPORT
    showSplashMessage(QSplashScreen::tr("Initializing audio module..."));
    Audio::init();
#else
//    if (!noGui)
//        QMessageBox::warning(NULL, QMessageBox::tr("Warning"), QMessageBox::tr("Audio support is disabled when compiled"));
#endif

    showSplashMessage(QSplashScreen::tr("Loading main window..."));
    MainWindow main_window;

    Sanguosha->setParent(&main_window);
    main_window.show();
#ifndef SPLASH_DISABLED
    if (splash != NULL) {
        splash->finish(&main_window);
        delete splash;
    }
#endif

    foreach (const QString &_arg, qApp->arguments()) {
        QString arg = _arg;
        if (arg.startsWith("-connect:")) {
            arg.remove("-connect:");
            Config.HostAddress = arg;
            Config.setValue("HostAddress", arg);

            main_window.startConnection();
            break;
        }
    }

#ifdef Q_OS_WIN
    if (!noGui)
    {
        std::thread t(checkUpdate);
        t.detach();
    }
#endif

    return qApp->exec();
}
