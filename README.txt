This document describes how to compile and use the breakthrough game server.

communications are derived from blocking boost example :
boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp
boost_asio/example/cpp11/echo/blocking_tcp_echo_client.cpp

player's identification depends on players' db defined in common.h

compile.sh script should compile server and client

As breakthrough is a 2 players game, you should open 3 terminals identified with $1> $2> and $3>

Running 1 server and 2 clients on the same computer corresponds to:
 $1>./server 1234
 $2>./client localhost 1234
 $3>./client localhost 1234

For next commands, the server reply are mentionned 
(= is the beginning of a good reply and ? is the begining of a wrong reply)

identifying the first client as aze without its passwd 
 $2>id aze 
 ? 

identifying the first client as aze correctly
 $2>id aze 111
 =

creating a game 
 $2>new_game o
 =

identifying the second player as qsd and joining the game 0
 $3>id qsd 111
 =
 $3>join_game 0
 =

get_state command allows to get current player's state

after login
get_state commande returns "= -1"

after a successfull identification
get_state commande returns "= 0"

after creating the game, 
get_state command returns "= 1" 

after joining a game, 
get_state command returns "= 2" 
(for both players involved in the game)

then both players should play 
until get_state command returns "= 3" or " 4"

after one step that returns a win or a loss,
the state will automatically return to 0
if get_state returns "= 3", the next return will be "= 0" 
if get_state returns "= 4", the next return will be "= 0" 

then players will be allowed to create or join new games

play command allows to move pieces (when it is your turn to play)
get_board_str command allows to get the current turn and board 
get_turn command allows to get only the current turn

all commands are explained in PROTOCOL_DESCRIPTION.txt file

if you have questions, n@ai.univ-paris8.fr


