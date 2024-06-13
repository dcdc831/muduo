#include <vector>

#include "muduo/net/Channel.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/File.h"

namespace muduo {

namespace net {

class FileRing {
 public:
  FileRing(EventLoop* loop);
  ~FileRing();
  File* registerFile(int fileFd);

 private:
  void setUpIoUring();
  void handRead();
  EventLoop* loop_;
  int eventFd_;
  Channel channel_;
  io_uring ring_;
  std::vector<File*> files_;
};
}  // namespace net

}  // namespace muduo