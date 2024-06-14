#include <gtest/gtest.h>
#include <liburing/io_uring.h>
#include <unistd.h>

#include <thread>

#include "muduo/net/EventLoop.h"
#include "muduo/net/FileRing.h"

// Function prototypes
void io_uring_test(muduo::net::File* file, const char* write_data,
                   unsigned int write_size);

class IoUringTest : public ::testing::Test {
 protected:
  void SetUp() override {
    loop = new muduo::net::EventLoop();
    fd = ::open("testfile.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
    fd1 = ::open("testfile1.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
    // Initialize file contents
    ::write(fd, "This is testfile1", 17);
    ::write(fd1, "This is testfile2", 17);

    fileRing = new muduo::net::FileRing(loop);
    file = fileRing->registerFile(fd);
    file1 = fileRing->registerFile(fd1);
  }

  void TearDown() override {
    ::close(fd);
    ::close(fd1);
    delete fileRing;
    delete loop;
  }

  muduo::net::EventLoop* loop;
  int fd;
  int fd1;
  muduo::net::FileRing* fileRing;
  muduo::net::File* file;
  muduo::net::File* file1;
};

void io_uring_test(muduo::net::File* file, const char* write_data,
                   unsigned int write_size) {
  file->read();
  file->write(write_data, write_size);
}

TEST_F(IoUringTest, ReadWriteContentTest) {
  std::thread ioThread1([&]() {
    // test if file contents are read and written correctly
    io_uring_test(file, "Hello, io_uring1!", 17);

    sleep(3);
    // 测试buffer中读出的内容是否与初始化内容一致，以及此时文件内容是否与写入内容一致
    EXPECT_STREQ(file->getBuffer(), "This is testfile1");
    char buffer[4096];
    ::lseek(fd, 0, SEEK_SET);
    ::read(fd, buffer, 17);
    EXPECT_STREQ(buffer, "Hello, io_uring1!");
  });

  loop->runAfter(5.0, [this]() { loop->quit(); });
  loop->loop();
  ioThread1.join();
}

TEST_F(IoUringTest, ConcurrentReadWriteTests) {
  std::thread ioThread([&]() {
    for (int i = 0; i < 10; i++) {
      sleep(1);
      io_uring_test(file, "Hello, io_uring!", 16);
    }
  });

  std::thread ioThread1([&]() {
    for (int i = 0; i < 10; i++) {
      sleep(1);
      io_uring_test(file1, "Hello, io_uring1!", 17);
    }
  });
  loop->runAfter(20, [this]() { loop->quit(); });
  loop->loop();
  ioThread.join();
  ioThread1.join();

  // Add assertions or checks here to validate the correctness
  // of the read and write operations.
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
