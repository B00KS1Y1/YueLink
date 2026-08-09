#ifndef QtTestUtil_H
#define QtTestUtil_H

#include <QObject>
#include <QtTest/QtTest>
#include "QtTestUtil/TestRegistration.h"

/**
 * A macro to register a test class.
 * 用于注册测试类的宏。
 *
 * This macro will create a static variable which registers the
 * testclass with the TestRegistry, and creates an instance of the 
 * test class.
 * 此宏会创建一个静态变量，用于向 TestRegistry 注册测试类，
 * 并创建该测试类的实例。
 *
 * Execute this macro in the body of your unit test's .cpp file, e.g.
 * 请在单元测试的 .cpp 文件主体中使用此宏，例如：
 *    class MyTest {
 *			...
 *		};
 *
 *		QTTESTUTIL_REGISTER_TEST(MyTest)
 */
#define QTTESTUTIL_REGISTER_TEST(TestClass) \
	static QtTestUtil::TestRegistration<TestClass> TestClass##Registration

#endif
