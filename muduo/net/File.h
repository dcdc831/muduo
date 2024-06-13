#include <liburing.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstddef>

#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"

namespace muduo {

namespace net {

class File {
 public:
  File(EventLoop* loop, int fileFd, struct io_uring* ring);
  ~File();

  void read();
  void write(const void* data, unsigned int size);

  char* getBuffer() { return buffer_; }

 private:
  // void setUpIoUring();
  void submitRead();
  void submitWrite(const void* data, unsigned int size);
  void handleRead();

  EventLoop* loop_;
  int fileFd_;
  int eventFd_;
  // Channel channel_;
  struct io_uring* ring_;
  char buffer_[4096];
};

class FileOp {
 public:
  FileOp(int opNum, File* file);
  ~FileOp();

  int opNum_;
  File* file_;
};
}  // namespace net
}  // namespace muduo