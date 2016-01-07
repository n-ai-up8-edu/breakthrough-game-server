#ifndef COMMON_CLIENT_H
#define COMMON_CLIENT_H

#include <list>
#include <random>
#include <iterator>
#include <cstring>
#include "common.h"

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

void print_server_reply(char* _str);
      
bool can_move_to_empty(char* _str, char _col, int _posi, int _posf);

bool can_move_to_opp(char* _str, char _col, int _posi, int _posf);

std::list<std::pair<int,int> > board_2_moves(char* _str);


#endif
