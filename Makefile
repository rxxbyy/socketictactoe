# ============================================================================
# SockeTicTacToe - MakeFile
# ============================================================================
# To compile all targets, just execute make command without any arguments
#	$ make
# You can compile specific targets using: $ make <target>
# for example, to generate the host binary file use
#	$ make host
# ============================================================================
# date: nov 26, 2023
# author: rxxbyy
# ============================================================================
CC=gcc
CFLAGS=-Wall
DEPS = API/chat_api.o API/game_api.o # API: game interface and chat interface

# target: all, these will be the targets when you do a make command with no
#  arguments
all: host player 

./API/chat_api.o: ./API/chat_api.c
	$(CC) -c ./API/chat_api.c -o ./API/chat_api.o

./API/game_api.o: ./API/game_api.c
	$(CC) -c ./API/game_api.c -o ./API/game_api.o

host: host.c $(DEPS) # target: host depends on host.c and the API
	$(CC) host.c $(DEPS) -o ./bin/host

player: player.c $(DEPS) # target: player depends on player.c and the API
	$(CC) player.c $(DEPS) -o ./bin/player

clean:
	rm ./bin/host ./bin/player API/*.o
