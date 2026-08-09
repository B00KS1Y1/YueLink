#include <QCoreApplication>

#include "QtTestUtil/TestRegistry.h"

/**
 * Runs all tests registered with the QtTestUtil registry.
 * 运行所有已在 QtTestUtil 注册表中注册的测试。
 */
int main(int argc, char* argv[]) {
	QCoreApplication application(argc, argv);
	return QtTestUtil::TestRegistry::getInstance()->runTests(argc, argv);
}
