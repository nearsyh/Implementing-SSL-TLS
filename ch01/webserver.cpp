#include <cstdlib>
#include <cerrno>
#include <string>
#include <sys/types.h>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif
#include "base64.h"

const int HTTP_PORT = 80;
const int DEFAULT_LINE_LEN = 255;

std::string read_line(int connection) {
  static int line_len = DEFAULT_LINE_LEN;
  std::string line = "";
  char ch;
  while(true) {
    int size = read(connection, &ch, 1);
    if(size == -1) {
      perror("Error - recv from socket");
      return "";
    } else if(size == 0) {
      return line;
    } else {
      if(ch == '\n') return line.substr(0, line.length() - 1);
      else line += ch;
    }
  }
}

static void build_success_response(int connection) {
  char buf[DEFAULT_LINE_LEN];
  sprintf(buf, "HTTP/1.1 200 Success\r\n"
      "Connection: Close\r\n"
      "Content-Type:text/html\r\n"
      "\r\n"
      "<html><head><title>Test Page</title></head><body>Nothing here</body></html>"
      "\r\n");

  // Technically, this should account for short writes.
  if(send(connection, buf, strlen(buf), 0) < strlen(buf))
    perror("Trying to respond");
}

static void build_error_response(int connection, int error_code) {
  char buf[ 255 ];
  sprintf(buf, "HTTP/1.1 %d Error Occurred\r\n\r\n", error_code);

  // Technically, this should account for short writes.
  if(send(connection, buf, strlen(buf), 0 ) < strlen(buf))
    perror( "Trying to respond" );
}

static void process_http_request(int connection) {
  std::string request_line = read_line( connection );
  if(request_line.substr(0, 3) != "GET") {
    // Only supports "GET" requests
    build_error_response( connection, 501 );
  } else {
    // Skip over all header lines, don't care
    while(read_line(connection) != "") ;
    build_success_response(connection);
  }

#ifdef WIN32
  if(closesocket(connection) == -1)
#else
  if(close(connection) == -1)
#endif
    perror( "Unable to close connection" );
}

int main( int argc, char *argv[ ] ) {
  int listen_sock;
  int connect_sock;
  int on = 1;
  struct sockaddr_in local_addr;
  struct sockaddr_in client_addr;
  unsigned int client_addr_len = sizeof(client_addr);
#ifdef WIN32
  WSADATA wsaData;

  if(WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
    perror( "Unable to initialize winsock" );
    exit( 0 );
  }
#endif

  if((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    perror( "Unable to create listening socket" );
    exit( 0 );
  }

  if(setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    perror( "Setting socket option" );
    exit( 0 );
  }

  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons( HTTP_PORT );
  local_addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );

  if(bind(listen_sock, (sockaddr*) &local_addr, sizeof(local_addr)) == -1) {
    perror( "Unable to bind to local address" );
    exit( 0 );
  }

  if(listen(listen_sock, 5) == -1) {
    perror( "Unable to set socket backlog" );
    exit( 0 );
  }

  while((connect_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_addr_len)) != -1) {
    // TODO: ideally, this would spawn a new thread.
    process_http_request( connect_sock );
  }

  if(connect_sock == -1) {
    perror( "Unable to accept socket" );
  }

  return 0;
}
