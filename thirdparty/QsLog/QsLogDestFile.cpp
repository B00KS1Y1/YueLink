#include "QsLogDestFile.h"
#include <QDateTime>
#include <QStringConverter>
#include <QtGlobal>
#include <iostream>

const int QsLogging::SizeRotationStrategy::MaxBackupCount = 10;

QsLogging::RotationStrategy::~RotationStrategy()
{
}

QsLogging::SizeRotationStrategy::SizeRotationStrategy()
    : mCurrentSizeInBytes(0)
    , mMaxSizeInBytes(0)
    , mBackupsCount(0)
{
}

void QsLogging::SizeRotationStrategy::setInitialInfo(const QFile &file)
{
    mFileName = file.fileName();
    mCurrentSizeInBytes = file.size();
}

void QsLogging::SizeRotationStrategy::includeMessageInCalculation(const QString &message)
{
    mCurrentSizeInBytes += message.toUtf8().size();
}

bool QsLogging::SizeRotationStrategy::shouldRotate()
{
    return mCurrentSizeInBytes > mMaxSizeInBytes;
}

// Algorithm assumes backups will be named filename.X, where 1 <= X <= mBackupsCount.
// All X's will be shifted up.
// 此算法假定备份文件命名为 filename.X，其中 1 <= X <= mBackupsCount；
// 所有 X 值都将依次增大。
void QsLogging::SizeRotationStrategy::rotate()
{
    if (!mBackupsCount) {
        if (!QFile::remove(mFileName))
            std::cerr << "QsLog: backup delete failed " << qPrintable(mFileName);
        return;
    }

     // 1. find the last existing backup than can be shifted up
     // 1. 查找最后一个仍可向后顺移的现有备份。
     const QString logNamePattern = mFileName + QString::fromUtf8(".%1");
     int lastExistingBackupIndex = 0;
     for (int i = 1;i <= mBackupsCount;++i) {
         const QString backupFileName = logNamePattern.arg(i);
         if (QFile::exists(backupFileName))
             lastExistingBackupIndex = qMin(i, mBackupsCount - 1);
         else
             break;
     }

     // 2. shift up
     // 2. 将备份编号依次增大。
     for (int i = lastExistingBackupIndex;i >= 1;--i) {
         const QString oldName = logNamePattern.arg(i);
         const QString newName = logNamePattern.arg(i + 1);
         QFile::remove(newName);
         const bool renamed = QFile::rename(oldName, newName);
         if (!renamed) {
             std::cerr << "QsLog: could not rename backup " << qPrintable(oldName)
                       << " to " << qPrintable(newName);
         }
     }

     // 3. rename current log file
     // 3. 重命名当前日志文件。
     const QString newName = logNamePattern.arg(1);
     if (QFile::exists(newName))
         QFile::remove(newName);
     if (!QFile::rename(mFileName, newName)) {
         std::cerr << "QsLog: could not rename log " << qPrintable(mFileName)
                   << " to " << qPrintable(newName);
     }
}

QIODevice::OpenMode QsLogging::SizeRotationStrategy::recommendedOpenModeFlag()
{
    return QIODevice::Append;
}

void QsLogging::SizeRotationStrategy::setMaximumSizeInBytes(qint64 size)
{
    Q_ASSERT(size >= 0);
    mMaxSizeInBytes = size;
}

void QsLogging::SizeRotationStrategy::setBackupCount(int backups)
{
    Q_ASSERT(backups >= 0);
    mBackupsCount = qMin(backups, SizeRotationStrategy::MaxBackupCount);
}


QsLogging::FileDestination::FileDestination(const QString& filePath, RotationStrategyPtr rotationStrategy)
    : mRotationStrategy(rotationStrategy)
{
    mFile.setFileName(filePath);
    if (!mFile.open(QFile::WriteOnly | QFile::Text | mRotationStrategy->recommendedOpenModeFlag()))
        std::cerr << "QsLog: could not open log file " << qPrintable(filePath);
    mOutputStream.setDevice(&mFile);
    mOutputStream.setEncoding(QStringConverter::Utf8);

    mRotationStrategy->setInitialInfo(mFile);
}

void QsLogging::FileDestination::write(const QString& message, Level)
{
    mRotationStrategy->includeMessageInCalculation(message);
    if (mRotationStrategy->shouldRotate()) {
        mOutputStream.setDevice(NULL);
        mFile.close();
        mRotationStrategy->rotate();
        if (!mFile.open(QFile::WriteOnly | QFile::Text | mRotationStrategy->recommendedOpenModeFlag()))
            std::cerr << "QsLog: could not reopen log file " << qPrintable(mFile.fileName());
        mRotationStrategy->setInitialInfo(mFile);
        mOutputStream.setDevice(&mFile);
    }

    mOutputStream << message << Qt::endl;
    mOutputStream.flush();
}

bool QsLogging::FileDestination::isValid()
{
    return mFile.isOpen();
}

