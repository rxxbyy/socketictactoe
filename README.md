<h1> Welcome to SockeTicTacToe 👋 </h1>

## Introduction
SockeTicTacToe is a terminal game based in a server/client model using
unix sockets for interprocess communitacion.

## Installation
First, clone the repository and change to the root directory of the project
```
$ git clone https://github.com/rxxbyy/socketictactoe
$ cd socketictactoe
```
You must change the `game_addr_path` variable in _./API/game_api.h_ headerfile to a
valid path in your filesystem, this will be the path to setup the addresses of
the server and client sockets. Then, you can compile the project using `make`
```
$ make
```
this will write two executable files `host` and `player` in the _bin_ directory of the
project. You need to use two different shell instances, one for `host` and another one for 
`player`. First you need to execute `host`:
```
(in the first shell instance)
$ ./bin/host
```
and select the _"Start game server"_ option. Then execute `player`:
```
(in a second shell instance)
$ ./bin/player
```