#include "log.h"

#include <iostream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <memory>
using namespace std;
#undef _HAS_AUTO_PTR_ETC
#define _HAS_AUTO_PTR_ETC 1

/** @brief The time format that will go into the logs. */
#define DEFAULT_TIME_FORMAT "yyyy-MM-dd hh:mm:ss:zzz"

/** @brief Creates a string that will be represented in color on standard output. */
#define MAKE_COLOR_STRING(string, color) QString("\033[%1m%2\033[0m").arg(color).arg(string)

/** @brief The extension to use on log files. */
#define DEFAULT_LOG_EXTENSION ".log"

/** @brief Maximum log file index size before rolling over. */
#define MAX_FILE_INDEX 100

/** @brief Maximum size of a log file before creating a new one (bytes). */
#define MAX_FILE_SIZE 2 * 500 * 1024 // 10 MB

#define GREEN 32
#define YELLOW 33
#define RED 31
#define BLUE 34

#ifndef DELETE_OBJECT

#define DELETE_OBJECT(pointer) \
  if (pointer) { \
    delete pointer; \
    pointer = nullptr; \
  }
#endif

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    Log::instance().handleMessage(type, context.category, context.file, context.function, context.line, context.version,
                                  msg);
}

Log& Log::instance() {
    static Log log;
    return log;
}

void Log::init(const QString& logPath, const QString& defaultLogName) {
    if (logPath.isEmpty())
        throw invalid_argument("Invalid log path provided");

    if (defaultLogName.isEmpty())
        throw invalid_argument("Invalid log name provided for the default log file");

    QMutexLocker locker(&mutex);

    defaultCategoryName = defaultLogName;

    if (isInitialized)
        return;

    try {
        fullLogFolder = logPath;
        std::unique_ptr<QDir> appFolder(new QDir());

        if (appFolder->mkpath(fullLogFolder) && openLogFile(defaultCategoryName)) {
            qInstallMessageHandler(messageHandler);
            isInitialized = true;
        }
        else {
            qFatal("Could not create log folder or could not open default log file");
        }
    }
    catch(bad_alloc&) {
        throw bad_alloc();
    }
    catch(...) {
        qFatal("Could not install log message handler. Check if provided folder has permissions.");
    }
}

void Log::handleMessage(QtMsgType type,
                        const QString& category,
                        const QString& file,
                        const QString& method,
                        int line,
                        int version,
                        const QString& message) {

    Q_UNUSED(file)
    Q_UNUSED(line)
    if (message.isEmpty())
        return;

    QMutexLocker locker(&mutex);

    if (!useLogs)
        return;

    QString location;

#ifdef QT_DEBUG
    location+= QString(" (Location: %1:%2)").arg(file).arg(QString::number(line));
#endif

    QString logMessage(QDateTime::currentDateTime().toString(DEFAULT_TIME_FORMAT));

    switch(type) {
    case QtDebugMsg: {
        logMessage += " DBG " + category + ": " + message + location;
        cout << MAKE_COLOR_STRING(logMessage, GREEN).toStdString() << endl;

        break;
    }
    case QtInfoMsg: {
        logMessage += " INF " + category + ": " + message + location;
        cout << logMessage.toStdString() << endl;

        break;
    }
    case QtWarningMsg: {
        logMessage += " WRN " + category + ": " + message + location;
        cout << MAKE_COLOR_STRING(logMessage, YELLOW).toStdString() << endl;

        break;
    }
    case QtCriticalMsg: {
        logMessage += " CRI " + category + ": " + message + location;
        cout << MAKE_COLOR_STRING(logMessage, RED).toStdString() << endl;

        break;
    }
    case QtFatalMsg: {
        logMessage += " FAT " + category + ": " + message + location;
        cout << MAKE_COLOR_STRING(logMessage, RED).toStdString() << endl;

        break;
    }
    }

    cout.flush();

    // If default log file creation failed, then we don't get here so it's safe.
    (*categories[defaultCategoryName]->stream) << logMessage << Qt::endl;

    Q_EMIT logMessageReceived(type, logMessage);

    if (category != "default" && category != "main") {
        if (!categories.contains(category)) {
            if (openLogFile(category))
                (*categories[category]->stream) << logMessage << Qt::endl;
        }
        else {
            (*categories[category]->stream) << logMessage << Qt::endl;
        }
    }

    postLog();

    Q_UNUSED(method)
    Q_UNUSED(version)
}

Log::~Log() {
    for (QMap<QString, CategoryHelper*>::iterator i = categories.begin(); i != categories.end(); i++) {
        cleanCategory(i.key());
    }
}

void Log::setUseLogs(bool use) {
    QMutexLocker locker(&mutex);

    useLogs = use;
}

bool Log::openLogFile(const QString& name) {
    std::unique_ptr<QFile> logFile;
    std::unique_ptr<QTextStream> stream;

    try {
        logFile.reset(new QFile(QString("%1/%2%3").arg(fullLogFolder).arg(name).arg(DEFAULT_LOG_EXTENSION)));

        if (logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            stream.reset(new QTextStream(logFile.get()));

            CategoryHelper* helper = new CategoryHelper();

            helper->file = logFile.release();
            helper->stream = stream.release();

            categories[name] = helper;

            return true;
        }
        else {
            qCritical() << (QString("Log - Could not open file: %1 - Reason: %2").arg(name).arg(logFile->errorString()));
            return false;
        }
    }
    catch (bad_alloc&) {
        throw bad_alloc();
    }
    catch (...) {
        qCritical() << QString("Log - Could not create log: %1").arg(name);
        return false;
    }    
}

quint32 Log::getAvailableLogFileIndex(const QString& fileName) {
    QDir logDir(fullLogFolder);

    QStringList nameFilters(fileName + QString("%1.*").arg(DEFAULT_LOG_EXTENSION));

    QFileInfoList files = logDir.entryInfoList(nameFilters, QDir::Files | QDir::NoSymLinks |
                                               QDir::NoDotAndDotDot);

    quint32 availableIndex = 1;

    for (qint32 i = 0; i < files.count(); i++) {
        // Get the last section from the log filename. E.G: from main.log.1 will return 1.
        QString indexSection(files[i].fileName().section('.', -1));

        if (!indexSection.isEmpty()) {
            quint32 fileIndex = 0;
            bool conversionOK = false;

            fileIndex = indexSection.toUInt(&conversionOK);

            if (conversionOK && fileIndex >= availableIndex) {
                availableIndex = fileIndex + 1;
            }
        }
    }

    if (availableIndex > MAX_FILE_INDEX) {
        availableIndex = 1;
    }

    return availableIndex;
}

void Log::postLog() {
    // Check if any file needs rollover and do it.
    for (QMap<QString, CategoryHelper*>::iterator i = categories.begin(); i != categories.end(); i++) {
        if (i.value()->file->size() > MAX_FILE_SIZE) {
            QString key(i.key());
            QString newFileName(QString("%1/%2.log.%3").arg(fullLogFolder).arg(key).arg(
                                    QString::number(getAvailableLogFileIndex(key))));

            QFile newFile(newFileName);

            bool canContinue = true;

            if (newFile.exists()) {
                if (!newFile.remove())
                    canContinue = false;
            }

            if (canContinue) {
                if (categories[key]->file->rename(newFileName)) {
                    cleanCategory(key);
                    openLogFile(key);
                }
                else {
                    qCritical() << "Log - Could not rename log file.";
                }
            }
            else {
                qCritical() << "Log - Could not delete existing log file.";
            }
        }
    }
}

void Log::cleanCategory(const QString& category) {
    CategoryHelper* helper = categories[category];

    if (helper) {
        if (helper->stream) {
            helper->stream->flush();
        }

        if (helper->file) {
            helper->file->close();
        }

        DELETE_OBJECT(helper->stream)
        DELETE_OBJECT(helper->file)
    }

    DELETE_OBJECT(helper)
}

Log::Log(): QObject(nullptr), isInitialized(false), fullLogFolder("."), useLogs(true) {
}
