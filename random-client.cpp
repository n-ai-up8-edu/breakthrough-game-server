// ---------------------------------------------//
// n@ai.univ-paris8.fr                          //
// july 2015                                    //
//                                              //
// simple random client that only moves forward //
// ---------------------------------------------//
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <random>
#include <iterator>
#include <boost/asio.hpp>
#include "common.h"

bool debug = false;

using boost::asio::ip::tcp;

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter _start, Iter _end, RandomGenerator& _g) {
  std::uniform_int_distribution<> dis(0, std::distance(_start, _end) - 1);
  std::advance(_start, dis(_g));
  return _start;
}

template<typename Iter>
Iter select_randomly(Iter _start, Iter _end) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return select_randomly(_start, _end, gen);
}

void print_server_reply(char* _str) {
  for(int i = 0; i < strlen(_str); i++) {
    if(_str[i] == ';') _str[i] = '\n';
  }
  printf("%s\n", _str);
}
      
std::list<int> parseShowBoardMessage(char* _str) {
  std::list<int> positions;
  char turn = _str[2];
  int forward = BOARD_EDGE;
  if(turn == WHITE) forward = -BOARD_EDGE;

  for(int i = 0; i < BOARD_SIZE; i++) {
    if(_str[i+3] == turn) {
      if(i+3+forward < BOARD_SIZE && i+3+forward >= 0) {
	if(_str[i+3+forward] != turn) {
	  positions.push_back(i);
	}
      }
   }
  }
  return positions;
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

    char sbuf[MESSAGE_MAX_LENGTH];
    boost::asio::streambuf rbuf;
    boost::asio::streambuf streamreply;
    std::istream is(&streamreply);
    std::string reply;

    printf("identifying as aze player\n");
    boost::asio::write(s, boost::asio::buffer((char*)"id aze 111\n", 11));
    boost::asio::read_until(s, streamreply, '\n');
    std::getline(is, reply);

    printf("creating a game\n");
    boost::asio::write(s, boost::asio::buffer((char*)"new_game o\n", 11));
    boost::asio::read_until(s, streamreply, '\n');
    std::getline(is, reply);

    printf("waiting for another player to join the game\n");
    // wait for someone to join the game
    while(true) {
      boost::asio::write(s, boost::asio::buffer((char*)"get_state\n", 10));
      boost::asio::read_until(s, streamreply, '\n');
      std::getline(is, reply);
      if(strncmp(reply.c_str(), "= 2", 3) == 0) break;
      sleep(1); // to wait 1 sec between each asio::write 
    }

    // play randomly forward
    printf("playing randomly forward\n"); fflush(stdout);
    while(true) {
      boost::asio::write(s, boost::asio::buffer((char*)"get_state\n", 10));
      boost::asio::read_until(s, streamreply, '\n');
      std::getline(is, reply);
      // if state is PS_WIN1 or PS_WIN2
      if(strncmp(reply.c_str(), "= 3", 3) == 0 ||
	 strncmp(reply.c_str(), "= 4", 3) == 0) {
	break;
      }
      // if state is PS_IN_GAME
      if(strncmp(reply.c_str(), "= 2", 3) == 0) {
	boost::asio::write(s, boost::asio::buffer((char*)"get_turn\n", 9));
	boost::asio::read_until(s, streamreply, '\n');
	std::getline(is, reply);
	if(strncmp(reply.c_str(), "= you", 5) == 0) {
	  boost::asio::write(s, boost::asio::buffer((char*)"get_board_str\n", 14));
	  boost::asio::read_until(s, streamreply, '\n');
	  std::getline(is, reply);
	  std::list<int> mypawns = parseShowBoardMessage((char*)reply.c_str());
	  if(mypawns.size() == 0) {
	    printf("mypawns.size() == 0\n");
	  } else {
	    int rpawn = *select_randomly(mypawns.begin(), mypawns.end());
	    int raw = rpawn/BOARD_EDGE;
	    int col = rpawn%BOARD_EDGE;
	    // 0=A0 7=A7 8=B0 ...
	    char msg[MESSAGE_MAX_LENGTH];
	    sprintf(msg, "play %c%d:%c%d\n", 'A'+raw, col, 'A'+raw-1, col); 
	    printf("sending : %s", msg);
	    boost::asio::write(s, boost::asio::buffer((char*)msg, strlen(msg)));
	    boost::asio::read_until(s, streamreply, '\n');
	    std::getline(is, reply);
	  }
	}
      }
      sleep(1); // to wait 1 sec between each asio::write
    }

    printf("game ends. quitting server\n");
    boost::asio::write(s, boost::asio::buffer((char*)"quit\n", 5));
    boost::asio::read_until(s, streamreply, '\n');
    std::getline(is, reply);
    
  } catch (std::exception& e) {
    fprintf(stderr, "Exception: %s\n", e.what()); 
  }
  
  return 0;
}
