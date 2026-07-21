#ifndef PATH_H
#define PATH_H

#include <QString>

namespace Utils::Path
{

QString configDirectory();
QString configFile(const QString &fileName);
QString dataDirectory();
QString dataFile(const QString &fileName);

} // namespace Utils::Path

#endif // PATH_H
