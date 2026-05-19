include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(unbounded_queue_test
    unbounded_queue_test.cpp
)
target_link_libraries(unbounded_queue_test PRIVATE
    RCom_base
    gtest_main
)
add_test(NAME unbounded_queue_test COMMAND unbounded_queue_test)

add_executable(bounded_queue_test
    bounded_queue_test.cpp
)
target_link_libraries(bounded_queue_test PRIVATE
    RCom_base
    gtest_main
)
add_test(NAME bounded_queue_test COMMAND bounded_queue_test)

add_executable(macros_test
    macros_test.cpp
)
target_link_libraries(macros_test PRIVATE
    RCom_base
    gtest_main
)
add_test(NAME macros_test COMMAND macros_test)

add_executable(atomic_rw_lock_test
    atomic_rw_lock_test.cpp
)
target_link_libraries(atomic_rw_lock_test PRIVATE
    RCom_base
    gtest_main
)
add_test(NAME atomic_rw_lock_test COMMAND atomic_rw_lock_test)
