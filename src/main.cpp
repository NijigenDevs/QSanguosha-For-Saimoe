#include <QFile>
#include <QCoreApplication>
#include <QApplication>
#include <QTranslator>
#include <QDateTime>
#include <QSplashScreen>
#include <QMessageBox>
#include <random>

#include "server.h"
#include "settings.h"
#include "engine.h"
#include "mainwindow.h"
#include "audio.h"
#include "stylehelper.h"

#ifdef Q_OS_WIN
#include <thread>
#include "UpdateChecker.h"
#endif

#ifndef WINDOWS
#include <QDir>
#endif

#ifdef USE_BREAKPAD
#include <client/windows/handler/exception_handler.h>
#include <QProcess>

using namespace google_breakpad;
#endif

int main(int argc, char *argv[])
{
    bool noGui = argc > 1 && strcmp(argv[1], "-server") == 0;
    new Settings;
    if (noGui)
    {
        new QCoreApplication(argc, argv);
    }
    else
    {
        Config.scale();
        if (Config.ScaleFactor <= 0)
        {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
            QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
        }
        else
        {
            qputenv("QT_SCALE_FACTOR", QByteArray::number(Config.ScaleFactor));
        }
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
    std::mt19937 engine(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    std::uniform_int_distribution<> dis(1, UINT_MAX);
    qsrand(dis(engine));

    // load the main translation file first for we need to translate messages of splash.
    QTranslator translator;
    translator.load("sanguosha.qm");
    qApp->installTranslator(&translator);

    showSplashMessage(QSplashScreen::tr("Loading translation..."));
    QTranslator qt_translator;
    qt_translator.load("qt_zh_CN.qm");
    qApp->installTranslator(&qt_translator);

    showSplashMessage(QSplashScreen::tr("Initializing game engine..."));
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
		if (argc >= 4)
		{
			int port = atoi(argv[2]);
			if (port <= 0 || port > 65535)
				exit(2);

			Server *server = new Server(qApp, argv[3]);
			printf("Server is starting on port %u\n", port);

			if (server->listen(port))
				printf("Starting successfully\n");
			else
				printf("Starting failed!\n");
		}
		else
		{
			Server *server = new Server(qApp);
			printf("Server is starting on port %u\n", Config.ServerPort);

			if (server->listen())
				printf("Starting successfully\n");
			else
				printf("Starting failed!\n");
		}

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
    // only gui application can update by wyupdate
    if (!noGui)
    {
        std::thread t(UpdateChecker::CheckUpdate, false);
        t.detach();
    }
#endif

    return qApp->exec();
}
