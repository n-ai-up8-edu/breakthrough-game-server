// --------------------//
// n@ai.univ-paris8.fr //
// july 2015           //
// --------------------//
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <list>
#include "common.h"

using boost::asio::ip::tcp;

#define MAX_GAME 2
#define MAX_PLAYER 512

#define SHOW_COORD

int all_players_state[MAX_PLAYER];

struct game_info_t {
  char player1_color;
  char turn;
  int player1_id;
  char player1_name[128];
  int player2_id;
  char player2_name[128];
  char board[BOARD_SIZE];
  char last_move[128];

  void print_board() {
#ifdef SHOW_COORD
    if(BOARD_EDGE == 8) {
      printf("   0 1 2 3 4 5 6 7\nA ");
    }
    int line_number = 1;
#else
    printf(" ");
#endif
    for(int i = 0; i < BOARD_SIZE; i++) {
      if(i > 0 && (i%BOARD_EDGE) == 0) { 
#ifdef SHOW_COORD
	char tmp[16];
	printf("\n%c ", 'A'+line_number);
	line_number ++;
#else
	printf("\n ");
#endif
      }
      printf(" %c", board[i]);
    }
    printf("\n");
  }
};

boost::mutex mtx_all_games;

struct score_t {
  int win;
  int lost;
  int abort;
};

struct games_t {
  std::list<int> players;
  game_info_t* infos;
  score_t scores[MAX_PLAYER];

  games_t() {
    infos = new game_info_t[MAX_GAME];
    for(int i = 0; i < MAX_GAME; i++) {
      infos[i].player1_color = WHITE;
      infos[i].turn = FREE_SPACE;
      infos[i].player1_id = -1;
      sprintf(infos[i].player1_name, "%s", (char*)"");
      infos[i].player2_id = -1;
      sprintf(infos[i].player2_name, "%s", (char*)"");
      init_board(i);
      sprintf(infos[i].last_move, "%s", (char*)"");
    }
  }
  ~games_t() {
    delete(infos);
  }

  void add_player(int _pid) {
    mtx_all_games.lock();
    players.push_back(_pid);
    mtx_all_games.unlock();
  }
  void del_player(int _pid) {
    mtx_all_games.lock();
    players.remove(_pid);
    mtx_all_games.unlock();
  }
  
  void print_state() {
    mtx_all_games.lock();
    printf(" nb_player: %d\n", (int)players.size());
    for(std::list<int>::iterator ii = players.begin(); ii != players.end(); ii++) {
      printf(" \t%d\t%s\t%d\n", *ii, (char*)players_db[(*ii)*3+2], all_players_state[*ii]);
    }
    printf(" open games:\n");
    for(int i = 0; i < MAX_GAME; i++) { 
      if(infos[i].player1_id != -1 && infos[i].player2_id == -1) {
	char p2_color = inv_col(infos[i].player1_color);
	printf(" %d\t%d:%c\t%d:%c %c\n", 
	       i, infos[i].player1_id, infos[i].player1_color, 
	       infos[i].player2_id, p2_color,
	       infos[i].turn);
      }
    }
    printf(" current games:\n");
    for(int i = 0; i < MAX_GAME; i++) { 
      if(infos[i].player1_id != -1  && infos[i].player2_id != -1) {
	char p2_color = inv_col(infos[i].player1_color);
	printf(" \t%d:%c\t%d:%c %c\n", 
	       infos[i].player1_id, infos[i].player1_color, 
	       infos[i].player2_id, p2_color,
	       infos[i].turn);
      }
    }
    printf("\n");
    mtx_all_games.unlock();
  }

  bool player_identification(char* _name, char*_pass, int& _pid);
  bool player_is_free(int _pid);
  bool new_game(int _pid, char* _pname, char _c, int& _gid);
  bool del_game(int _gid, int _pid);
  int get_game(int _pid);

  // these functions internally check parameters integrity
  void get_str_board(int _gid);
  bool join_game(int _gid, int _pid, char*_pname);
  bool game_started(int _gid);
  void init_board(int _gid);
  void get_board(int _gid, char* _board);
  void move(int _gid, int _posI, int _posF);
  char endgame(int _gid);
  void add_win_to_score(int _gid, int _pid);
  void add_lost_to_score(int _gid, int _pid);
  void app_win(int _gid, int _pid);
  void app_lost(int _gid, int _pid);
  bool quit(int _pid, char* _pname);

  // get_score and set_score
  void read_score(char* _pname, score_t& _s);
  void write_score(char* _pname, score_t _s);
};
games_t all_games;

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

// "id PLAYER_NAME PLAYER_PASSWD"
bool parse_id(char* _in, int& _pid, char* _pname) {
  char name[128];
  char pass[128];
  char* pch = strtok (_in, " ");
  strncpy(name, pch, 128);
  pch = strtok (NULL, " ");
  strncpy(pass, pch, 128);
  int pid;
  if(all_games.player_identification((char*)name, (char*)pass, pid) == false) return false;
  if(all_games.player_is_free(pid) == true) return false;
  _pid = pid;
  strncpy(_pname, name, 128);
  all_players_state[_pid] = PS_FREE;
  all_games.players.push_back(_pid);
  all_games.read_score((char*)name, all_games.scores[_pid]);
  return true;
}

// "new_game MY_COLOR"
bool parse_new_game(int _pid, char* _pname, char* _in, int& _ngid) {
  char color;
  int r = sscanf(_in, "%c", &color);
  if( r != 1) return false;
  if(color != WHITE && color != BLACK) return false;
  if(all_games.new_game(_pid, (char*)_pname, color, _ngid) == true) {
    all_players_state[_pid] = PS_WAIT_GAME;
    return true;
  }
  return false;
}

// "join_game GAME_ID"
bool parse_join_game(int _pid, char* _pname, char* _in) {
  int gid;
  int r = sscanf(_in, "%d", &gid);
  if( r != 1) return false;
  if(all_games.player_is_free(_pid) == false) return false;
  return all_games.join_game(gid, _pid, (char*)_pname);
}
 
// "play POS_I:POS_F"
bool parse_play(int _pid, char* _in, int& _gid, int& _posI, int& _posF) {
  // (check if you play on a board)
  int ngid = all_games.get_game(_pid);
  if(ngid == -1) return false;
  if(all_games.infos[ngid].player2_id == -1) return false;
  // (check if it is your turn to play)
  char current_turn = all_games.infos[ngid].turn;
  if(_pid == all_games.infos[ngid].player1_id) {
    if(current_turn != all_games.infos[ngid].player1_color) {
      return false;
    }
  } else if(_pid == all_games.infos[ngid].player2_id) {
    if(current_turn == all_games.infos[ngid].player1_color) {
      return false;
    }
  }
  // (check if POS_I and POS_F are on board)
  char line_i;
  int col_i;
  char line_f;
  int col_f;
  int r = sscanf(_in, "%c%d:%c%d", &line_i, &col_i, &line_f, &col_f);
  if( r != 4) return false;
  int nposi = col_i + (line_i-'A')*BOARD_EDGE;
  int nposf = col_f + (line_f-'A')*BOARD_EDGE;
  //printf("move from %d to %d\n", nposi, nposf);
  if(nposi < 0 || nposi >= BOARD_SIZE) return false;
  if(nposf < 0 || nposf >= BOARD_SIZE) return false;

  // (check if POS_I is one of your pawns)
  char posi_col = all_games.infos[ngid].board[nposi];
  if(posi_col != current_turn) return false;

  // (check if POS_F is free or opponent pawn)
  char posf_col = all_games.infos[ngid].board[nposf];
  if(posf_col == current_turn) return false;
  
  // (check if move(POS_I,POS_F) is valid move)
  if(col_f < col_i-1) return false;
  if(col_f > col_i+1) return false;
  if(current_turn == WHITE) {
    if(line_f != (line_i-1)) return false;
    _gid = ngid;
    _posI = nposi;
    _posF = nposf;
    return true;
  } else if(current_turn == BLACK) {
    if((line_f-1) != line_i) return false;
    _gid = ngid;
    _posI = nposi;
    _posF = nposf;
    return true;
  } 
  return false;
}

// parse all the commands received from a client
// - help
// - id PLAYER_NAME PLAYER_PASSWD
// - get_state
// - new_game MY_COLOR 
// - del_game GAME_ID
// - list_games
// - list_players
// - join_game GAME_ID 
// - show_board
// - get_board_str
// - get_turn [GAME_ID]
// - play POS_I:POS_F
// - resign
// - get_last
// - get_score
// - quit

bool parse_command(int& _pid, char* _pname, char* _in) {
  if(strlen(_in) == 0) {
    sprintf(_in, "?\n");
    return false;
  }
  if(strncmp(_in, "help", 4) == 0) {
    strcpy(_in, "=  id PLAYER_NAME PLAYER_PASSWD;");
    strcat(_in, "  get_state;");
    strcat(_in, "  new_game MY_COLOR;");
    strcat(_in, "  del_game GAME_ID;");
    strcat(_in, "  list_games;");
    strcat(_in, "  list_players;");
    strcat(_in, "  join_game GAME_ID;");
    strcat(_in, "  show_board;");
    strcat(_in, "  get_board_str;");
    strcat(_in, "  get_turn [GAME_ID];");
    strcat(_in, "  play POS_I:POS_F;");
    strcat(_in, "  resign;");
    strcat(_in, "  get_last;");
    strcat(_in, "  get_score;");
    strcat(_in, "  quit\n");
    return false;
  }

  if(_pid == -1) {
    if(strncmp(_in, "id", 2) == 0) {
      int new_id = -1;
      if(parse_id(&_in[3], _pid, _pname) == false) {
	sprintf(_in, "?\n");
	return false;
      }
      sprintf(_in, "=\n");
      return false;
    } 
  }
  if(strncmp(_in, "get_state", 9) == 0) {
    if(_pid == -1) {
      sprintf(_in, "?\n");
      return false;
    }
    sprintf(_in, "= %d\n", all_players_state[_pid]);    
    if(all_players_state[_pid] == PS_WIN1 || 
       all_players_state[_pid] == PS_WIN2) {
      all_players_state[_pid] = PS_FREE;
    }
    return false;
  }
  if(strncmp(_in, "new_game", 8) == 0) {
    if(all_players_state[_pid] != PS_FREE) {
      sprintf(_in, "? %d\n", all_players_state[_pid]);
      return false;
    }
    int new_game_id = -1;
    if(parse_new_game(_pid, _pname, &_in[9], new_game_id) == false) {
      sprintf(_in, "?\n");
      return false;
    }
    sprintf(_in, "= %d\n", new_game_id);
    return false;
  } 
  if(strncmp(_in, "del_game", 8) == 0) {
    if(all_players_state[_pid] != PS_WAIT_GAME) {
      sprintf(_in, "? %d\n", all_players_state[_pid]);
      return false;
    }
    int ngid = all_games.get_game(_pid);
    if(ngid == -1) {
      sprintf(_in, "?\n");
      return false;
    }
    if(all_games.del_game(ngid, _pid) == false) {
      sprintf(_in, "?\n");
      return false;
    }
    all_games.init_board(ngid);
    sprintf(_in, "=\n");
    return false;
  } 
  if(strncmp(_in, "list_games", 10) == 0) {
    char msg[156];
    int msg_size = 0;
    sprintf(msg, "= games list;\tID\tPLAYER_ID\tCOLOR;");
    for(int i = 0; i < MAX_GAME; i++) {
      if(all_games.infos[i].player1_id != -1 && all_games.infos[i].player2_id == -1) {
	char tmp[32];
	sprintf(tmp, "\t%d\t%d\t\t%c;", i, all_games.infos[i].player1_id, all_games.infos[i].player1_color);
	strcat(msg, tmp);
      } 
    }
    sprintf(_in, "%s\n", msg);
    return false;
  } 
  if(strncmp(_in, "list_players", 12) == 0) {
    char msg[156];
    int msg_size = 0;
    sprintf(msg, "= players list;\tID\tNAME;");
    for(std::list<int>::iterator ii = all_games.players.begin(); ii != all_games.players.end(); ii++) {
      char tmp[32];
      sprintf(tmp, "\t%d\t%s\t%d;", *ii, (char*)players_db[(*ii)*3+2], all_players_state[*ii]);
      strcat(msg, tmp);
    }
    sprintf(_in, "%s\n", msg);
    return false;
  } 
  if(strncmp(_in, "join_game", 9) == 0) {
    if(all_players_state[_pid] != PS_FREE) {
      sprintf(_in, "? %d\n", all_players_state[_pid]);
      return false;
    }
    if(parse_join_game(_pid, _pname, &_in[10]) == false) {
      sprintf(_in, "?\n");
      return false;
    }
    sprintf(_in, "=\n");
    return false;  
  }  
  if(strncmp(_in, "show_board", 9) == 0) {
    int ngid = -1;
    int r = sscanf(&_in[11], "%d", &ngid);
    if( r != 1) {
      // if parsing the command line fails, 
      // then trying to retreive GAME_ID from PLAYER_ID 
      ngid = all_games.get_game(_pid);
      if(ngid == -1) {
	sprintf(_in, "?\n");
	return false;
      }
    } 
    if(all_games.game_started(ngid) == false) {
      sprintf(_in, "?\n");
      return false;
    }
    char local_board[BOARD_SIZE];
    all_games.get_board(ngid, (char*)local_board);
    mtx_all_games.lock();
    char p2_color = inv_col(all_games.infos[ngid].player1_color);
    sprintf(_in, "= %s=%c %s=%c turn=%c;",
	    all_games.infos[ngid].player1_name,
	    all_games.infos[ngid].player1_color,
	    all_games.infos[ngid].player2_name,
	    p2_color,
	    all_games.infos[ngid].turn);
    mtx_all_games.unlock();

#define SHOW_COORD
#ifdef SHOW_COORD
    if(BOARD_EDGE == 8) {
      strcat(_in, "   0 1 2 3 4 5 6 7;A ");
    }
    int line_number = 1;
#else
    strcat(_in, " ");
#endif
    for(int i = 0; i < BOARD_SIZE; i++) {
      if(i > 0 && (i%BOARD_EDGE) == 0) { 
#ifdef SHOW_COORD
	char tmp[16];
	sprintf(tmp, ";%c ", 'A'+line_number);
	strcat(_in, tmp);
	line_number ++;
#else
	strcat(_in, "; ");
#endif
      }
      strcat(_in, " ");
      strncat(_in, &local_board[i], 1);
    }
    strcat(_in, "\n");
    return false;
  }
  if(strncmp(_in, "get_board_str", 13) == 0) {
    int ngid = -1;
    int r = sscanf(&_in[11], "%d", &ngid);
    if( r != 1) {
      // if parsing the command line fails, 
      // then trying to retreive GAME_ID from PLAYER_ID 
      ngid = all_games.get_game(_pid);
      if(ngid == -1) {
	sprintf(_in, "?\n");
	return false;
      }
    } 
    if(all_games.game_started(ngid) == false) {
      sprintf(_in, "?\n");
      return false;
    }
    char local_board[BOARD_SIZE];
    all_games.get_board(ngid, (char*)local_board);
    mtx_all_games.lock();
    char p2_color = inv_col(all_games.infos[ngid].player1_color);
    sprintf(_in, "= %c", all_games.infos[ngid].turn);
    mtx_all_games.unlock();
    for(int i = 0; i < BOARD_SIZE; i++) {
      strncat(_in, &local_board[i], 1);
    }
    strcat(_in, "\n");
    return false;
  }

  if(strncmp(_in, "get_turn", 8) == 0) {
    if(all_players_state[_pid] != PS_IN_GAME) {
      sprintf(_in, "? %d\n", all_players_state[_pid]);
      return false;
    }
    int ngid = -1;
    ngid = all_games.get_game(_pid);
    if(ngid == -1) {
      sprintf(_in, "?\n");
      return false;
    } 
    if(all_games.game_started(ngid) == false) {
      sprintf(_in, "?\n");
      return false;
    }
    char current_turn = all_games.infos[ngid].turn;
    if(_pid == all_games.infos[ngid].player1_id) {
      if(current_turn == all_games.infos[ngid].player1_color) {
	sprintf(_in, "= you %c\n", current_turn);
	return false;
      } else {
	sprintf(_in, "= opp %c\n", current_turn);
	return false;
      }
    } else if(_pid == all_games.infos[ngid].player2_id) {
      if(current_turn == all_games.infos[ngid].player1_color) {
	sprintf(_in, "= opp %c\n", current_turn);
	return false;
      } else {
	sprintf(_in, "= you %c\n", current_turn);
	return false;
      }
    } 
    if(current_turn == all_games.infos[ngid].player1_color) {
      sprintf(_in, "= p1 %c\n", current_turn);
      return false;
    } 
    // else
    sprintf(_in, "= p2 %c\n", current_turn);
    return false;
  }
  //#define FOO
#ifdef FOO
  if(strncmp(_in, "foo", 12) == 0) {
    all_games.infos[0].board[0] = 'o';
    all_games.infos[0].print_board();
    sprintf(_in, "=\n");
    return false;
  }
#endif
  if(strncmp(_in, "play", 4) == 0) {
    mtx_all_games.lock();
    int player_state = all_players_state[_pid];
    mtx_all_games.unlock();
    if(player_state != PS_IN_GAME) {
      sprintf(_in, "? %d\n", player_state);
      return false;
    }
    int ngid;
    int pos_i;
    int pos_f;
    if(parse_play(_pid, &_in[5], ngid, pos_i, pos_f) == false) {
      sprintf(_in, "?\n");
      return false;
    } 
    sprintf(all_games.infos[ngid].last_move, "%c %s", 
	    all_games.infos[ngid].turn, &_in[5]);
    char pcol = all_games.infos[ngid].turn;
    all_games.move(ngid, pos_i, pos_f);
    char winner = all_games.endgame(ngid);
    if( winner == pcol) {
      all_games.add_win_to_score(ngid, _pid);
      all_games.app_win(ngid, _pid);
    }
    sprintf(_in, "=\n");
    return false;
  }
  if(strncmp(_in, "resign", 6) == 0) {
    if(all_players_state[_pid] != PS_IN_GAME) {
      sprintf(_in, "? %d\n", all_players_state[_pid]);
      return false;
    }
    int ngid = all_games.get_game(_pid);
    if(ngid == -1) {
      sprintf(_in, "?\n");
      return false;
    }
    all_games.add_lost_to_score(ngid, _pid);
    all_games.app_lost(ngid, _pid);
    sprintf(_in, "=\n");
    return false;
  }
  if(strncmp(_in, "get_last", 8) == 0) {
    int ngid = all_games.get_game(_pid);
    if(ngid == -1) {
      sprintf(_in, "?\n");
      return false;
    }
    if(all_games.game_started(ngid) == false) {
      sprintf(_in, "?\n");
      return false;
    }
    mtx_all_games.lock();
    if(strcmp(all_games.infos[ngid].last_move, (char*)"") == 0) {
      sprintf(_in, "?\n");
      return false;
    }
    sprintf(_in, "= %s\n", all_games.infos[ngid].last_move);
    mtx_all_games.unlock();
    return false;
  }
  if(strncmp(_in, "get_score", 10) == 0) {
    if(all_players_state[_pid] == PS_UNDEF) {
      sprintf(_in, "? %d\n", all_players_state[_pid]);
      return false;
    }
    mtx_all_games.lock();
    sprintf(_in, "= %d %d %d\n", 
	    all_games.scores[_pid].win, 
	    all_games.scores[_pid].lost, 
	    all_games.scores[_pid].abort);
    mtx_all_games.unlock();
    return false;
  }
  if(strncmp(_in, "quit", 4) == 0) {
    all_games.quit(_pid, _pname);
    sprintf(_in, "=\n");
    return true; 
  }
  sprintf(_in, "?\n");
  return false; 
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

bool games_t::player_identification(char*_pname, char*_pass, int& _pid) {
  if(player_name_is_valide(_pname) == false) return false;
  int players_db_size = (int)(sizeof(players_db)/sizeof(*players_db));
  for(int i = 0; i < players_db_size/3; i++) {
    if(strncmp(_pname, players_db[3*i+2], 128) == 0 &&
       strncmp(_pass, players_db[3*i+1], 128) == 0) {
      _pid = atoi(players_db[3*i]);
      return true;
    }
  }
  return false;
}

bool games_t::player_is_free(int _pid) {
  if(_pid < 0) return false;
  if(_pid > MAX_PLAYER) return false;
  mtx_all_games.lock();
  bool ret = false;
  if(all_players_state[_pid] == PS_FREE) ret = true;
  mtx_all_games.unlock();
  return ret;
}

bool games_t::new_game(int _pid, char* _pname, char _c, int& _gid) {
  mtx_all_games.lock();
  for(int i = 0; i < MAX_GAME; i++) {
    if(infos[i].player1_id == -1 && infos[i].player2_id == -1) {
      _gid = i;
      infos[i].player1_color = _c;
      infos[i].turn = WHITE;
      infos[i].player1_id = _pid;
      strncpy(infos[i].player1_name, _pname, 128);
      mtx_all_games.unlock();
      init_board(i);
      return true;
    }
  }
  mtx_all_games.unlock();
  return false;
}

bool games_t::del_game(int _gid, int _pid) {
  if(_gid < 0) return false;
  if(_gid >= MAX_GAME) return false;

  mtx_all_games.lock();
  if(infos[_gid].player2_id != -1) {
    mtx_all_games.unlock();
    return false;
  }
  infos[_gid].player1_color = WHITE;
  infos[_gid].turn = FREE_SPACE;
  infos[_gid].player1_id = -1;
  sprintf(infos[_gid].player1_name, "%s", (char*)"");
  infos[_gid].player2_id = -1;
  sprintf(infos[_gid].player2_name, "%s", (char*)"");
  all_players_state[_pid] = PS_FREE;
  mtx_all_games.unlock();
  return true;
}

int games_t::get_game(int _pid) {
  mtx_all_games.lock();
  for(int i = 0; i < MAX_GAME; i++) {
    if(infos[i].player1_id == _pid || infos[i].player2_id == _pid) {
      mtx_all_games.unlock();
      return i;
    }
  }
  mtx_all_games.unlock();
  return -1;
}

bool games_t::join_game(int _gid, int _pid, char*_pname) {
  if(_gid < 0) return false;
  if(_gid >= MAX_GAME) return false;

  mtx_all_games.lock();
  bool ret = false;
  if(infos[_gid].player2_id == -1 && infos[_gid].turn != END) {
    ret = true;
    infos[_gid].player2_id = _pid;
    strncpy(infos[_gid].player2_name, _pname, 128);
  }
  all_players_state[_pid] = PS_IN_GAME;
  all_players_state[infos[_gid].player1_id] = PS_IN_GAME;
  mtx_all_games.unlock();
  return ret;
}

bool games_t::game_started(int _gid) {
  if(_gid < 0) return false;
  if(_gid >= MAX_GAME) return false;

  mtx_all_games.lock();
  if(infos[_gid].player2_id == -1) {
      mtx_all_games.unlock();
    return false;
  }
  mtx_all_games.unlock();
  return true;
}

void games_t::init_board(int _gid) {
  if(_gid < 0) return;
  if(_gid >= MAX_GAME) return;

  mtx_all_games.lock();
  for(int i = 0; i < BOARD_SIZE; i++) {
    if(i < 2*BOARD_EDGE) 
      infos[_gid].board[i] = BLACK;
    else if(i >= BOARD_SIZE-2*BOARD_EDGE) 
      infos[_gid].board[i] = WHITE;
    else 
      infos[_gid].board[i] = FREE_SPACE;
  }
  mtx_all_games.unlock();
}

void games_t::get_board(int _gid, char* _board) {
  if(_gid < 0) return;
  if(_gid >= MAX_GAME) return;

  mtx_all_games.lock();
  for(int i = 0; i < BOARD_SIZE; i++) {
    _board[i] = infos[_gid].board[i];
  }
  mtx_all_games.unlock();
}

// apply move and update turn
void games_t::move(int _gid, int _posI, int _posF) {
  if(_gid < 0) return;
  if(_gid >= MAX_GAME) return;
  if(_posI < 0 || _posI >= BOARD_SIZE) return;
  if(_posF < 0 || _posF >= BOARD_SIZE) return;

  mtx_all_games.lock();
  infos[_gid].board[_posF] = infos[_gid].board[_posI];
  infos[_gid].board[_posI] = FREE_SPACE;
  infos[_gid].turn = inv_col(infos[_gid].turn);
  mtx_all_games.unlock();
}

char games_t::endgame(int _gid) {
  if(_gid < 0) return FREE_SPACE;
  if(_gid >= MAX_GAME) return FREE_SPACE;

  mtx_all_games.unlock();
  char ret = FREE_SPACE;
  for(int i = 0; i < BOARD_EDGE; i++) {
    if(infos[_gid].board[i] == WHITE) {
      ret = WHITE;
      break;
    }
  }
  for(int i = BOARD_SIZE-BOARD_EDGE; i < BOARD_SIZE; i++) {
    if(infos[_gid].board[i] == BLACK) {
      ret = BLACK;
      break;
    }
  }
  mtx_all_games.unlock();
  return ret;
}

void games_t::add_win_to_score(int _gid, int _pid) {
  if(_pid < 0) return;
  if(_pid > MAX_PLAYER) return;
  if(_gid < 0) return;
  if(_gid > MAX_GAME) return;

  mtx_all_games.lock();
  int opp_pid;
  if(infos[_gid].player1_id == _pid) {
    opp_pid = infos[_gid].player2_id;
  } else {
    opp_pid = infos[_gid].player1_id;
  }
  scores[_pid].win ++;
  scores[opp_pid].lost ++;
  mtx_all_games.unlock();
}

void games_t::add_lost_to_score(int _gid, int _pid) {
  if(_pid < 0) return;
  if(_pid > MAX_PLAYER) return;
  if(_gid < 0) return;
  if(_gid > MAX_GAME) return;

  mtx_all_games.lock();
  int opp_pid;
  if(infos[_gid].player1_id == _pid) {
    opp_pid = infos[_gid].player2_id;
  } else {
    opp_pid = infos[_gid].player1_id;
  }
  scores[_pid].lost ++;
  scores[opp_pid].win ++;
  mtx_all_games.unlock();
}

void games_t::app_win(int _gid, int _pid) {
  if(_pid < 0) return;
  if(_pid > MAX_PLAYER) return;
  if(_gid < 0) return;
  if(_gid > MAX_GAME) return;
  mtx_all_games.lock();    
  if(infos[_gid].player1_id == _pid) {
    all_players_state[_pid] = PS_WIN1;
    all_players_state[infos[_gid].player2_id] = PS_WIN1;
  } else {
    all_players_state[_pid] = PS_WIN2;
    all_players_state[infos[_gid].player1_id] = PS_WIN2;
  }
  infos[_gid].player1_color = WHITE;
  infos[_gid].turn = FREE_SPACE;
  infos[_gid].player1_id = -1;
  sprintf(infos[_gid].player1_name, "%s", (char*)"");
  infos[_gid].player2_id = -1;
  sprintf(infos[_gid].player2_name, "%s", (char*)"");
  mtx_all_games.unlock();      
}

void games_t::app_lost(int _gid, int _pid) {
  if(_pid < 0) return;
  if(_pid > MAX_PLAYER) return;
  if(_gid < 0) return;
  if(_gid > MAX_GAME) return;
  mtx_all_games.lock();    
  if(infos[_gid].player1_id == _pid) {
    all_players_state[_pid] = PS_WIN2;
    all_players_state[infos[_gid].player2_id] = PS_WIN2;
  } else {
    all_players_state[_pid] = PS_WIN1;
    all_players_state[infos[_gid].player1_id] = PS_WIN1;
  }
  infos[_gid].player1_color = WHITE;
  infos[_gid].turn = FREE_SPACE;
  infos[_gid].player1_id = -1;
  sprintf(infos[_gid].player1_name, "%s", (char*)"");
  infos[_gid].player2_id = -1;
  sprintf(infos[_gid].player2_name, "%s", (char*)"");
  mtx_all_games.unlock();      
}

bool games_t::quit(int _pid, char* _pname) {
  if(_pid < 0) return false;
  if(_pid > MAX_PLAYER) return false;

  mtx_all_games.lock();    
  if(all_players_state[_pid] == PS_UNDEF) {
    mtx_all_games.unlock();
    write_score((char*)_pname, scores[_pid]);
    return true;
  }
  if(all_players_state[_pid] == PS_FREE || 
     all_players_state[_pid] == PS_WIN1 || 
     all_players_state[_pid] == PS_WIN2) {
    all_players_state[_pid] = PS_UNDEF;
    players.remove(_pid);
    mtx_all_games.unlock();
    write_score((char*)_pname, scores[_pid]);
    return true;
  }
  if(all_players_state[_pid] == PS_IN_GAME) {
    mtx_all_games.unlock();
    int ngid = get_game(_pid);
    if(ngid == -1) {
      printf("quit ERROR pid %d without gid\n", _pid);
      return true;
    }
    mtx_all_games.unlock();
    all_games.add_lost_to_score(ngid, _pid);
    all_games.app_lost(ngid, _pid);
    mtx_all_games.lock();
  }
  if(all_players_state[_pid] == PS_WAIT_GAME) {
    int ngid = get_game(_pid);
    if(ngid == -1) {
      printf("quit ERROR pid %d without gid\n", _pid);
      return true;
    }
    mtx_all_games.unlock();
    infos[ngid].player1_color = WHITE;
    infos[ngid].turn = FREE_SPACE;
    infos[ngid].player1_id = -1;
    sprintf(infos[ngid].player1_name, "%s", (char*)"");
    mtx_all_games.lock();
  }
  all_players_state[_pid] = PS_UNDEF;
  players.remove(_pid);
  mtx_all_games.unlock();
  write_score((char*)_pname, scores[_pid]);
  return true;
}

void games_t::read_score(char* _pname, score_t& _s) {
  _s.win = 0;
  _s.lost = 0;
  _s.abort = 0;
  if(strlen(_pname) <= 0) return;
  mtx_all_games.lock();
  FILE* fp;
  if ((fp = fopen(score_filename,"r")) == 0) {
    fprintf(stderr, "fopen %s error\n", score_filename);
      mtx_all_games.unlock();
    return;
  }

  char line[1024];
  while(fgets( line, sizeof(line), fp) ) {
    score_t pscore;
    char spname[128];
    if(sscanf(line,"%d %d %d %s", 
	      &pscore.win, 
	      &pscore.lost, 
	      &pscore.abort,
	      spname) == 4) {
      if(strncmp(_pname, spname, 64) == 0) {
	_s.win = pscore.win;
	_s.lost = pscore.lost;
	_s.abort = pscore.abort;
	fclose(fp);
	mtx_all_games.unlock();
	return;
      }
    }
  }
  fclose(fp);
  mtx_all_games.unlock();
} 

void games_t::write_score(char* _pname, score_t _s) {
  if(strlen(_pname) <= 0) return;
  mtx_all_games.lock();
  FILE* fp;
  if ((fp = fopen(score_filename,"r+")) == 0) {
    fprintf(stderr, "fopen %s error\n", score_filename);
    mtx_all_games.unlock();
    return;
  }
  char line[1024];
  while(fgets( line, sizeof(line), fp) ) {
    score_t pscore;
    char spname[128];
    if(sscanf(line,"%d %d %d %s", 
	      &pscore.win, 
	      &pscore.lost, 
	      &pscore.abort,
	      spname) == 4) {
      if(strncmp(_pname, spname, 64) == 0) {
	char nline[128];
	fseek(fp, -strlen(line), SEEK_CUR);
	sprintf(nline, "%d %d %d %s\n",
		_s.win, _s.lost, _s.abort, spname);
	fwrite(nline, sizeof(char), strlen(nline), fp);
	fflush(fp);
	fclose(fp);
	mtx_all_games.unlock();
	return;
      }
    }
  }
  char nline[128];
  sprintf(nline, "%d %d %d %s\n",
	  _s.win, _s.lost, _s.abort, _pname);
  fwrite(nline, sizeof(char), strlen(nline), fp);
  fflush(fp);
  fclose(fp);
  mtx_all_games.unlock();
} 

////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

void session(tcp::socket sock) {
  try {
    int current_player_id = -1;
    char current_player_name[128];
    for (;;) {
      char data[MESSAGE_MAX_LENGTH];

      boost::system::error_code error;
      boost::asio::streambuf streamread;
      boost::asio::read_until(sock, streamread, '\n', error);
      std::istream is(&streamread);
      char cdata[4096];
      std::string sdata;
      std::getline(is, sdata);
      sprintf(cdata, "%s", sdata.c_str());

      if (error == boost::asio::error::eof) {
	all_games.quit(current_player_id, current_player_name);
        break; // Connection closed cleanly by peer.
      } else if (error) {
	all_games.quit(current_player_id, current_player_name);
	throw boost::system::system_error(error); 
      }
      printf("---server recv from %d::\n%s\n", current_player_id, cdata);
      bool end = parse_command(current_player_id, (char*)current_player_name, cdata);
      boost::asio::write(sock, boost::asio::buffer(cdata, (int)strlen(cdata)));
      printf("---server reply to %d::\n%s\n", current_player_id, cdata);
      all_games.print_state();
      if(end == true) break;
    }
  } catch (std::exception& e) { /* ... */ }
}

void server(boost::asio::io_service& io_service, unsigned short port) {
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));

  for (;;) {
    tcp::socket sock(io_service);
    a.accept(sock);
    std::thread(session, std::move(sock)).detach();
  }
}

int main(int ac, char* av[]) {

  for(int i = 0; i < MAX_PLAYER; i++) {
    all_players_state[i] = PS_UNDEF;
  }

  try {
    if (ac != 2) {
      fprintf(stderr, "Usage: %s <port>\n", av[0]);
      return 1;
    }

    boost::asio::io_service io_service;
    server(io_service, std::atoi(av[1]));

  } catch (std::exception& e) {
    fprintf(stderr, "Exception: %s\n", e.what()); 
  }

  return 0;
}
