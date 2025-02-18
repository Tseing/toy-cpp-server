#include "utils.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
  // 与 server.cpp 相同的方式初始化 socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  errif(sockfd == -1, "socket create error");

  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(8008);

  // 连接服务端
  errif(connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1,
        "socket connet error");

  while (true) {
    char buf[BUFFER_SIZE];
    bzero(&buf, sizeof(buf));
    // 客户端从键盘读入数据
    std::cin >> buf;

    // 从缓冲区写入服务端
    ssize_t write_bytes = write(sockfd, buf, sizeof(buf));

    if(write_bytes == -1){
      std::cout << "socket already disconnected, cannot write any more!" << std::endl;
      break;
    }

    // 客户端发送清空缓冲区，准备接收
    bzero(&buf, sizeof(buf));
    ssize_t read_bytes = read(sockfd, buf, sizeof(buf));

    // 与 server.cpp 相同方式接收数据
    if (read_bytes > 0) {
      std::cout << "message from server fd " << sockfd << ": " << buf
                << std::endl;
      write(sockfd, buf, sizeof(buf));
    }
    else if (read_bytes == 0) {
      std::cout << "server fd " << sockfd << " disconnected" << std::endl;
    }
    else if (read_bytes == -1) {
      close(sockfd);
      errif(true, "socket read error");
    }
  }

  return 0;
}