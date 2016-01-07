This document describes how to compile and use the breakthrough game server.

Communications are derived from blocking boost example :
boost_asio/example/cpp11/echo/blocking_tcp_echo_server.cpp
boost_asio/example/cpp11/echo/blocking_tcp_echo_client.cpp

Player's identification depends on players' db defined in common.h

Makefile script should compile all programs by running :
 $>make depend; make all

It needs boost to be installed and g++11 support

As breakthrough is a 2 players game, you should open 3 terminals identified with $1> $2> and $3>

Running 1 server and 2 clients on the same computer corresponds to:
 $1>./server 1234
 $2>./client localhost 1234
 $3>./client localhost 1234
[except that client can be cmd-client, random-client-A and random-client-B]
random-client-A is a random player that creates a new game
random-client-B is a random player that joins the game 0

If you want to run a match between 2 random players, start the server, run first random-client-A and then run random-client-B :
 $1>./server 1234
 $2>./random-client-A localhost 1234
 $3>./random-client-B localhost 1234

If you run classical client like cmd-client :
for next commands, the server reply are mentionned 
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


here is an example of a complete game :
 $1>./server 1234
 $2>./client localhost 1234
 $3>./client localhost 1234

 $2>id aze 111
 =
 $2>new_game o
 =
 $3>id qsd 111
 =
 $3>join_game 0
 =
 $2>get_state
 = 2
 $3>get_state
 = 2
 $2>get_turn
 = you o
 $3>get_turn 
 = opp o
 $2>show_board
 = aze=@ rty=o turn=o
    A B C D E F G H
 8  @ @ @ @ @ @ @ @
 7  @ @ @ @ @ @ @ @
 6  . . . . . . . .
 5  . . . . . . . .
 4  . . . . . . . .
 3  . . . . . . . .
 2  o o o o o o o o
 1  o o o o o o o o
 $2>play A2:B3
 $3>play H7:G6
 $2>play H2:G3
 $3>play A7:B6
 $2>play G2:F3
 $3>play G6:H5
 $2>play B2:C3
 $3>play D7:C6
 $2>play D2:E3
 $3>play C6:B5

 $2>play H1:G2
 $3>play B7:C6
 $2>play A1:B2
 $3>play A8:B7
 $2>play E2:D3
 $3>play B6:A5
 $2>play D3:E4
 $3>play C7:D6
 $2>play E3:D4
 $3>play D8:C7

 $2>play G3:F4
 $3>play H5:H4
 $2>play D1:E2
 $3>play C6:C5
 $2>play E2:D3
 $3>play B5:B4
 $2>play E1:D2
 $3>play E8:D7 
 $2>play D3:C4
 $3>play G7:F6

 $2>play C3:B4
 $3>play C5:B4
 $2>play D2:C3
 $3>play B4:C3
 $2>play B2:C3
 $3>play C7:B6
 $2>play E4:D5
 $3>play B6:C5
 $2>play C4:B5
 $3>play F7:E6

 $2>play F3:E4
 $3>play E6:E5
 $2>play F2:E3
 $3>play H8:H7
 $2>play C2:D3
 $3>play H7:G6 
 $2>play F4:F5
 $3>play G6:F5
 $2>play E4:F5
 $3>play E5:D4

 $2>play E3:D4
 $3>play F6:E5 
 $2>play D4:E5
 $3>play D6:E5
 $2>play C1:D2
 $3>play H4:G3
 $2>play F1:E2
 $3>play G8:F7
 $2>play D3:C4
 $3>play E7:D6

 $2>play F5:F6
 $3>play E5:D4
 $2>play C3:D4
 $3>play C5:B4
 $2>play B1:B2
 $3>play A5:A4
 $2>play D4:E5
 $3>play A4:B3
 $2>play F6:G7
 $3>play F8:G7

 $2>play E5:D6
 $3>play B3:A2
 $2>play resign

that corresponds to the log :

 1.A2:B3 2.H7:G6 3.H2:G3 4.A7:B6 5.G2:F3 6.G6:H5 7.B2:C3 8.D7:C6 9.D2:E3 10.C6:B5 11.H1:G2 12.B7:C6 13.A1:B2 14.A8:B7 15.E2:D3 16.B6:A5 17.D3:E4 18.C7:D6 19.E3:D4 20.D8:C7 21.G3:F4 22.H5:H4 23.D1:E2 24.C6:C5 25.E2:D3 26.B5:B4 27.E1:D2 28.E8:D7 29.D3:C4 30.G7:F6 31.C3:B4 32.C5:B4 33.D2:C3 34.B4:C3 35.B2:C3 36.C7:B6 37.E4:D5 38.B6:C5 39.C4:B5 40.F7:E6 41.F3:E4 42.E6:E5 43.F2:E3 44.H8:H7 45.C2:D3 46.H7:G6 47.F4:F5 48.G6:F5 49.E4:F5 50.E5:D4 51.E3:D4 52.F6:E5 53.D4:E5 54.D6:E5 55.C1:D2 56.H4:G3 57.F1:E2 58.G8:F7 59.D3:C4 60.E7:D6 61.F5:F6 62.E5:D4 63.C3:D4 64.C5:B4 65.B1:B2 66.A5:A4 67.D4:E5 68.A4:B3 69.F6:G7 70.F8:G7 71.E5:D6 72.B3:A2 73.resign 

and if you replace X by : when pieces are captured , it makes
 1.A2:B3 2.H7:G6 3.H2:G3 4.A7:B6 5.G2:F3 6.G6:H5 7.B2:C3 8.D7:C6 9.D2:E3 10.C6:B5 11.H1:G2 12.B7:C6 13.A1:B2 14.A8:B7 15.E2:D3 16.B6:A5 17.D3:E4 18.C7:D6 19.E3:D4 20.D8:C7 21.G3:F4 22.H5:H4 23.D1:E2 24.C6:C5 25.E2:D3 26.B5:B4 27.E1:D2 28.E8:D7 29.D3:C4 30.G7:F6 31.C3XB4 32.C5XB4 33.D2:C3 34.B4XC3 35.B2XC3 36.C7:B6 37.E4:D5 38.B6:C5 39.C4:B5 40.F7:E6 41.F3:E4 42.E6:E5 43.F2:E3 44.H8:H7 45.C2:D3 46.H7:G6 47.F4:F5 48.G6XF5 49.E4XF5 50.E5XD4 51.E3XD4 52.F6:E5 53.D4XE5 54.D6XE5 55.C1:D2 56.H4:G3 57.F1:E2 58.G8:F7 59.D3:C4 60.E7:D6 61.F5:F6 62.E5:D4 63.C3XD4 64.C5:B4 65.B1:B2 66.A5:A4 67.D4:E5 68.A4XB3 69.F6:G7 70.F8XG7 71.E5XD6 72.B3:A2 73.resign 
(note that this last type of logfile is not yet available)


if you have questions, n@ai.univ-paris8.fr


