#include "func_trait.hpp"

#include <gtest/gtest.h>

DEFINE_TYPE_TRAIT(HasFoo, foo)
DEFINE_TYPE_TRAIT(HasBar, bar)

struct WithFoo {
    void foo() {}
};

struct WithFooAndBar {
    void foo() {}
    void bar() {}
};

struct WithoutFoo {
    void baz() {}
};

struct WithFooOverloaded {
    void foo() {}
    void foo(int) {}
};

struct WithInheritedFoo : WithFoo {};

TEST(MacrosTypeTraitTest, DetectsMemberFunction) {
    EXPECT_TRUE(HasFoo<WithFoo>::value);
    EXPECT_TRUE(HasFoo<WithFooAndBar>::value);
}

TEST(MacrosTypeTraitTest, DetectsAbsenceOfMemberFunction) {
    EXPECT_FALSE(HasFoo<WithoutFoo>::value);
    EXPECT_FALSE(HasBar<WithFoo>::value);
}

TEST(MacrosTypeTraitTest, OverloadedMemberFunctionNotDetected) {
    EXPECT_FALSE(HasFoo<WithFooOverloaded>::value);
}

TEST(MacrosTypeTraitTest, DetectsInheritedMemberFunction) {
    EXPECT_TRUE(HasFoo<WithInheritedFoo>::value);
}

TEST(MacrosTypeTraitTest, PrimitiveTypesHaveNoMemberFunction) {
    EXPECT_FALSE(HasFoo<int>::value);
    EXPECT_FALSE(HasFoo<double>::value);
}
