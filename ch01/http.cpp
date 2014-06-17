/**
 * http.c - Frank Song
 * 2014 June 16
 *
 * Based on Implementing SSL/TLS Using Cryptography and PKI
 */

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <iostream>
#include <regex>
#include "base64.h"
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif
using namespace std;

/*
 * Accept a uri. Retrive the host and the path
 */
int parse_url(string uri, string& host, string& path) {
  smatch m;
  if(!regex_match(uri, m, std::regex("(http://)?([^/]*)(/(.*))?"))) return -1;
  host = m[2].str();
  path = m[4].str();
  return 0;
}

/*
 * Parse the proxy url and get host, username, password and port
 */
int parse_proxy(string proxy, string& host, string username, string& password, int& port) {
  smatch m;
  if(!regex_match(proxy, m, std::regex("(http://)?(([^:]*)(:([^@]*))?@)?([^:]*)(:([0-9]*))?"))) return -1;
  username = m[3].str();
  password = m[5].str();
  host = m[6].str();
  port = m[8].str() == "" ? 80 : atoi(m[8].str().c_str());
  return 0;
}

const int MAX_GET_COMMAND = 255;
/**
 * HTTP get method
 */
int http_get(int connection, string path, string host, string proxy_host, string proxy_user, string proxy_passwd) {
  static char get_command[MAX_GET_COMMAND];
  if(proxy_host == "") sprintf(get_command, "GET /%s HTTP/1.1\r\n", path.c_str());
  else sprintf(get_command, "GET http://%s/%s HTTP1.1\r\n", host.c_str(), path.c_str());
  if(send(connection, get_command, strlen(get_command), 0) == -1) return -1;
  sprintf(get_command, "Host: %s\r\n", host.c_str());
  if(send(connection, get_command, strlen(get_command), 0) == -1) return -1;

  if (proxy_user != "") {
    sprintf(get_command, "Proxy-Authorization: BASIC %s\r\n", base64_encode(proxy_user + ":" + proxy_passwd).c_str());
    if(send(connection, get_command, strlen(get_command), 0) == -1) return -1;
  }

  sprintf(get_command, "Connection: close\r\n\r\n");
  if(send(connection, get_command, strlen(get_command), 0) == -1) return -1;
  return 0;
}

const int BUFFER_SIZE = 255;
/*
 * output the result
 */
void display_result(int connection) {
  int received = 0;
  static char recv_buf[BUFFER_SIZE + 1];
  while((received = recv(connection, recv_buf, BUFFER_SIZE, 0)) > 0) {
    recv_buf[received] = '\0';
    cout << recv_buf;
  }
  cout << endl;
}

void usage(char *cmd) {
  cerr << "Usage : " << cmd << " : [-p http://[username:passwd@]proxy-host:proxy-port] -h <URL>\n" << endl;
  exit(1);
}

const int HTTP_PORT = 80;

int main(int argc, char** argv) {
  int client_connection;
  string url, proxy;
  struct hostent *host_name;
  struct sockaddr_in host_address;
  int index;
#ifdef WIN32
  WSDATA wsaData;
#endif

  url = proxy = "";
  index = 1;
  int ch;
  while((ch = getopt(argc, argv, "h:p:")) != -1) {
    switch(ch) {
      case 'p':
        proxy = optarg;
        break;
      case 'h':
        url = optarg;
        break;
      default:
        usage(argv[0]);
    }
  }
  if(url == "") usage(argv[0]);

  string host, path;
  if(parse_url(url, host, path) == -1) {
    cerr << "Error - malformed URL " << url << endl;
    return 1;
  }

  string proxy_host, proxy_user, proxy_password;
  int proxy_port;
  if(proxy != "" && parse_proxy(proxy, proxy_host, proxy_user, proxy_password, proxy_port) == -1) {
    cerr << "Error - malformed Proxy " << url << endl;
  }

  if(proxy_host != "") {
    cout << "Connecting to host " << proxy_host << endl;
    host_name = gethostbyname(proxy_host.c_str());
  } else {
    cout << "Connecting to host " << host << endl;
    host_name = gethostbyname(host.c_str());
  }

  if(!host_name) {
    perror("Error in name resolution");
    return 3;
  }

  host_address.sin_family = AF_INET;
  host_address.sin_port = htons(proxy_host != "" ? proxy_port : HTTP_PORT);
  memcpy(&host_address.sin_addr, host_name->h_addr_list[0], sizeof(in_addr));

#ifdef WIN32
  if(WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
    cerr << "Error, unable to initialize winsock" << endl;
    return 2;
  }
#endif
  client_connection = socket(PF_INET, SOCK_STREAM, 0);
  if(!client_connection) {
    perror("Unable to create local socket");
    return 2;
  }

  if(connect(client_connection, (sockaddr*)&host_address, sizeof(host_address)) == -1) {
    perror("Unable to connect to host");
    return 4;
  }

  cout << "Retrieving document : " << path << endl;

  http_get(client_connection, path, host, proxy_host, proxy_user, proxy_password);
  display_result(client_connection);
  cout << "Shutting down." << endl;

#ifdef WIN32
  if(closesocket(client_connection) == -1)
#else
  if(close(client_connection) == -1)
#endif
  {
    perror("Error closing client connection");
    return 5;
  }

#ifdef WIN32
  WSACleanup();
#endif
  return 0;
}
