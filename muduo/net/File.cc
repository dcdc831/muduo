#include <liburing.h>
#include <liburing/io_uring.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/File.h"

using namespace muduo;
using namespace muduo::net;

FileOp::FileOp(int opNum, File* file) : opNum_(opNum), file_(file) {}
FileOp::~FileOp() {}

File::File(EventLoop* loop, int fileFd, struct io_uring* ring)
    : loop_(loop),
      fileFd_(fileFd),
      eventFd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      ring_(ring) {}

File::~File() {
  ::close(fileFd_);
  ::close(eventFd_);
}

void File::read() { submitRead(); }

void File::write(const void* data, unsigned int size) {
  submitWrite(data, size);
}

void File::submitRead() {
  struct io_uring_sqe* sqe = _io_uring_get_sqe(ring_);
  if (sqe == NULL) {
    // TODO: When this happen, sq is full. Considering to set a flag to refuse
    // the comming read submit.
    LOG_TRACE << "io_uring_get_sqe failed";
    printf("io_uring_get_sqe failed\n");
    return;
  } else {
    memset(buffer_, 0, sizeof(buffer_));
    io_uring_prep_read(sqe, fileFd_, buffer_, sizeof(buffer_), 0);
    FileOp* fileOp = new FileOp(1, this);
    sqe->user_data = uintptr_t(fileOp);
    io_uring_submit(ring_);
  }
}

void File::submitWrite(const void* data, unsigned int size) {
  struct io_uring_sqe* sqe = _io_uring_get_sqe(ring_);
  if (sqe == NULL) {
    LOG_TRACE << "io_uring_get_sqe failed";
    printf("io_uring_get_sqe failed\n");
    return;
  } else {
    io_uring_prep_write(sqe, fileFd_, data, size, 0);
    FileOp* fileOp = new FileOp(2, this);
    sqe->user_data = uintptr_t(fileOp);
    io_uring_submit(ring_);
  }
}
