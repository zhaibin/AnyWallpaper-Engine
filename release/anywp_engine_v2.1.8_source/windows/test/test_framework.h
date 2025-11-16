#ifndef ANYWP_ENGINE_TEST_FRAMEWORK_H_
#define ANYWP_ENGINE_TEST_FRAMEWORK_H_

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace anywp_engine {
namespace test {

/**
 * Simple unit test framework for C++ modules
 * 
 * Usage:
 *   TEST_SUITE("ModuleName") {
 *     TEST_CASE("Test description") {
 *       ASSERT_TRUE(condition);
 *       ASSERT_EQUAL(expected, actual);
 *     }
 *   }
 */

class TestRunner {
public:
  static TestRunner& Instance() {
    static TestRunner instance;
    return instance;
  }
  
  using TestFunc = std::function<void()>;
  
  void AddTest(const std::string& suite, const std::string& name, TestFunc func) {
    tests_.push_back({suite, name, func});
  }
  
  int Run() {
    int passed = 0;
    int failed = 0;
    
    std::cout << "\n========================================\n";
    std::cout << "Running AnyWP Engine Unit Tests\n";
    std::cout << "========================================\n\n";
    
    for (const auto& test : tests_) {
      std::cout << "[" << test.suite << "] " << test.name << "... ";
      
      try {
        test.func();
        std::cout << "✅ PASSED\n";
        passed++;
      } catch (const AssertionError& e) {
        std::cout << "❌ FAILED\n";
        std::cout << "  Error: " << e.what() << "\n";
        failed++;
      } catch (const std::exception& e) {
        std::cout << "❌ ERROR\n";
        std::cout << "  Exception: " << e.what() << "\n";
        failed++;
      }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "Test Results:\n";
    std::cout << "  Total:  " << (passed + failed) << "\n";
    std::cout << "  Passed: " << passed << " ✅\n";
    std::cout << "  Failed: " << failed << " ❌\n";
    std::cout << "========================================\n\n";
    
    return failed;
  }
  
  class AssertionError : public std::runtime_error {
  public:
    AssertionError(const std::string& msg) : std::runtime_error(msg) {}
  };
  
private:
  struct Test {
    std::string suite;
    std::string name;
    TestFunc func;
  };
  
  std::vector<Test> tests_;
};

// Test macros
#define TEST_SUITE(suite_name) \
  namespace test_##suite_name { \
    static const char* current_suite_name = #suite_name; \
  } \
  namespace test_##suite_name

#define TEST_CASE(test_name) \
  void test_##test_name(); \
  static struct test_##test_name##_registrar { \
    test_##test_name##_registrar() { \
      anywp_engine::test::TestRunner::Instance().AddTest( \
        current_suite_name, #test_name, test_##test_name); \
    } \
  } test_##test_name##_registrar_instance; \
  void test_##test_name()

// Assertion macros
#define ASSERT_TRUE(condition) \
  if (!(condition)) { \
    throw anywp_engine::test::TestRunner::AssertionError( \
      "Assertion failed: " #condition); \
  }

#define ASSERT_FALSE(condition) \
  if (condition) { \
    throw anywp_engine::test::TestRunner::AssertionError( \
      "Assertion failed: !(" #condition ")"); \
  }

#define ASSERT_EQUAL(expected, actual) \
  if ((expected) != (actual)) { \
    throw anywp_engine::test::TestRunner::AssertionError( \
      "Assertion failed: " #expected " == " #actual); \
  }

#define ASSERT_NOT_EQUAL(expected, actual) \
  if ((expected) == (actual)) { \
    throw anywp_engine::test::TestRunner::AssertionError( \
      "Assertion failed: " #expected " != " #actual); \
  }

#define ASSERT_NULL(ptr) \
  if ((ptr) != nullptr) { \
    throw anywp_engine::test::TestRunner::AssertionError( \
      "Assertion failed: " #ptr " is not null"); \
  }

#define ASSERT_NOT_NULL(ptr) \
  if ((ptr) == nullptr) { \
    throw anywp_engine::test::TestRunner::AssertionError( \
      "Assertion failed: " #ptr " is null"); \
  }

}  // namespace test
}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_TEST_FRAMEWORK_H_


