#ifndef DESKTOPFILELAUNCHER_H
#define DESKTOPFILELAUNCHER_H

#include "ifilelauncher.h"

class DesktopFileLauncher final : public IFileLauncher
{
public:
    [[nodiscard]] bool openFile(const QString &filePath,
                                QString *errorMessage) override;
    [[nodiscard]] bool revealInFolder(const QString &filePath,
                                      QString *errorMessage) override;
};

#endif // DESKTOPFILELAUNCHER_H
