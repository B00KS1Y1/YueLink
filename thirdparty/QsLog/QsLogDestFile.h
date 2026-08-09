#ifndef QSLOGDESTFILE_H
#define QSLOGDESTFILE_H

#include "QsLogDest.h"
#include <QFile>
#include <QTextStream>
#include <QtGlobal>
#include <QSharedPointer>

namespace QsLogging
{
class RotationStrategy
{
public:
    virtual ~RotationStrategy();

    virtual void setInitialInfo(const QFile &file) = 0;
    virtual void includeMessageInCalculation(const QString &message) = 0;
    virtual bool shouldRotate() = 0;
    virtual void rotate() = 0;
    virtual QIODevice::OpenMode recommendedOpenModeFlag() = 0;
};

// Never rotates file, overwrites existing file.
// 从不轮转文件，并覆盖现有文件。
class NullRotationStrategy : public RotationStrategy
{
public:
    void setInitialInfo(const QFile &) override {}
    void includeMessageInCalculation(const QString &) override {}
    bool shouldRotate() override { return false; }
    void rotate() override {}
    QIODevice::OpenMode recommendedOpenModeFlag() override { return QIODevice::Truncate; }
};

// Rotates after a size is reached, keeps a number of <= 10 backups, appends to existing file.
// 达到指定大小后轮转，最多保留 10 个备份，并向现有文件追加内容。
class SizeRotationStrategy : public RotationStrategy
{
public:
    SizeRotationStrategy();
    static const int MaxBackupCount;

    void setInitialInfo(const QFile &file) override;
    void includeMessageInCalculation(const QString &message) override;
    bool shouldRotate() override;
    void rotate() override;
    QIODevice::OpenMode recommendedOpenModeFlag() override;

    void setMaximumSizeInBytes(qint64 size);
    void setBackupCount(int backups);

private:
    QString mFileName;
    qint64 mCurrentSizeInBytes;
    qint64 mMaxSizeInBytes;
    int mBackupsCount;
};

typedef QSharedPointer<RotationStrategy> RotationStrategyPtr;

// file message sink
// 文件消息输出目标。
class FileDestination : public Destination
{
public:
    FileDestination(const QString& filePath, RotationStrategyPtr rotationStrategy);
    void write(const QString& message, Level level) override;
    bool isValid() override;

private:
    QFile mFile;
    QTextStream mOutputStream;
    QSharedPointer<RotationStrategy> mRotationStrategy;
};

}

#endif // QSLOGDESTFILE_H
