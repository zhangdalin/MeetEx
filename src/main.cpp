#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "login.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

#include <iostream>

QFile logFile;
QTextStream logStream;
QMutex logMutex;

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QMutexLocker locker(&logMutex);

    // write console logs
    std::cout << msg.toStdString() << std::endl;

    if (logFile.isOpen()) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

        QString prefix;
        switch (type) {
        case QtDebugMsg:     prefix = "[DEBUG]"; break;
        case QtInfoMsg:      prefix = "[INFO]"; break;
        case QtWarningMsg:   prefix = "[WARN]"; break;
        case QtCriticalMsg:  prefix = "[ERROR]"; break;
        case QtFatalMsg:     prefix = "[FATAL]"; break;
        }

        logStream << timestamp << " " << prefix << " " << msg << "\n";
        logStream.flush();
    }
}

int main(int argc, char *argv[])
{
    logFile.setFileName("meetex.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        logStream.setDevice(&logFile);
        qInstallMessageHandler(customMessageHandler);
    }

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MeetEx_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    QObject::connect(&a, &QCoreApplication::aboutToQuit, []() {
        // qInfo() << __FUNCTION__ << "If you have something to do before application quit, please add here.";
    });

    auto login = std::make_unique<Login>();
    login->show();

    return a.exec();
}
