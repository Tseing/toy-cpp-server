#include "utils.h"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

// fcntl(fd, F_GETFL) 取得 fd 的文件状态标志
// fcntl(fd, F_SETFL, cmd) 将 fd 的文件状态标志设置为 cmd
// 将文件描述符设置为非阻塞模式
void setnonblocking(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main() {
  // 初始化 socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  errif(sockfd == -1, "socket create error");

  // 声明 IP 地址结构体，以 0 初始化
  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));

  // 设置地址族、IP地址和端口
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(8008);

  // 将 IP 与 socket 绑定
  errif(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1,
        "socket bind error");

  // 以最大值 SOMAXCONN (128) 监听 socket
  errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

  // 创建 epoll，返回 epoll 的 fd
  int epfd = epoll_create1(0);
  errif(epfd == -1, "epoll create error");

  // 初始化 epoll 事件的容器，ev 为自定义事件
  struct epoll_event events[MAX_EVENTS], ev;
  bzero(&events, sizeof(events));
  bzero(&ev, sizeof(ev));

  // 将事件 fd 指定为 socket fd
  ev.data.fd = sockfd;
  // 当有新的数据到达时，就会触发 EPOLLIN 事件
  // 文件描述符的状态发生变化时，由信号的上升沿触发 EPOLLET 事件
  ev.events = EPOLLIN | EPOLLET;
  setnonblocking(sockfd);

  // 将服务器 socket fd 添加到 epoll
  epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

  while (true) {
    // 非阻塞等待 epoll 中的触发，事件存储在 events 中，返回事件数量
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    errif(nfds == -1, "epoll wait error");

    // 处理触发的事件
    for (int i = 0; i < nfds; i++) {
      // 事件 fd 为服务端 socket fd，处理客户端新连接
      if (events[i].data.fd == sockfd) {
        struct sockaddr_in clnt_addr;
        bzero(&clnt_addr, sizeof(clnt_addr));
        socklen_t clnt_addr_len = sizeof(clnt_addr);

        int clnt_sockfd =
            accept(sockfd, (sockaddr *)&clnt_addr, &clnt_addr_len);
        errif(clnt_sockfd == -1, "socket accept error");

        std::cout << "new client fd " << clnt_sockfd
                  << "! IP: " << inet_ntoa(clnt_addr.sin_addr)
                  << " Port: " << ntohs(clnt_addr.sin_port) << std::endl;

        // 获取了新连接，设置客户端 socket 事件
        bzero(&ev, sizeof(ev));
        ev.data.fd = clnt_sockfd;
        ev.events = EPOLLIN | EPOLLET;

        // 将客户端 socket 也设为设为非阻塞，并加入到 epoll
        setnonblocking(clnt_sockfd);
        epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev);
      }
      // 事件 fd 为客户端，且有新的数据到达，处理客户端数据
      else if (events[i].events & EPOLLIN) {
        char buf[READ_BUFFER];

        // 非阻塞 IO，一次从客户端读取 buf 大小的数据，直至全部读完
        while (true) {
          ssize_t read_bytes = read(events[i].data.fd, buf, sizeof(buf));
          if (read_bytes > 0) {
            std::cout << "message from client df " << events[i].data.fd << ": "
                      << buf << std::endl;

            // 将数据 echo 给客户端
            write(events[i].data.fd, buf, sizeof(buf));
          }
          // 客户端正常中断，继续读取
          else if (read_bytes == -1 && errno == EINTR) {
            std::cout << "continue reading..." << std::endl;
            continue;
          }
          // 非阻塞 IO 中，该信号表示数据全部读取完毕
          else if (read_bytes == -1 &&
                   ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            std::cout << "finish reading once, errno: " << errno << std::endl;
            break;
          }
          // EOF
          else if (read_bytes == 0) {
            // 关闭 socket 后，socket fd 自动从 epoll 树上移除
            close(events[i].data.fd);
            std::cout << "EOF, client fd " << events[i].data.fd
                      << " disconnected" << std::endl;
            break;
          }
          // 其他错误
          else {
            std::cout << "something else happened in reading" << std::endl;
          }
        }

      } else {
        std::cout << "something else happened in epoll" << std::endl;
      }
    }
  }
  close(sockfd);
  return 0;
}