#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "application/chatcoordinator.h"
#include "infrastructure/path.h"
#include "infrastructure/runtimebootstrap.h"
#include "infrastructure/sqlitechatrepository.h"
#include "infrastructure/tcpchattransport.h"
#include "infrastructure/udppeerdiscovery.h"
#include "YueLink/lanchatmanager.h"

#include <QyLog.h>

#include <memory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QCoreApplication::setOrganizationName(QStringLiteral("YueLink"));
    QCoreApplication::setApplicationName(QStringLiteral("YueLink"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/yuelink/assets/yuelink-app-icon.png")));

    RuntimeBootstrap::initialize();
    QLOG_INFO() << QStringLiteral("[应用程序] YueLink 图形界面正在启动");

    if (!QDir().mkpath(Path::dataDirectory()))
    {
        QLOG_ERROR() << QStringLiteral("[应用程序] 创建数据目录失败");
        QMessageBox::critical(nullptr, QObject::tr("YueLink"), QObject::tr("无法创建 YueLink 数据目录。"));
        RuntimeBootstrap::shutdown();
        return 3;
    }
    ChatCoordinator coordinator(std::make_unique<UdpPeerDiscovery>(), std::make_unique<TcpChatTransport>(), std::make_unique<SqliteChatRepository>());
    LanChatManager::setCoordinator(&coordinator);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            QLOG_FATAL() << QStringLiteral("[应用程序] 创建 QML 根对象失败");
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.loadFromModule("YueLink", "Main");

    if (!engine.rootObjects().isEmpty())
    {
        const Domain::OperationResult startResult = coordinator.start();
        if (!startResult)
        {
            QLOG_ERROR() << QStringLiteral("[应用程序] 服务启动失败 原因=") << startResult.message;
        }
    }

    const int exitCode = QCoreApplication::exec();
    coordinator.stop();
    QLOG_INFO() << QStringLiteral("[应用程序] YueLink 图形界面已停止 退出码=") << exitCode;
    RuntimeBootstrap::shutdown();
    return exitCode;
}
