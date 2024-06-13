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
  File(EventLoop* loop, int fileFd);
  ~File();

  void read();
  void write(const void* data, unsigned int size);

 private:
  void setUpIoUring();
  void submitRead();
  void submitWrite(const void* data, unsigned int size);
  void handleRead();

  EventLoop* loop_;
  int fileFd_;
  int eventFd_;
  Channel channel_;
  struct io_uring ring_;
  char buffer_[4096];
};
}  // namespace net
}  // namespace muduo