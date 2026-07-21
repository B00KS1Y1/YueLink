#ifndef IFILELAUNCHER_H
#define IFILELAUNCHER_H

#include <QString>

class IFileLauncher
{
public:
    virtual ~IFileLauncher() = default;

    [[nodiscard]] virtual bool openFile(const QString &filePath,
                                        QString *errorMessage) = 0;
    [[nodiscard]] virtual bool revealInFolder(const QString &filePath,
                                              QString *errorMessage) = 0;
};

#endif // IFILELAUNCHER_H
