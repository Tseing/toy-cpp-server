#include <arpa/inet.h>
#include <cstring>

int main() {
  // 与 server.cpp 相同的方式初始化 socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(8008);

  // 连接服务端
  connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr));

  return 0;
}