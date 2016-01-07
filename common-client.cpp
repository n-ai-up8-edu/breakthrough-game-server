#include "common-client.h"

void print_server_reply(char* _str) {
  for(int i = 0; i < strlen(_str); i++) {
    if(_str[i] == ';') _str[i] = '\n';
  }
  printf("%s\n", _str);
}
      
bool can_move_to_empty(char* _str, char _col, int _posi, int _posf) {
  if(_posi < 0) return false;
  if(_posf >= BOARD_SIZE) return false;
  if(_str[_posi] != _col) return false;
  if(_str[_posf] != FREE_SPACE) return false;
  int col_i = _posi%BOARD_EDGE;
  int col_f = _posf%BOARD_EDGE;
  if(col_i == 0 && col_f == (BOARD_EDGE-1)) return false;
  if(col_i == (BOARD_EDGE-1) && col_f == 0) return false;
  return true;
}

bool can_move_to_opp(char* _str, char _col, int _posi, int _posf) {
  if(_posi < 0) return false;
  if(_posf >= BOARD_SIZE) return false;
  if(_str[_posi] != _col) return false;
  int col_i = _posi%BOARD_EDGE;
  int col_f = _posf%BOARD_EDGE;
  if(col_i == 0 && col_f == (BOARD_EDGE-1)) return false;
  if(col_i == (BOARD_EDGE-1) && col_f == 0) return false;
  if(_col == BLACK && _str[_posf] == WHITE) return true;
  if(_col == WHITE && _str[_posf] == BLACK) return true;
  return false;
}

std::list<std::pair<int,int> > board_2_moves(char* _str) {
  std::list<std::pair<int,int> > moves;
  char turn = _str[2];
  int forward = 1; 
  int first = 0; int last = BOARD_SIZE-BOARD_EDGE;
  if(turn == WHITE) {
    forward = -1; 
    first = BOARD_EDGE; last = BOARD_SIZE;
  }

  for(int i = first; i < last; i++) {
    if(can_move_to_empty(&_str[3], turn, i, i+forward*BOARD_EDGE))
      moves.push_back(std::pair<int, int>(i, i+forward*BOARD_EDGE));
    if(can_move_to_empty(&_str[3], turn, i, i+forward*BOARD_EDGE+1))
      moves.push_back(std::pair<int, int>(i, i+forward*BOARD_EDGE+1));
    if(can_move_to_empty(&_str[3], turn, i, i+forward*BOARD_EDGE-1))
      moves.push_back(std::pair<int, int>(i, i+forward*BOARD_EDGE-1));
    
    if(can_move_to_opp(&_str[3], turn, i, i+forward*BOARD_EDGE+1))
      moves.push_back(std::pair<int, int>(i, i+forward*BOARD_EDGE+1));
    if(can_move_to_opp(&_str[3], turn, i, i+forward*BOARD_EDGE-1))
      moves.push_back(std::pair<int, int>(i, i+forward*BOARD_EDGE-1));
  }
  return moves;
}

