// --------------------//
// n@ai.univ-paris8.fr //
// july 2015           //
// --------------------//
#ifndef COMMON_H
#define COMMON_H

#define MESSAGE_MAX_LENGTH 4096
#define BOARD_EDGE 8
#define BOARD_SIZE BOARD_EDGE*BOARD_EDGE

#define FREE_SPACE '.'
#define BLACK '@'
#define WHITE 'o'
#define END   '_'

char inv_col(char _col) {
  if(_col == WHITE) return BLACK;
  if(_col == BLACK) return WHITE;
  return _col;
}

// PS as PLAYER STATE
#define PS_UNDEF       -1
#define PS_FREE         0
#define PS_WAIT_GAME    1
#define PS_IN_GAME      2
#define PS_WIN1         3
#define PS_WIN2         4

// LISTING players' ID PASS NAME
// ID should be unique for each player
// ID is >= 0 and < 512
// player's name and passwd restrictions are :
//          [0:9][a:z][A:Z][-_@.~]
//          1 char MIN
//          64 char MAX

char* players_db[] = {
  (char*)"0",(char*)"111",(char*)"aze",
  (char*)"1",(char*)"111",(char*)"rty",
  (char*)"2",(char*)"111",(char*)"qsd"
};

char* score_filename = (char*)"scores.txt";
#endif
