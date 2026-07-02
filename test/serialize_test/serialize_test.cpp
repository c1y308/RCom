#include "data_stream.hpp"
#include "serializable.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

using serialize::DataStream;

enum RoleType {
    ROLE_READER = 1,
    ROLE_WRITER = 2,
};

struct Payload : public serialize::Serializable {
    std::string name;
    std::uint32_t id = 0;
    RoleType role = ROLE_READER;
    std::list<std::int32_t> samples;

    SERIALIZE(name, id, role, samples)
};

struct Envelope : public serialize::Serializable {
    std::int64_t sequence = 0;
    Payload payload;

    SERIALIZE(sequence, payload)
};

TEST(DataStreamTest, RoundTripsPrimitiveTypes) {
    serialize::DataStream stream;
    stream << true
           << std::int32_t{-42}
           << std::uint32_t{42}
           << std::int64_t{-42000000000LL}
           << std::uint64_t{42000000000ULL}
           << 3.5f
           << 7.25
           << std::string()
           << std::string("hello");

    bool bool_value = false;
    std::int32_t int32_value = 0;
    std::uint32_t uint32_value = 0;
    std::int64_t int64_value = 0;
    std::uint64_t uint64_value = 0;
    float float_value = 0.0f;
    double double_value = 0.0;
    std::string empty;
    std::string text;

    EXPECT_TRUE(stream.read(bool_value));
    EXPECT_TRUE(stream.read(int32_value));
    EXPECT_TRUE(stream.read(uint32_value));
    EXPECT_TRUE(stream.read(int64_value));
    EXPECT_TRUE(stream.read(uint64_value));
    EXPECT_TRUE(stream.read(float_value));
    EXPECT_TRUE(stream.read(double_value));
    EXPECT_TRUE(stream.read(empty));
    EXPECT_TRUE(stream.read(text));

    EXPECT_TRUE(bool_value);
    EXPECT_EQ(int32_value, -42);
    EXPECT_EQ(uint32_value, 42U);
    EXPECT_EQ(int64_value, -42000000000LL);
    EXPECT_EQ(uint64_value, 42000000000ULL);
    EXPECT_FLOAT_EQ(float_value, 3.5f);
    EXPECT_DOUBLE_EQ(double_value, 7.25);
    EXPECT_TRUE(empty.empty());
    EXPECT_EQ(text, "hello");
}

TEST(DataStreamTest, CurrentCharReaderRejectsCharMarker) {
    serialize::DataStream stream;
    stream << 'x';

    char value = 0;
    EXPECT_FALSE(stream.read(value));
}

TEST(DataStreamTest, RoundTripsContainersElementByElement) {
    const std::list<std::int32_t> list_values{4, 5, 6};
    const std::set<std::int32_t> set_values{7, 8, 9};
    const std::map<std::string, std::int32_t> map_values{{"one", 1}, {"two", 2}};

    serialize::DataStream stream;
    stream << list_values << set_values << map_values;

    std::list<std::int32_t> list_out;
    std::set<std::int32_t> set_out;
    std::map<std::string, std::int32_t> map_out;

    EXPECT_TRUE(stream.read(list_out));
    EXPECT_TRUE(stream.read(set_out));
    EXPECT_TRUE(stream.read(map_out));

    EXPECT_EQ(list_out, list_values);
    EXPECT_EQ(set_out, set_values);
    EXPECT_EQ(map_out, map_values);
}

TEST(DataStreamTest, ReadsVectorBytesFromCurrentWireFormat) {
    serialize::DataStream stream;
    const char type = serialize::DataStream::VECTOR;
    const char bytes[] = {1, 2, 3};

    stream.write(&type, sizeof(type));
    stream << std::int32_t{3};
    stream.write(bytes, sizeof(bytes));

    std::vector<char> output;
    EXPECT_TRUE(stream.read(output));

    const std::vector<char> expected{1, 2, 3};
    EXPECT_EQ(output, expected);
}

TEST(DataStreamTest, RoundTripsNestedSerializableAndEnum) {
    Envelope input;
    input.sequence = 99;
    input.payload.name = "camera";
    input.payload.id = 7;
    input.payload.role = ROLE_WRITER;
    input.payload.samples = {10, 20, 30};

    serialize::DataStream stream;
    stream << input;

    Envelope output;
    EXPECT_TRUE(stream.read(output));

    EXPECT_EQ(output.sequence, input.sequence);
    EXPECT_EQ(output.payload.name, input.payload.name);
    EXPECT_EQ(output.payload.id, input.payload.id);
    EXPECT_EQ(output.payload.role, input.payload.role);
    EXPECT_EQ(output.payload.samples, input.payload.samples);
}

TEST(DataStreamTest, FailsOnTypeMismatch) {
    serialize::DataStream stream;
    stream << std::int32_t{123};

    bool value = false;
    EXPECT_FALSE(stream.read(value));
}

TEST(DataStreamTest, FailsOnNegativeStringLength) {
    serialize::DataStream stream;
    const char type = serialize::DataStream::STRING;
    stream.write(&type, sizeof(type));
    stream << std::int32_t{-1};

    std::string value;
    EXPECT_FALSE(stream.read(value));
}
