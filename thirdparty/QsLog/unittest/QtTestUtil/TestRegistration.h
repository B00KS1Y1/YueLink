#ifndef QtTestUtil_TestRegistration_H
#define QtTestUtil_TestRegistration_H

#include "QtTestUtil/TestRegistry.h"

namespace QtTestUtil {

	/**
	 * A wrapper class around a test to manage registration and static
	 * creation of an instance of the test class.
	 * This class is used by QTTESTUTIL_REGISTER_TEST(), and you should not 
	 * use this class directly.
	 * 测试包装类，用于管理测试注册以及测试类实例的静态创建。
	 * QTTESTUTIL_REGISTER_TEST() 会使用此类，不应直接使用本类。
	 */
	template<typename TestClass>
	class TestRegistration {
		public:
			TestRegistration() {
				test_ = new TestClass();
				TestRegistry::getInstance()->registerTest(test_);
			}

			~TestRegistration() {
				delete test_;
			}
		
		private:
			TestClass* test_;
	};

}

#endif
