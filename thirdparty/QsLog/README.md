## QsLog - the simple Qt logger ##
QsLog is an easy to use logger that is based on Qt's QDebug class. QsLog is released as open source, under the MIT license. 

###Contribution policy###
Bug fixes are welcome, larger changes however are not encouraged at this point due to the lack of time on my side for reviewing and integrating them. Your best bet in this case would be to open a ticket for your change or forking the project and implementing your change there, with the possibility of having it integrated in the future. 
All contributions will be credited, license of the contributions should be MIT. 

### Features ###
* Six logging levels (from trace to fatal)
* Logging level threshold configurable at runtime.
* Minimum overhead when logging is turned off.
* Supports multiple destinations, comes with file and debug destinations.
* Thread-safe
* Supports logging of common Qt types out of the box.
* Small dependency: just drop it in your project directly.

### Usage ###
* Include QsLog.h. Include QsLogDest.h only where you create/add destinations.
* Get the instance of the logger by calling QsLogging::Logger::instance();
* Optionally set the logging level. Info is default.
* Create as many destinations as you want by using the QsLogging::DestinationFactory.
* Add the destinations to the logger instance by calling addDestination.

**Note**: The logger does not take ownership of the destinations. Make sure that the destinations still exist when you call one of the logging macros. A good place to create the destinations is the program's main function.

### Disabling logging ###
Sometimes it's necessary to turn off logging. This can be done in several ways:

* globally, at compile time, by enabling the QS_LOG_DISABLE macro in the supplied .pri file.
* globally, at run time, by setting the log level to "OffLevel".
* per file, at compile time, by including QsLogDisableForThisFile.h in the target file.

### Thread safety ###
The Qt docs say: 
A **thread-safe** function can be called simultaneously from multiple threads, even when the invocations use shared data, because all references to the shared data are serialized.
A **reentrant** function can also be called simultaneously from multiple threads, but only if each invocation uses its own data.

Since sending the log message to the destinations is protected by a mutex, the logging macros are thread-safe provided that the log has been initialized - i.e: instance() has been called. 
The instance function and the setup functions (e.g: setLoggingLevel, addDestination) are NOT thread-safe and are NOT reentrant.

## QsLog——简单的 Qt 日志记录器

QsLog 是一个基于 Qt QDebug 类、易于使用的日志记录器。QsLog 以 MIT 许可证作为开源软件发布。

### 贡献政策

欢迎提交错误修复。不过，由于我目前没有足够时间审查和整合较大的改动，因此暂不鼓励提交此类改动。遇到这种情况，最合适的做法是为你的改动创建工单，或者派生本项目并在派生版本中实现改动；这些改动将来仍有可能被合并。

所有贡献都会得到署名，且贡献内容应采用 MIT 许可证。

### 功能特性

* 六个日志级别（从跟踪到致命）
* 可在运行时配置日志级别阈值。
* 关闭日志记录时开销极小。
* 支持多个输出目标，并自带文件和调试输出目标。
* 线程安全。
* 开箱即用地支持记录常见 Qt 类型。
* 依赖很少：只需直接将其放入项目中即可。

### 用法

* 包含 QsLog.h。只有在创建/添加输出目标的位置才需要包含 QsLogDest.h。
* 调用 QsLogging::Logger::instance() 获取日志记录器实例。
* 可按需设置日志级别；默认级别为 Info。
* 使用 QsLogging::DestinationFactory 创建任意数量的输出目标。
* 调用 addDestination 将输出目标添加到日志记录器实例。

**注意**：日志记录器不取得输出目标的所有权。调用任一日志宏时，请确保输出目标仍然存在。程序的 main 函数通常是创建这些输出目标的合适位置。

### 禁用日志记录

有时需要关闭日志记录，可以通过以下几种方式实现：

* 全局、编译时：在随附的 .pri 文件中启用 QS_LOG_DISABLE 宏。
* 全局、运行时：将日志级别设为“OffLevel”。
* 单个文件、编译时：在目标文件中包含 QsLogDisableForThisFile.h。

### 线程安全

Qt 文档说明：

**线程安全**函数可以由多个线程同时调用，即使这些调用使用共享数据也可以，因为对共享数据的所有引用都会被串行化。

**可重入**函数也可以由多个线程同时调用，但前提是每次调用都使用各自的数据。

由于向输出目标发送日志消息的过程受互斥锁保护，因此，只要日志系统已经初始化（即已调用 instance()），日志宏就是线程安全的。

instance 函数和设置函数（例如 setLoggingLevel、addDestination）既不是线程安全的，也不是可重入的。
