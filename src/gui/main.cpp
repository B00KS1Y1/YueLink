#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QLockFile>
#include <QMessageBox>
#include <QQmlApplicationEngine>

#include "core/chatservice.h"
#include "gui/lanchatmanager.h"
#include "infrastructure/path.h"
#include "infrastructure/qsettingsidentitystore.h"
#include "infrastructure/runtimebootstrap.h"
#include "infrastructure/sqlitechatrepository.h"
#include "infrastructure/tcpchattransport.h"
#include "infrastructure/udppeerdiscovery.h"

#include <spdlog/spdlog.h>

#include <memory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("YueLink"));
    QCoreApplication::setApplicationName(QStringLiteral("YueLink"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/yuelink-app-icon.png")));

    RuntimeBootstrap::initialize();
    spdlog::info("[应用程序] YueLink 图形界面正在启动");

    if (!QDir().mkpath(Utils::Path::dataDirectory()))
    {
        spdlog::error("[应用程序] 创建数据目录失败");
        QMessageBox::critical(nullptr, QObject::tr("YueLink"), QObject::tr("无法创建 YueLink 数据目录。"));
        RuntimeBootstrap::shutdown();
        return 3;
    }
    QLockFile instanceLock(Utils::Path::dataFile(QStringLiteral("runtime.lock")));
    if (!instanceLock.tryLock())
    {
        spdlog::error("[应用程序] 另一个 YueLink 实例正在使用当前配置");
        QMessageBox::critical(nullptr, QObject::tr("YueLink"), QObject::tr("另一个 YueLink 实例正在使用当前配置。"));
        RuntimeBootstrap::shutdown();
        return 2;
    }

    ChatService service(std::make_unique<UdpPeerDiscovery>(),
                        std::make_unique<TcpChatTransport>(),
                        std::make_unique<SqliteChatRepository>(),
                        std::make_unique<QSettingsIdentityStore>());
    LanChatManager::setService(&service);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            spdlog::critical("[应用程序] 创建 QML 根对象失败");
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.loadFromModule("YueLink", "Main");

    if (!engine.rootObjects().isEmpty())
    {
        const Domain::OperationResult startResult = service.start();
        if (!startResult)
        {
            spdlog::error("[应用程序] 服务启动失败 原因={}", startResult.message.toUtf8().toStdString());
        }
    }

    const int exitCode = QCoreApplication::exec();
    service.stop();
    spdlog::info("[应用程序] YueLink 图形界面已停止 退出码={}", exitCode);
    RuntimeBootstrap::shutdown();
    return exitCode;
}
