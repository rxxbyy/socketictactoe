/* ===========================================================================
 *  SockeTicTacToe - host.c
 * ===========================================================================
 * Server side code, this program contains the logic from the server, it uses
 * the API to display the main game menu and perform all game logic over the 
 * connection.
 * Before we can actually play, we need to create a PF_UNIX socket, bind 
 * an AF_UNIX address to it, call to listen an accept a connection from the
 * opponent.
 * ===========================================================================
 * date: nov 26, 2023
 * author: rxxbyy
 * ===========================================================================
 */
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>

#include "API/game_api.h"
#include "API/chat_api.h"


int main(void)
{
    int host_sock;                                      /* host socket file descriptor */
    int conn;                                           /* client socket file descriptor */
    size_t host_size;                                   /* size of the host socket */
    socklen_t isocklen = sizeof (struct sockaddr_un);   /* size of a socket unix address */
    struct sockaddr_un host_address;
    struct sockaddr_un incoming_socketaddr;

    int user_op;                            /* store user inputs  */
    __game_table_t gtable = {0, 0, 0, 0};   /* initialize the main game table from game API */
    char chat_msg[MAX_BUF_SIZE];            /* buffer for receiving messages (200 bytes max) */
    char msg_sign = 't';                    /* current message sign */
    int conn_status = 1;                    /* connection status (1 = ok, 0 = bad) */


    display_menu();
    scanf("%d", &user_op);

    if (user_op != 1)
        exit(0);

    printf("Waiting for an opponent to connect...\n");

    /* we create a socket type SOCK_STREAM with unix protocol 
    family using the default protocol (0). */
    if ( (host_sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("error while creating socket");
        exit(1);
    }

    /* we setup the unix address indicating the family (AF_UNIX), and
        a path to a file in the current filesystem */
    host_address.sun_family = AF_UNIX;
    strncpy(host_address.sun_path, game_addr_path, sizeof( host_address.sun_path ));
    host_address.sun_path[sizeof (host_address.sun_path) - 1] = '\0';

    /* The size of the host is the size of the whole struct, this is, the offset of
    the sun path plus the length of it */
    host_size = offsetof(struct sockaddr_un, sun_path) + strlen(host_address.sun_path);

    /* We need to unlink the file `game_addr_path` to make sure we are creating a new 
        socket type file */
    unlink(game_addr_path);
    if ( bind(host_sock, (struct sockaddr*) &host_address, host_size) < 0) {
        perror("error while binding host socket");
        exit(1);
    }

    if (listen(host_sock, 5) < 0) {
        perror("listen: host socket listen error!");
        exit(EXIT_FAILURE);
    }

    /* Accept a client connection, the file descriptor of the connected 
    socket will be stored in `conn` */
    conn = accept(host_sock, (struct sockaddr *) &incoming_socketaddr, &isocklen );
    if (conn < 0) {
        perror("accept: host socket accept connection error!");
        exit(EXIT_FAILURE);
    }

    while (!is_full(&gtable) && gtable.winner == 0 && conn_status != 0) {

        /* We use the same channel of communitacion for send/receive the game 
        table, and for send/receive messages, thus, we need a way to
        detect if the incoming message is a chat type (c) or a game
        table (t). */
        switch (msg_sign) {
            case 'c':
                conn_status = read_sign(conn, &msg_sign);

                read_message(conn, chat_msg);
                if (send_message(conn))
                    msg_sign = 't';
                else
                    msg_sign = 'c';
                continue; /* avoid calling game api functions while we are chatting */
            break;
            case 't':
                display_table(&gtable);
                printf("Select an option\n");
                printf("[1] play\n");
                printf("[2] chat\n");
                printf("> ");
                scanf("%d", &user_op);

                switch (user_op) {
                    case 1:
                        play(&gtable);
                        set_winner(&gtable);
                        conn_status = send_table(&gtable, conn);

                        if (gtable.winner != 0) /* if we win with the last play */
                            goto end_game;      /* jump to break all switch/case and while loop */

                        printf("Waiting for opponent to play...\n");
                        conn_status = read_table(&gtable, conn, 1);
                    break;
                    case 2:
                        printf("\nMaximum message size: 200 chars\n");
                        printf("\nYou can put spaces in the message with '.'\n"
                        "the opponent will se an space for every '.' (ex. 'hi.mom' = 'hi mom'), \n"
                        "if you want to put an actual '.', write '\\.'\n");
                        send_message(conn);
                        msg_sign = 'c';
                    break;
                }
        }
    }
end_game: /* we jump here if the while loop end or if a winner is detected */
    if (conn_status == 0) { /* checking if the client killed his process during the game */
        printf("Opponent has abandoned the match\n");
        (gtable.turn == 1 ? printf("X wins\n") : printf("O wins\n"));
    } else {
        display_table(&gtable);
        display_winner(&gtable);
    }

    close(host_sock); /* close connection between two sockets */
    return 0;
}
