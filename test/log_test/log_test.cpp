#include "logger.hpp"
#include "logstream.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string ReadFile(const std::filesystem::path& path) {
    std::ifstream input(path);
    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = logger::Logger::get_instance();
        logger_->close();
        logger_->console(false);
        logger_->level(logger::Logger::LOG_DEBUG);
        logger_->max(0);

        const auto suffix =
            std::chrono::steady_clock::now().time_since_epoch().count();
        log_dir_ = std::filesystem::temp_directory_path() /
                   ("rcom_log_test_" + std::to_string(suffix));
        std::filesystem::create_directories(log_dir_);
    }

    void TearDown() override {
        logger_->close();
        std::filesystem::remove_all(log_dir_);
    }

    std::filesystem::path LogPath(const std::string& filename) const {
        return log_dir_ / filename;
    }

    logger::Logger* logger_{nullptr};
    std::filesystem::path log_dir_;
};

} // namespace

TEST_F(LoggerTest, WritesFormattedMessageToFile) {
    const auto path = LogPath("formatted.log");
    ASSERT_NO_THROW(logger_->open(path.string()));

    logger_->log(logger::Logger::LOG_INFO, "log_test.cpp", 42, "value=%d", 7);
    logger_->close();

    const auto contents = ReadFile(path);
    EXPECT_NE(contents.find("INFO"), std::string::npos);
    EXPECT_NE(contents.find("log_test.cpp:42"), std::string::npos);
    EXPECT_NE(contents.find("value=7"), std::string::npos);
}

TEST_F(LoggerTest, FiltersMessagesBelowConfiguredLevel) {
    const auto path = LogPath("filtered.log");
    ASSERT_NO_THROW(logger_->open(path.string()));

    logger_->level(logger::Logger::LOG_WARNING);
    logger_->log(logger::Logger::LOG_DEBUG, "log_test.cpp", 10, "debug");
    logger_->log(logger::Logger::LOG_ERROR, "log_test.cpp", 11, "error");
    logger_->close();

    const auto contents = ReadFile(path);
    EXPECT_EQ(contents.find("debug"), std::string::npos);
    EXPECT_NE(contents.find("ERROR"), std::string::npos);
    EXPECT_NE(contents.find("error"), std::string::npos);
}

TEST_F(LoggerTest, LogStreamWritesOnDestruction) {
    const auto path = LogPath("stream.log");
    ASSERT_NO_THROW(logger_->open(path.string()));

    {
        auto stream =
            logger_->log_stream(logger::Logger::LOG_DEBUG, "log_test.cpp", 88);
        stream << "stream value=" << 9;
    }
    logger_->close();

    const auto contents = ReadFile(path);
    EXPECT_NE(contents.find("DEBUG"), std::string::npos);
    EXPECT_NE(contents.find("stream value=9"), std::string::npos);
}
