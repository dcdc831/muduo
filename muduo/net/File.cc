#include <liburing.h>
#include <liburing/io_uring.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstddef>
#include <cstdio>

#include "muduo/net/EventLoop.h"
#include "muduo/net/File.h"

using namespace muduo;
using namespace muduo::net;

File::File(EventLoop* loop, int fileFd)
    : loop_(loop),
      fileFd_(fileFd),
      eventFd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      channel_(loop, eventFd_) {
  setUpIoUring();
  channel_.setReadCallback(std::bind(&File::handleRead, this));
  channel_.enableReading();
}

File::~File() {
  ::close(fileFd_);
  ::close(eventFd_);
}

void File::read() { submitRead(); }

void File::write(const void* data, unsigned int size) {
  submitWrite(data, size);
}

void File::setUpIoUring() {
  io_uring_queue_init(32, &ring_, 0);
  io_uring_register_eventfd(&ring_, eventFd_);
}

void File::submitRead() {
  struct io_uring_sqe* sqe = _io_uring_get_sqe(&ring_);
  memset(buffer_, 0, sizeof(buffer_));
  io_uring_prep_read(sqe, fileFd_, buffer_, sizeof(buffer_), 0);
  sqe->user_data = 1;
  io_uring_submit(&ring_);
}

void File::submitWrite(const void* data, unsigned int size) {
  struct io_uring_sqe* sqe = _io_uring_get_sqe(&ring_);
  io_uring_prep_write(sqe, fileFd_, data, size, 0);
  sqe->user_data = 2;
  io_uring_submit(&ring_);
}

void File::handleRead() {
  uint64_t res;
  ::read(eventFd_, &res, sizeof(res));  // 清除 eventfd 上的事件

  struct io_uring_cqe* cqe;
  while (io_uring_peek_cqe(&ring_, &cqe) == 0) {
    if (cqe->res >= 0) {
      // 处理完成事件
      if (cqe->user_data == 1) {
        // 处理读完成事件
        // 打印buffer_中内容
        printf("read: %s\n", buffer_);
      } else if (cqe->user_data == 2) {
        // 处理写完成事件
        printf("write: %d bytes\n", cqe->res);
      }
    }
    io_uring_cqe_seen(&ring_, cqe);
  }
}