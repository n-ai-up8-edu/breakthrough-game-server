// --------------------//
// n@ai.univ-paris8.fr //
// july 2015           //
// --------------------//
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "common.h"

bool debug = true;

using boost::asio::ip::tcp;

void print_server_reply(char* _str) {
  for(int i = 0; i < strlen(_str); i++) {
    if(_str[i] == ';') _str[i] = '\n';
  }
  printf("%s\n", _str);
}
      
int main(int ac, char* av[]) {
  try {
    if (ac != 3) { 
      fprintf(stderr, "Usage: %s <host> <port>\n", av[0]);
      return 1;
    }

    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({av[1], av[2]}));

    while(true) {
      std::cout << "$client> ";
      char request[MESSAGE_MAX_LENGTH];
      std::cin.getline(request, MESSAGE_MAX_LENGTH);
      if (std::cin.eof()==1) {
	std::cin.clear();
	std::cin.ignore();
	continue;
      }
      sprintf(request, "%s\n", request);
      if(debug) printf(" <<< %s", (char*)request);

      int request_length = (int)strlen(request);
      if(request_length > 4) {
	boost::asio::write(s, boost::asio::buffer(request, request_length));
	boost::asio::streambuf streamreply;
	boost::asio::read_until(s, streamreply, '\n');
	std::istream is(&streamreply);
	std::string reply;
	std::getline(is, reply);
	int req = strncmp(request, "quit", 4);
	int ret = strncmp(reply.c_str(), "=", 1);
	if(req == 0 && ret == 0) break;
	ret = strncmp(reply.c_str(), "TOO MANY PLAYERS CONNECTED", 26);
	if(ret == 0) {
	  printf("%s\n", reply.c_str());
	  printf("bye\n");
	  break;
	}
	if(debug) print_server_reply((char*)reply.c_str());
      }
    }

  } catch (std::exception& e) {
    fprintf(stderr, "Exception: %s\n", e.what()); 
  }

  return 0;
}
