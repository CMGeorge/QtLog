#ifndef LOG_H
#define LOG_H

#include "log_global.h"

#include <QtDebug>
#include <QObject>
#include <QMutexLocker>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QLoggingCategory>
#include <QDir>


class LOG_EXPORT Log: public QObject {
    Q_OBJECT
public:

    /**
    * @brief Gets the instance of this singleton.
    * @attention MAKE SURE YOU CALL THE init METHOD AT THE BEGINNING OF APP. INSTANCE CREATION IS NOT THREAD SAFE.
    */
    static Log& instance();

    /**
    * @brief Installs the message handler which enables logging to files. If you do not call this method, all logs will
    * be output to stdout without timestamp and using Qt's default pattern.
    * @param logPath the path where to store the log files. If it does not exist, it will be created. Make
    * sure the folder has permissions.
    * @param defaultLogName Name of the file that will contain all logs (without extension. EG: Sanasoft). An extension
    * of type ".log" will be added.
    * @details This method is thread safe and if called once, next calls will have no effect.
    */
    void init(const QString& logPath, const QString& defaultLogName = QStringLiteral("Sanasoft"));

    /**
    * @brief Method that will be used in the message handler to actually log a message to a file and/or stdout.
    * @details This method will create the required file based on the category.
    * @attention THIS METHOD IS ONLY PUBLIC BECAUSE IT IS NEEDED IN THE MESSAGE HANDLER METHOD. DO NOT USE IT DIRECTLY.
    * @param[in] type The type of message (QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtSystemMsg).
    * @param[in] category The log category to write the message into. It corresponds to a certain file. If the file does
    * not exist, it is automatically created.
    * @param[in] file The name of the file where the log was called.
    * @param[in] method Name of the method that generated the log. This will only be set in debug mode.
    * @param[in] line The line of the file that generated the log.
    * @param[in] version The version.
    * @param[in] message The message to log.
    */
    void handleMessage(QtMsgType type, const QString& category, const QString& file, const QString& method,
                       int line, int version, const QString& message);

    /**
    * @brief Destructor that closes opened files and cleans any allocated space.
    */
    ~Log();

    /**
    * @brief Helper method to disable logging.
    * @param use Set to true to use logs, or to false to disable.
    */
    void setUseLogs(bool use);

Q_SIGNALS:

    /**
    * @brief Signal that will be emitted when a log message is received. Use this with Queued Connection.
    */
    void logMessageReceived(QtMsgType type, const QString& message);

private:

    /** @brief Helper class that will hold a category's file and text stream. */
    class CategoryHelper {
    public:
        /**
        * @brief The file of this category instance.
        * @attention THIS CLASS DOES NOT TAKE OWNERSHIP OF THE OBJECT.
        */
        QFile* file;

        /**
        * @brief The text stream of this category instance.
        * @attention THIS CLASS DOES NOT TAKE OWNERSHIP OF THE OBJECT.
        */
        QTextStream* stream;
    };

    /**
    * @brief Opens the log file with the specified name in the specified folder.
    * @details A new entry will be added to the categories map.
    * @param name Name of the file to open. If the file does not exist, it creates it.
    * @return @b True if the file was opened or @b false otherwise.
    */
    bool openLogFile(const QString& name);

    /**
    * @brief Gets the available log file index from the log folder for the specified base log name.
    * @details It will scan the directory for given filename and check the max index. If, for example, the specified
    * file is "test" and the log folder contains test.log and test.log.1, the available log index will be 2. If the
    * maximum log file index from folder is MAX_FILE_INDEX (EG: test.log.MAX_FILE_INDEX), then the available index will
    * be 1 so that a rollover can be performed.
    * @param fileName The base log file name to search for in the log folder.
    */
    quint32 getAvailableLogFileIndex(const QString& fileName);

    /**
    * @brief Method that performs post-log operations. For example, it checks if any file is past MAX_FILE_SIZE and if
    * so it will perform a rollover, renaming the file to file.log.availableIndex, where available index will be
    * computed using getAvailableLogFileIndex.
    */
    void postLog();

    /**
    * @brief Deletes a category, closing the file associated.
    * @param[in] category The category to free.
    */
    void cleanCategory(const QString& category);

    /**
    * @brief The constructor of the singleton class.
    */
    Log();
    Log(const Log&);
    Log& operator=(const Log&);

    /** @brief Mutex used to sync the writing to files.*/
    QMutex mutex;

    /** @brief Map between the category name and the file that corresponds to it. */
    QMap<QString, CategoryHelper*> categories;

    /** @brief Flag indicating if the class was already initialized. */
    bool isInitialized;

    /** @brief Helper var for the full log folder. */
    QString fullLogFolder;

    /** @brief Name of the default logging category (the one that will contain all logs unified). */
    QString defaultCategoryName;

    /** @brief Flag indicating if logs are used. */
    bool useLogs;
};

#endif // LOG_H
