#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "common.h"

using boost::asio::ip::tcp;

void print_server_reply(char* _str) {
  for(int i = 0; i < strlen(_str); i++) {
    if(_str[i] == ';') _str[i] = '\n';
  }
  printf("---reply::\n%s\n", _str);
}
      

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({argv[1], argv[2]}));

    while(true) {
      std::cout << "Enter message: ";
      char request[MESSAGE_MAX_LENGTH];
      std::cin.getline(request, MESSAGE_MAX_LENGTH);
      sprintf(request, "%s\n", request);
      size_t request_length = std::strlen(request);
      printf("---request::\n%s", request);
      boost::asio::write(s, boost::asio::buffer(request, request_length));
      //char reply[MESSAGE_MAX_LENGTH];
      //int reply_size = 
      //size_t reply_length = boost::asio::read(s, boost::asio::buffer(reply, 10));
      
      printf("---start READ UNTIL\n");
      boost::asio::streambuf streamreply;
      boost::asio::read_until(s, streamreply, '\n');
      printf("---end READ UNTIL\n");
      std::istream is(&streamreply);
      std::string reply;
      std::getline(is, reply);
      int req = strncmp(request, "quit", 4);
      int ret = strncmp(reply.c_str(), "ok", 2);
      if(req == 0 && ret == 0) break;
      ret = strncmp(reply.c_str(), "TOO MANY PLAYERS CONNECTED", 26);
      if(ret == 0) {
	printf("%s\n", reply.c_str());
	printf("---bye ...\n");
	break;
      }
      print_server_reply((char*)reply.c_str());
    }
  }
  catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
    }
  

  return 0;
}
