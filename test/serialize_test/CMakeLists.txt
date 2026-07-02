rcom_add_gtest(serialize_test
    serialize_test.cpp
)

target_link_libraries(serialize_test PRIVATE
    RCom_serialize
)
