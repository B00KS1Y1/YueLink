QsLog - the simple Qt logger
-------------------------------------------------------------------------------
QsLog is an easy to use logger that is based on Qt's QDebug class.

Features
-------------------------------------------------------------------------------
    * Six logging levels (from trace to fatal)
    * Logging level threshold configurable at runtime.
    * Minimum overhead when logging is turned off.
    * Multiple destinations, comes with file and debug destinations.
    * Thread-safe
    * Logging of common Qt types out of the box.
    * Immediate logging or queueing messages in a separate thread.
    * Small dependency: just drop it in your project directly.

Usage
-------------------------------------------------------------------------------
By directly including QsLog in your project:
    1. Include QsLog.pri in your pro file
    2. Include QsLog.h in your C++ files. Include QsLogDest.h only where you create/add destinations.
    3. Get the instance of the logger by calling QsLogging::Logger::instance();
    4. Optionally set the logging level. Info is default.
    5. Create as many destinations as you want by using the QsLogging::DestinationFactory.
    6. Add the destinations to the logger instance by calling addDestination.
    7. Start logging!
    Note: when you want to use QsLog both from an executable and a shared library you have to
          link dynamically with QsLog due to a limitation with static variables.

By linking to QsLog dynamically:
    1. Build QsLog using the QsLogSharedLibrary.pro.
    2. Add the QsLog shared library to your LIBS project dependencies.
    3. Follow the steps in "directly including QsLog in your project" starting with step 2.

Configuration
-------------------------------------------------------------------------------
QsLog has several configurable parameters:
    * calling Logger::setIncludeSourceLocation(true) enables writing the file and line number
      automatically for each logging call.
    * calling Logger::setUseSeparateThread(true) routes log messages through a dedicated
      single-thread pool.

Sometimes it's necessary to turn off logging. This can be done in several ways:
    * globally, at compile time, by enabling the QS_LOG_DISABLE macro in the .pri file.
    * globally, at run time, by setting the log level to "OffLevel".
    * per file, at compile time, by including QsLogDisableForThisFile.h in the target file.

Thread safety
-------------------------------------------------------------------------------
The Qt docs say: A thread-safe function can be called simultaneously from multiple threads,
even when the invocations use shared data, because all references to the shared data are serialized.
A reentrant function can also be called simultaneously from multiple threads, but only if each
invocation uses its own data.

Since sending the log message to the destinations is protected by a mutex, the logging macros are
thread-safe provided that the log has been initialized - i.e: instance() has been called.
The instance function and the setup functions (e.g: setLoggingLevel, addDestination) are NOT
thread-safe and are NOT reentrant.

IMPORTANT: when using a separate thread for logging, your program might crash at exit time on some
           operating systems if you won't call Logger::destroyInstance() before your program exits.
           This function can be called either before returning from main in a console app or
           inside QCoreApplication::aboutToQuit in a Qt GUI app.
           The reason is that the logging thread is still running as some objects are destroyed by
           the OS. Calling destroyInstance will wait for the thread to finish.
           Nothing will happen if you forget to call the function when not using a separate thread
           for logging.

QsLog——简单的 Qt 日志记录器
-------------------------------------------------------------------------------
QsLog 是一个基于 Qt QDebug 类、易于使用的日志记录器。

功能特性
-------------------------------------------------------------------------------
    * 六个日志级别（从跟踪到致命）
    * 可在运行时配置日志级别阈值。
    * 关闭日志记录时开销极小。
    * 支持多个输出目标，并自带文件和调试输出目标。
    * 线程安全。
    * 开箱即用地支持记录常见 Qt 类型。
    * 可立即写入日志，也可将消息排队后在独立线程中写入。
    * 依赖很少：只需直接将其放入项目中即可。

用法
-------------------------------------------------------------------------------
将 QsLog 直接包含到项目中：
    1. 在 pro 文件中包含 QsLog.pri。
    2. 在 C++ 文件中包含 QsLog.h。只有在创建/添加输出目标的位置才需要包含 QsLogDest.h。
    3. 调用 QsLogging::Logger::instance() 获取日志记录器实例。
    4. 可按需设置日志级别；默认级别为 Info。
    5. 使用 QsLogging::DestinationFactory 创建任意数量的输出目标。
    6. 调用 addDestination 将输出目标添加到日志记录器实例。
    7. 开始记录日志！
    注意：如果需要在可执行文件和共享库中同时使用 QsLog，则受静态变量限制，
          必须动态链接 QsLog。

动态链接 QsLog：
    1. 使用 QsLogSharedLibrary.pro 构建 QsLog。
    2. 将 QsLog 共享库添加到项目的 LIBS 依赖项中。
    3. 从第 2 步开始，按照“将 QsLog 直接包含到项目中”的步骤操作。

配置
-------------------------------------------------------------------------------
QsLog 提供以下几个可配置参数：
    * 调用 Logger::setIncludeSourceLocation(true) 可为每次日志调用自动写入文件名和行号。
    * 调用 Logger::setUseSeparateThread(true) 可通过专用的单线程线程池传递日志消息。

有时需要关闭日志记录，可以通过以下几种方式实现：
    * 全局、编译时：在 .pri 文件中启用 QS_LOG_DISABLE 宏。
    * 全局、运行时：将日志级别设为“OffLevel”。
    * 单个文件、编译时：在目标文件中包含 QsLogDisableForThisFile.h。

线程安全
-------------------------------------------------------------------------------
Qt 文档说明：线程安全函数可以由多个线程同时调用，即使这些调用使用共享数据也可以，
因为对共享数据的所有引用都会被串行化。可重入函数也可以由多个线程同时调用，
但前提是每次调用都使用各自的数据。

由于向输出目标发送日志消息的过程受互斥锁保护，因此，只要日志系统已经初始化
（即已调用 instance()），日志宏就是线程安全的。instance 函数和设置函数
（例如 setLoggingLevel、addDestination）既不是线程安全的，也不是可重入的。

重要：使用独立线程记录日志时，如果程序退出前没有调用 Logger::destroyInstance()，
      程序在某些操作系统上可能会在退出时崩溃。控制台程序可以在 main 返回前调用
      此函数，Qt GUI 程序则可以在 QCoreApplication::aboutToQuit 中调用。
      原因是操作系统销毁部分对象时，日志线程可能仍在运行。调用 destroyInstance
      会等待该线程结束。如果没有使用独立线程记录日志，即使忘记调用此函数也不会
      产生任何影响。
