rcom_add_gtest(singal_test
    singal_test.cpp
)

target_link_libraries(singal_test PRIVATE
    RCom_base
)
