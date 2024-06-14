#include <liburing.h>
#include <liburing/io_uring.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstddef>
#include <cstdio>

#include "muduo/net/FileRing.h"

using namespace muduo;
using namespace muduo::net;

FileRing::FileRing(EventLoop* loop)
    : loop_(loop),
      eventFd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      channel_(loop_, eventFd_) {
  setUpIoUring();
  channel_.setReadCallback(std::bind(&FileRing::handRead, this));
  channel_.enableReading();
}

FileRing::~FileRing() { ::close(eventFd_); }

void FileRing::setUpIoUring() {
  io_uring_queue_init(64, &ring_, 0);
  io_uring_register_eventfd(&ring_, eventFd_);
}

void FileRing::handRead() {
  uint64_t res;
  ::read(eventFd_, &res, sizeof(res));  // 清除 eventfd 上的事件

  struct io_uring_cqe* cqe;
  int ret = io_uring_peek_cqe(&ring_, &cqe);
  if (ret == 0) {
    if (cqe->res >= 0) {
      FileOp* fileOp = reinterpret_cast<FileOp*>(cqe->user_data);
      if (fileOp->opNum_ == 1) {
        printf("read: %s\n", fileOp->file_->getBuffer());
      } else if (fileOp->opNum_ == 2) {
        printf("write: %d bytes\n", cqe->res);
      }
    }
    io_uring_cqe_seen(&ring_, cqe);
  }
}

File* FileRing::registerFile(int fileFd) {
  File* file = new File(loop_, fileFd, &(this->ring_));
  files_.push_back(file);
  return file;
}