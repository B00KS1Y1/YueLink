#ifndef QtTestUtil_TestRegistry_H
#define QtTestUtil_TestRegistry_H

#include <QList>

class QObject;

namespace QtTestUtil {
	
	/**
	 * A registry of QtTest test classes.
	 * All test classes registered with QTTESTUTIL_REGISTER_TEST add 
	 * themselves to this registry. All registered tests can then be run at 
	 * once using runTests().
	 * QtTest 测试类的注册表。
	 * 所有通过 QTTESTUTIL_REGISTER_TEST 注册的测试类都会将自身加入此注册表；
	 * 随后可使用 runTests() 一次性运行全部已注册测试。
	 */
	class TestRegistry {
		public:
			/**
			 * Retrieve the single instance of the registry.
			 * 获取注册表的唯一实例。
			 */
			static TestRegistry* getInstance();

			/**
			 * Register a QtTest test. 
			 * This method is called  by QTTESTUTIL_REGISTER_TEST, and you should 
			 * not use this method directly.
			 * 注册一个 QtTest 测试。
			 * 此方法由 QTTESTUTIL_REGISTER_TEST 调用，不应直接调用。
			 */
			void registerTest(QObject*);

			/**
			 * Run all registered tests using QTest::qExec()
			 * 使用 QTest::qExec() 运行所有已注册测试。
			 */
			int runTests(int argc, char* argv[]);

		private:
			TestRegistry() {}
		
		private:
			QList<QObject*> tests_;
	};
}

#endif
