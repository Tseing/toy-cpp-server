#include "utils.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

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

  // 处理客户端连接
  struct sockaddr_in clnt_addr;
  socklen_t clnt_addr_len = sizeof(clnt_addr);
  bzero(&clnt_addr, sizeof(clnt_addr));

  // 用 accept() 处理连接，accept() 会阻塞，直至有连接建立
  int clnt_sockfd = accept(sockfd, (sockaddr *)&clnt_addr, &clnt_addr_len);
  errif(clnt_sockfd == -1, "socket accept error");

  std::cout << "new client fd " << clnt_sockfd
            << "! IP: " << inet_ntoa(clnt_addr.sin_addr)
            << " Port: " << ntohs(clnt_addr.sin_port) << std::endl;

  while (true) {
    // 定义一个缓冲区
    char buf[1024];
    bzero(&buf, sizeof(buf));

    // 从客户端读取数据到缓冲区，返回读取数据大小
    ssize_t read_bytes = read(clnt_sockfd, buf, sizeof(buf));

    if (read_bytes > 0) {
      std::cout << "message from client fd " << clnt_sockfd << ": " << buf
                << std::endl;
      // 将相同数据写回客户端
      write(clnt_sockfd, buf, sizeof(buf));
    }
    // 返回 0，EOF
    else if (read_bytes == 0) {
      std::cout << "client fd " << clnt_sockfd << " disconnected" << std::endl;
    }
    // 返回 -1，错误
    else if (read_bytes == -1) {
      close(clnt_sockfd);
      errif(true, "socket read error");
    }
  }

  return 0;
}