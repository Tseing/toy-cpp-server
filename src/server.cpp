#include <arpa/inet.h>
#include <cstring>
#include <iostream>

int main() {
  // 初始化 socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // 声明 IP 地址结构体，以 0 初始化
  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));

  // 设置地址族、IP地址和端口
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(8008);

  // 将 IP 与 socket 绑定
  bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr));

  // 以最大值 SOMAXCONN (128) 监听 socket
  listen(sockfd, SOMAXCONN);

  // 处理客户端连接
  struct sockaddr_in clnt_addr;
  socklen_t clnt_addr_len = sizeof(clnt_addr);
  bzero(&clnt_addr, sizeof(clnt_addr));

  // 用 accept() 处理连接，accept() 会阻塞，直至有连接建立
  int clnt_sockfd = accept(sockfd, (sockaddr *)&clnt_addr, &clnt_addr_len);

  std::cout << "new client fd " << clnt_sockfd
            << "! IP: " << inet_ntoa(clnt_addr.sin_addr)
            << " Port: " << ntohs(clnt_addr.sin_port) << std::endl;

  return 0;
}