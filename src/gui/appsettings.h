#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class AppSettings : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AppSettings)
    QML_SINGLETON
    Q_PROPERTY(QString themeMode READ themeMode NOTIFY settingsChanged)
    Q_PROPERTY(QString primaryColor READ primaryColor NOTIFY settingsChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool notificationsEnabled READ notificationsEnabled NOTIFY settingsChanged)
    Q_PROPERTY(QString logLevel READ logLevel NOTIFY settingsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    [[nodiscard]] QString themeMode() const;
    [[nodiscard]] QString primaryColor() const;
    [[nodiscard]] bool animationsEnabled() const;
    [[nodiscard]] bool notificationsEnabled() const;
    [[nodiscard]] QString logLevel() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool save(const QString &themeMode,
                          const QString &primaryColor,
                          bool animationsEnabled,
                          bool notificationsEnabled,
                          const QString &logLevel);

signals:
    void settingsChanged();
    void lastErrorChanged();

private:
    void setLastError(const QString &error);
    [[nodiscard]] static QString normalizedThemeMode(const QString &mode);
    [[nodiscard]] static QString normalizedLogLevel(const QString &level);

    QString m_themeMode;
    QString m_primaryColor;
    QString m_logLevel;
    QString m_lastError;
    bool m_animationsEnabled = true;
    bool m_notificationsEnabled = true;
};

#endif // APPSETTINGS_H
