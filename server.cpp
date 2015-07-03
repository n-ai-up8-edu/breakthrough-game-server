#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include "common.h"
#include <list>
using boost::asio::ip::tcp;

std::list<int> players_id;

#define MAX_GAME 2
#define BOARD_EDGE 8
#define BOARD_SIZE BOARD_EDGE*BOARD_EDGE

#define FREE_SPACE '.'
#define BLACK '@'
#define WHITE 'o'

char inv_col(char _col) {
  if(_col == WHITE) return BLACK;
  if(_col == BLACK) return WHITE;
  return _col;
}

struct game_info_t {
  char player1_color;
  char turn;
  int player1_id;
  int player1_key;
  char player1_name[128];
  int player2_id;
  int player2_key;
  char player2_name[128];
  char board[BOARD_SIZE];
};

struct games_t {
  game_info_t* infos;

  games_t() {
    infos = new game_info_t[MAX_GAME];
    for(int i = 0; i < MAX_GAME; i++) {
      infos[i].player1_color = WHITE;
      infos[i].turn = FREE_SPACE;
      infos[i].player1_id = -1;
      infos[i].player1_key = -1;
      sprintf(infos[i].player1_name, "%s", (char*)"");
      infos[i].player2_id = -1;
      infos[i].player2_key = -1;
      sprintf(infos[i].player2_name, "%s", (char*)"");
      init_board(i);
    }
  }
  ~games_t() {
    delete(infos);
  }

  void print_state() {
    printf("---nb_player: %d\n", (int)players_id.size());
    for(int i = 0; i < MAX_GAME; i++) {
      if(infos[i].player1_id != -1 || infos[i].player1_key != -1 || 
	 infos[i].player2_id != -1 || infos[i].player2_key != -1) {

	char p2_color = inv_col(infos[i].player1_color);

	printf("---\t%d:%d:%c\t%d:%d:%c %c\n", 
	       infos[i].player1_id, infos[i].player1_key,
	       infos[i].player1_color, 
	       infos[i].player2_id, infos[i].player2_key,
	       p2_color,
	       infos[i].turn);
      }
    }
  }

  void quit(int _id, int _key) {
    players_id.remove(_id);
    for(int i = 0; i < MAX_GAME; i++) {
      if(infos[i].player1_id == _id || infos[i].player1_key == _key) {
	infos[i].player1_id = -1; infos[i].player1_key = -1;
	infos[i].player2_id = -1; infos[i].player2_key = -1;
      }
      if(infos[i].player2_id == _id || infos[i].player2_key == _key) {
	infos[i].player1_id = -1; infos[i].player1_key = -1;
	infos[i].player2_id = -1; infos[i].player2_key = -1;
      }
    }
  }

  bool player_identification(int _pid, char*_pname);
  bool player_key_involved(int _player_key);
  bool player_id_involved(int _player_id);
  bool new_game(int& _game_id);
  bool join_game(int _game_id);

  int get_game(int _player_id);

  void init_board(int _gid);
  void get_board(int _gid, char* _board);
};
games_t all_games;

struct score_t {
  int win;
  int lost;
  int abort;
};

void get_score(int _pkey, score_t& _s) {
  _s.win = 0;
  _s.lost = 0;
  _s.abort = 0;
  if(_pkey == -1) return;
  FILE* fp;
  if ((fp = fopen(score_filename,"r")) == 0) {
    fprintf(stderr, "fopen %s error\n", score_filename);
    return;
  }
  int i_pkey = -1;
  char pname[128];
  int all_players_size = (int)(sizeof(all_players)/sizeof(*all_players));
  for(int i = 0; i < all_players_size; i++) {
    if(sscanf(all_players[i], "%d:%s", &i_pkey, pname) == 2) {
      if(_pkey == i_pkey) {
	break;
      }
    } 
  }
  if(i_pkey != _pkey) return;

  char line[1024];
  while(fgets( line, sizeof(line), fp) ) {
    score_t pscore;
    char spname[128];
    if(sscanf(line,"%d %d %d %s", 
	      &pscore.win, 
	      &pscore.lost, 
	      &pscore.abort,
	      spname) == 4) {
      if(strncmp(pname, spname, 64) == 0) {
	_s.win = pscore.win;
	_s.lost = pscore.lost;
	_s.abort = pscore.abort;
	fclose(fp);
	return;
      }
    }
  }
  fclose(fp);
} 
void set_score(int _pkey, score_t _s) {
  
} 

// [0:9][a:z][A:Z][-_@.~]
bool allowed_char(char _char) {
  if(_char >= '0' && _char <= '9') return true;
  if(_char >= 'a' && _char <= 'z') return true;
  if(_char >= 'A' && _char <= 'Z') return true;
  if(_char == '-' || _char == '_' || _char == '@' ||
     _char == '.' || _char == '~' ) return true;
  return false;
}
bool player_name_is_valide(char* _pname) {
  int pname_size = strlen(_pname);
  if(pname_size < 1) return false;
  if(pname_size > 64) return false;
  for(int i = 0; i < pname_size; i++) {
    if(allowed_char(_pname[i]) == false) return false;
  }
  return true;
}

// return false when new_game is NOT ALLOWED
bool parse_new_game(int _id, char _in[MESSAGE_MAX_LENGTH - 8], int& _pkey, int& _new_game_id) {
  char color;
  int key;
  char name[128];
  int r = sscanf(_in, "%c %d:%s", &color, &key, name);
  if( r != 3) return false;
  if(color != WHITE & color != BLACK) return false;
  if(all_games.player_identification(key, name) == false) return false;
  if(all_games.player_key_involved(key) == true) return false;
  if(all_games.player_id_involved(_id) == true) return false;
  _pkey = key;
  if(all_games.new_game(_new_game_id) == false) return false;
  all_games.infos[_new_game_id].player1_color = color;
  all_games.infos[_new_game_id].turn = WHITE;
  all_games.infos[_new_game_id].player1_id = _id;
  all_games.infos[_new_game_id].player1_key = key;
  sprintf(all_games.infos[_new_game_id].player1_name, "%s", name);
  return true;
}

bool parse_join_game(int _pid, char _in[MESSAGE_MAX_LENGTH - 8], int& _pkey) {
  int gid;
  int key;
  char name[128];
  int r = sscanf(_in, "%d %d:%s", &gid, &key, name);
  if( r != 3) return false;
  if(all_games.player_identification(key, name) == false) return false;
  if(all_games.player_key_involved(key) == true) return false;
  if(all_games.player_id_involved(_pid) == true) return false;
  _pkey = key;
  if(all_games.join_game(gid) == false) return false;
  all_games.infos[gid].player2_key = key;
  all_games.infos[gid].player2_id = _pid;
  all_games.infos[gid].turn = WHITE;
  sprintf(all_games.infos[gid].player2_name, "%s", name);
  return true;
}
 
void parse_command(int& _player_id, int& _player_key, char _in[MESSAGE_MAX_LENGTH]) {
  if(_player_id == -1) {
    sprintf(_in, "TOO MANY PLAYERS CONNECTED\n");
    return;
  }
  if(strncmp(_in, "new_game", 8) == 0) {
    int new_game_id = -1;
    if(parse_new_game(_player_id, &_in[9], _player_key, new_game_id) == false) {
      sprintf(_in, "NOK\n");
      return;
    }
    sprintf(_in, "game_id: %2d\n", new_game_id);
    return;
  } 
  if(strncmp(_in, "list_game", 9) == 0) {
    char msg[156];
    int msg_size = 0;
    sprintf(msg, "game list;\tID\tPLAYER_ID\tCOLOR;");
    for(int i = 0; i < MAX_GAME; i++) {
      if(all_games.infos[i].player1_id != -1 && all_games.infos[i].player2_id == -1) {
	char tmp[32];
	sprintf(tmp, "\t%d\t%d\t\t%c;", i, all_games.infos[i].player1_id, all_games.infos[i].player1_color);
	strcat(msg, tmp);
      } 
    }
    sprintf(_in, "%s\n", msg);
    return;
  } 
  if(strncmp(_in, "join_game", 9) == 0) {
    if(parse_join_game(_player_id, &_in[10], _player_key) == false) {
      sprintf(_in, "NOK\n");
      return;
    }
    sprintf(_in, "ok\n");
    return; 
  }  
  if(strncmp(_in, "show_board", 9) == 0) {
    int gid = all_games.get_game(_player_id);
    if(gid == -1) {
      sprintf(_in, "NOK\n");
      return;
    }
    char local_board[BOARD_SIZE];
    all_games.get_board(gid, (char*)local_board);

    char p2_color = inv_col(all_games.infos[gid].player1_color);
    sprintf(_in, "%s=%c %s=%c turn=%c;",
	    all_games.infos[gid].player1_name,
	    all_games.infos[gid].player1_color,
	    all_games.infos[gid].player2_name,
	    p2_color,
	    all_games.infos[gid].turn);

    for(int i = 0; i < BOARD_SIZE; i++) {
      if(i > 0 && (i%BOARD_EDGE) == 0) { 
	strcat(_in, ";");
      }
      strncat(_in, &local_board[i], 1);
    }
    strcat(_in, "\n");
    return;
  }
  if(strncmp(_in, "show_score", 10) == 0) {
    score_t pscore;
    get_score(_player_key, pscore);
    sprintf(_in, "%d %d %d\n", pscore.win, pscore.lost, pscore.abort);
    return;
  }
  if(strncmp(_in, "quit", 4) == 0) {
    printf("---recv quit from %d\n", _player_id);
    all_games.quit(_player_id, _player_key);
    sprintf(_in, "ok\n");
    return; 
  }
  sprintf(_in, "NOK\n");
  return; 
}


bool games_t::player_identification(int _pid, char*_pname) {
  int all_players_size = (int)(sizeof(all_players)/sizeof(*all_players));
  char identif[128];
  sprintf(identif, "%d:%s", _pid, _pname);
  if(player_name_is_valide(_pname) == false) return false;
  for(int i = 0; i < all_players_size; i++) {
    if(strncmp(identif, all_players[i], 128) == 0) return true;
  }
  return false;
}

bool games_t::player_key_involved(int _player_key) {
  for(int i = 0; i < MAX_GAME; i++) {
    if(infos[i].player1_key == _player_key) return true;
    if(infos[i].player2_key == _player_key) return true;
  }
  return false;
}
bool games_t::player_id_involved(int _player_id) {
  for(int i = 0; i < MAX_GAME; i++) {
    if(infos[i].player1_id == _player_id) return true;
    if(infos[i].player2_id == _player_id) return true;
  }
  return false;
}
bool games_t::new_game(int& _game_id) {
  for(int i = 0; i < MAX_GAME; i++) {
    if(infos[i].player1_id == -1 && infos[i].player2_id == -1) {
      _game_id = i;
      return true;
    }
  }
  return false;
}
bool games_t::join_game(int _gid) {
  for(int i = 0; i < MAX_GAME; i++) {
    if(infos[i].player1_id == _gid && infos[i].player2_id == -1) {
      return true;
    }
  }
  return false;
}
int games_t::get_game(int _pid) {
  for(int i = 0; i < MAX_GAME; i++) {
    if(infos[i].player1_id == _pid & infos[i].player2_id != -1) {
      return i;
    }
    if(infos[i].player1_id != -1 & infos[i].player2_id == _pid) {
      return i;
    }
  }
  return -1;
}

void games_t::init_board(int _gid) {
  for(int i = 0; i < BOARD_SIZE; i++) {
    if(i < 2*BOARD_EDGE) 
      infos[_gid].board[i] = BLACK;
    else if(i >= BOARD_SIZE-2*BOARD_EDGE) 
      infos[_gid].board[i] = WHITE;
    else 
      infos[_gid].board[i] = FREE_SPACE;
  }
}

void games_t::get_board(int _gid, char* _board) {
  for(int i = 0; i < BOARD_SIZE; i++) {
    _board[i] = infos[_gid].board[i];
  }
}

void session(tcp::socket sock)
{
  try
  {
    int current_player_id = -1;
    for(int i = 0; i < MAX_GAME*2; i++) {
      bool ever_in = false;
      for(std::list<int>::iterator ii = players_id.begin();
	  ii != players_id.end(); ii++) {
	if(i == *ii) {
	  ever_in = true;
	  break;
	}
      }
      if(ever_in == false) {
	current_player_id = i;
	players_id.push_back(i);
	break;
      }
    }
    int current_player_key = -1;
    for (;;)
    {
      char data[MESSAGE_MAX_LENGTH];

      boost::system::error_code error;
      // size_t length = sock.read_some(boost::asio::buffer(data), error);
      boost::asio::streambuf streamread;
      boost::asio::read_until(sock, streamread, '\n', error);
      std::istream is(&streamread);
      char cdata[4096];
      std::string sdata;
      std::getline(is, sdata);
      sprintf(cdata, "%s", sdata.c_str());

      if (error == boost::asio::error::eof) {
	all_games.quit(current_player_id, current_player_key);
	//printf("boost::asio::error::eof\n");
        break; // Connection closed cleanly by peer.
      } else if (error) {
	all_games.quit(current_player_id, current_player_key);
	//printf("other error\n");
	throw boost::system::system_error(error); 
      }
      printf("---server rec::\n%s---from [%d]\n", cdata, current_player_id);
      parse_command(current_player_id, current_player_key, cdata);
      boost::asio::write(sock, boost::asio::buffer(cdata, (int)strlen(cdata)));
      printf("---server reply::\n%s---to [%d]\n", cdata, current_player_id);
      all_games.print_state();
      if(current_player_id == -1) return;
    }
  }
  catch (std::exception& e)
    {
      std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

void server(boost::asio::io_service& io_service, unsigned short port)
{
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));

  for (;;)
  {
    tcp::socket sock(io_service);
    a.accept(sock);
    std::thread(session, std::move(sock)).detach();
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: blocking_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    server(io_service, std::atoi(argv[1]));
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
