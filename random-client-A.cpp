// ----------------------------------------------//
// n@ai.univ-paris8.fr                           //
// july 2015                                     //
//                                               //
// simple random client that only moves forward  //
// (create new game, wait for opponent and play) //
// ----------------------------------------------//
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "common.h"
#include "common-client.h"

bool debug = false;

using boost::asio::ip::tcp;

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
      if(strncmp(reply.c_str(), "= 3", 3) == 0) {
	printf(" == win ==\n");
	break;
      }
      if(strncmp(reply.c_str(), "= 4", 3) == 0) {
	printf(" == lost ==\n");
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
	  std::list<std::pair<int,int> > mymoves = board_2_moves((char*)reply.c_str());
	  if(mymoves.size() == 0) {
	    printf("mymoves.size() == 0\n");
	  } else {
	    std::pair<int, int> rmove = *select_randomly(mymoves.begin(), mymoves.end());
	    int raw_i = rmove.first/BOARD_EDGE;
	    int col_i = rmove.first%BOARD_EDGE;
	    int raw_f = rmove.second/BOARD_EDGE;
	    int col_f = rmove.second%BOARD_EDGE;

	    char msg[MESSAGE_MAX_LENGTH];
	    sprintf(msg, "play %c%d:%c%d\n", 'A'+col_i, BOARD_EDGE-raw_i, 'A'+col_f, BOARD_EDGE-raw_f); 
	    printf(" ___ %s", msg);
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
