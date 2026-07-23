#ifndef CONSOLECONTROLLER_H
#define CONSOLECONTROLLER_H

#include <QObject>
#include <QString>

class ChatService;
class ConsoleRenderer;

namespace Cli
{
struct ParsedCommand;
}

class ConsoleController final : public QObject
{
    Q_OBJECT

public:
    ConsoleController(ChatService *service,
                      ConsoleRenderer *renderer,
                      QObject *parent = nullptr);
    void start();

public slots:
    void handleLine(const QString &line);

signals:
    void quitRequested();

private:
    void connectService();
    void execute(const Cli::ParsedCommand &command);
    void showHelp();
    void showStatus();
    void showPeers(bool onlineOnly);
    void showMessages(int limit);
    void showTransfers();
    void showProfile();
    void showConfig();
    [[nodiscard]] QString resolvePeer(const QString &selector) const;
    void reportResult(const QString &operation,
                      bool succeeded,
                      const QString &code,
                      const QString &message);

    ChatService *m_service = nullptr;
    ConsoleRenderer *m_renderer = nullptr;
    QString m_currentPeerId;
    bool m_quitting = false;
};

#endif // CONSOLECONTROLLER_H
