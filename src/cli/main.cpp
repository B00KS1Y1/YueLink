#include "consolecontroller.h"
#include "consoleinput.h"
#include "consolerenderer.h"
#include "core/chatservice.h"
#include "infrastructure/path.h"
#include "infrastructure/qsettingsidentitystore.h"
#include "infrastructure/runtimebootstrap.h"
#include "infrastructure/sqlitechatrepository.h"
#include "infrastructure/tcpchattransport.h"
#include "infrastructure/udppeerdiscovery.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QLockFile>
#include <QTextStream>
#include <QTimer>

#include <csignal>
#include <memory>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace
{
volatile std::sig_atomic_t InterruptRequested = 0;

void handleSignal(int)
{
    InterruptRequested = 1;
}
} // namespace

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("YueLink"));
    QCoreApplication::setApplicationName(QStringLiteral("YueLink"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("YueLink 局域网聊天终端客户端"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption jsonLinesOption(
        QStringLiteral("jsonl"),
        QStringLiteral("以一行一个 JSON 对象的格式输出事件。"));
    const QCommandLineOption noColorOption(
        QStringLiteral("no-color"),
        QStringLiteral("禁用终端颜色输出。"));
    parser.addOption(jsonLinesOption);
    parser.addOption(noColorOption);
    parser.process(app);

    RuntimeBootstrap::initialize();

    if (!QDir().mkpath(Utils::Path::dataDirectory()))
    {
        QTextStream(stderr) << QStringLiteral("无法创建 YueLink 数据目录。")
                            << Qt::endl;
        RuntimeBootstrap::shutdown();
        return 3;
    }

    QLockFile instanceLock(Utils::Path::dataFile(QStringLiteral("runtime.lock")));
    if (!instanceLock.tryLock())
    {
        QTextStream(stderr)
            << QStringLiteral("另一个 YueLink 实例正在使用当前配置。")
            << Qt::endl;
        RuntimeBootstrap::shutdown();
        return 2;
    }

    ChatService service(std::make_unique<UdpPeerDiscovery>(),
                        std::make_unique<TcpChatTransport>(),
                        std::make_unique<SqliteChatRepository>(),
                        std::make_unique<QSettingsIdentityStore>());
    ConsoleRenderer renderer(parser.isSet(jsonLinesOption));
    const Domain::OperationResult startResult = service.start();
    if (!startResult)
    {
        renderer.error(startResult.code, startResult.message);
        RuntimeBootstrap::shutdown();
        return 5;
    }

    ConsoleController controller(&service, &renderer);
    auto *input = new ConsoleInput(&app);
    QObject::connect(input,
                     &ConsoleInput::lineRead,
                     &controller,
                     &ConsoleController::handleLine,
                     Qt::QueuedConnection);
    QObject::connect(input,
                     &ConsoleInput::inputClosed,
                     &app,
                     &QCoreApplication::quit,
                     Qt::QueuedConnection);
    QObject::connect(&controller,
                     &ConsoleController::quitRequested,
                     &app,
                     &QCoreApplication::quit);

    std::signal(SIGINT, handleSignal);
#if defined(SIGTERM)
    std::signal(SIGTERM, handleSignal);
#endif
    QTimer signalTimer;
    signalTimer.setInterval(100);
    QObject::connect(&signalTimer, &QTimer::timeout, &app, [&app]() {
        if (InterruptRequested != 0)
        {
            app.quit();
        }
    });
    signalTimer.start();

    controller.start();
    input->start();
    const int exitCode = QCoreApplication::exec();
    service.stop();
    RuntimeBootstrap::shutdown();
    return exitCode;
}
