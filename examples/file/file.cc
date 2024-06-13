#include <liburing/io_uring.h>
#include <unistd.h>

#include <thread>

#include "muduo/net/EventLoop.h"
#include "muduo/net/FileRing.h"

int main() {
  muduo::net::EventLoop loop;

  int fd = ::open("testfile.txt", O_RDWR | O_CREAT, 0666);
  int fd1 = ::open("testfile1.txt", O_RDWR | O_CREAT, 0666);

  muduo::net::FileRing fileRing(&loop);

  muduo::net::File* file = fileRing.registerFile(fd);
  muduo::net::File* file1 = fileRing.registerFile(fd1);

  // Test, two separate threads to read and write two different files
  // concomitantly.

  std::thread ioThread([&]() {
    while (1) {
      sleep(1);
      file->read();
      file->write("Hello, io_uring!", 16);
    }
  });
  std::thread ioThread1([&]() {
    while (1) {
      sleep(1);
      file1->read();
      file1->write("Hello, io_uring1!", 17);
    }
  });

  loop.loop();

  ioThread.join();
  ioThread1.join();

  return 0;
}