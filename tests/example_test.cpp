#include <gtest/gtest.h>

TEST(ExampleTest, BasicAssertion) {
    EXPECT_EQ(7 * 6, 42);
}

TEST(ExampleTest, StringComparison) {
    std::string hello = "Hello";
    EXPECT_EQ(hello, "Hello");
}