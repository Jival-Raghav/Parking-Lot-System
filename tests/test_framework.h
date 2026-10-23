#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace test_framework {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& get_registry() {
    static std::vector<TestCase> registry;
    return registry;
}

inline bool& current_test_failed() {
    static bool failed = false;
    return failed;
}

inline int register_test(const std::string& name, std::function<void()> func) {
    get_registry().push_back({name, func});
    return 0;
}

#define TEST_CASE(name) \
    static void name(); \
    static int name##_reg = ::test_framework::register_test(#name, name); \
    static void name()

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "  FAIL: [" #condition "] at " << __FILE__ << ":" << __LINE__ << std::endl; \
            ::test_framework::current_test_failed() = true; \
        } \
    } while (0)

#define CHECK_EQ(a, b) \
    do { \
        auto _va = (a); \
        auto _vb = (b); \
        if (!(_va == _vb)) { \
            std::cerr << "  FAIL: [" #a " == " #b "] at " << __FILE__ << ":" << __LINE__ \
                      << " (left: " << _va << ", right: " << _vb << ")" << std::endl; \
            ::test_framework::current_test_failed() = true; \
        } \
    } while (0)

inline int run_all_tests() {
    int total = static_cast<int>(get_registry().size());
    int passed = 0;
    int failed = 0;

    for (const auto& test : get_registry()) {
        current_test_failed() = false;
        try {
            test.func();
        } catch (const std::exception& e) {
            std::cerr << "  EXCEPTION in " << test.name << ": " << e.what() << std::endl;
            current_test_failed() = true;
        } catch (...) {
            std::cerr << "  UNKNOWN EXCEPTION in " << test.name << std::endl;
            current_test_failed() = true;
        }

        if (current_test_failed()) {
            failed++;
            std::cout << "  [FAIL] " << test.name << std::endl;
        } else {
            passed++;
            std::cout << "  [PASS] " << test.name << std::endl;
        }
    }

    std::cout << "Results: " << passed << " passed, " << failed << " failed, " << total << " total\n";
    return (failed == 0) ? 0 : 1;
}

} // namespace test_framework
