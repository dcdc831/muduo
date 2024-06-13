#include <liburing/io_uring.h>
#include <unistd.h>

#include <thread>

#include "muduo/net/EventLoop.h"
#include "muduo/net/File.h"

int main() {
  muduo::net::EventLoop loop;

  int fd = ::open("testfile.txt", O_RDWR | O_CREAT, 0666);
  muduo::net::File ioFile(&loop, fd);

  // ioFile.read();
  // ioFile.write("Hello, io_uring!", 16);

  // loop.loop();

  // 测试功能：使用多线程形式，一个线程运行loop.loop()，另一个线程进行ioFile.read()和ioFile.write()操作

  std::thread ioThread([&]() {
    while (1) {
      ioFile.read();
      ioFile.write("Hello, io_uring!", 16);
    }
  });

  loop.loop();

  ioThread.join();

  // pid_t pid = fork();
  // if (pid == 0) {
  //   // 子进程
  //   while (1) {
  //     ioFile.read();
  //     ioFile.write("Hello, io_uring!", 16);
  //   }
  // } else {
  //   // 父进程

  //   loop.loop();
  // }
  return 0;
}