boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp
boost_asio/example/cpp11/echo/blocking_tcp_echo_client.cpp

+ echo while "quit" from client side

+ one player = player_key and player_name
  player_name = [0:9][a:z][A:Z][-_@.~]
  1 char MIN
  64 char MAX

+ new_game command
  $>new_game MY_COLOR PLAYER_KEY:PLAYER_NAME 
  (check if MY_COLOR is WHITE or BLACK)
  -->give NOK (ie if not WHITE or not BLACK)
  (check if PLAYER_KEY:PLAYER_NAME is ok)
  -->give NOK (ie refused)
  (check if PLAYER_KEY is ever in a game)
  -->give NOK (ie ever involved in a game)
  (check if PLAYER_ID is ever in a game)
  -->give NOK (ie one key per connection)
  (check if nb_game < MAX_GAME)
  -->give NOK (ie no game is free)
  else
  -->give the GAME_ID

+ list_game command
  $>list_game
  -->give the list of current games
  example :
  game list
       ID      PLAYER_ID       COLOR
       2       3	       @
  (means 1 game is open, game number 2 with player 3 as BLACK
   so you will be WHITE and you will start. WHITE always start.) 

+ join_game command
  $>join_game GAME_ID PLAYER_KEY:PLAYER_NAME
  (check if PLAYER_KEY:PLAYER_NAME is ok)
  -->give NOK (ie refused)
  (check if PLAYER_KEY is ever in a game)
  -->give NOK (ie ever involved in a game)
  (check if PLAYER_ID is ever in a game)
  -->give NOK (ie one key per connection)
  (check if game GAME_ID is open)
  -->give NOK (ie game GAME_ID is not open)
  else
  -->give ok

+ show_board command
  $>show_board GAME_ID
  (check if game GAME_ID is on going)
  -->give NOK (ie game GAME_ID is not on going)
  else
  -->give who is white, who is black, what is turn
  -->give the sequence that describes the board

+ show_score command
  $>show_score
  (check if player is identified)
  --> give "0 0 0" (ie if not identified)
  else
  -->give NB_WIN NB_LOST NB_ABORT




 
