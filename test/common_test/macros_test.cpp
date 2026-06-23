#include "macros.hpp"

#include <gtest/gtest.h>

namespace {

struct WithShutdown {
    void shutdown() { ++shutdown_count; }

    int shutdown_count{0};
};

struct WithoutShutdown {
    int value{7};
};

} // namespace

TEST(CommonMacrosTest, CallShutdownInvokesObjectWithShutdownMethod) {
    WithShutdown object;

    call_shutdown(object);

    EXPECT_EQ(object.shutdown_count, 1);
}

TEST(CommonMacrosTest, CallShutdownIgnoresObjectWithoutShutdownMethod) {
    WithoutShutdown object;

    call_shutdown(object);

    EXPECT_EQ(object.value, 7);
}
