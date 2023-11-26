/* ===========================================================================
 *  SockeTicTacToe - player.c
 * ===========================================================================
 * Client side code. Creates a PF_UNIX socket and connect to the game address
 * given defined in game_api.h. Client can't start a new chat. It only 
 * responds to the server queries.
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
    int cplayer_sock;                       /* client socket file descriptor */
    struct sockaddr_un server_address;      /* address we'll connect to */
    size_t address_len;                     /* client address length  */
    
    __game_table_t gtable = {0, 0, 0, 0};   /* initialize a game table from game API */
    char user_op;                           /* store user input options */

    char msg_sign;                          /* current message sign */
    char chat_msg[MAX_BUF_SIZE];            /* buffer for receive messages */
    int conn_status = 1;                    /* connnecton status (1 = ok, 0 = bad) */


    /* create a unix protocol family socket of type SOCK_STREAM, using the 
    default protocol (0) */
    if ( (cplayer_sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("error while creating socket");
        exit(1);
    }

    /* setup the address we want to connect to */
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, game_addr_path, sizeof(server_address.sun_path));
    server_address.sun_path[sizeof(server_address.sun_path) - 1] = '\0';

    address_len = sizeof (server_address.sun_family)
                    + strlen(server_address.sun_path);

    /* connecting to the address */
    if ((connect(cplayer_sock, (struct sockaddr *) &server_address, address_len)) < 0) {
        perror("connect: client socket connection error!");
        exit(EXIT_FAILURE);
    }

    while (!is_full(&gtable) && gtable.winner == 0 && conn_status) {
        printf("Waiting for host...\n");

        /* We use the same channel of communitacion for send/receive the game 
        table, and for send/receive messages, thus, we need a way to
        detect if the incoming message is a chat type (c) or a game
        table (t). */

        conn_status = read_sign(cplayer_sock, &msg_sign);
        switch (msg_sign) {
            case 't':
                conn_status = read_table(&gtable, cplayer_sock, 0);
                
                if (is_full(&gtable) || gtable.winner != 0 || !conn_status) 
                    goto end_game;

                display_table(&gtable);

                play(&gtable);
                set_winner(&gtable);
                conn_status = send_table(&gtable, cplayer_sock);
                break;
            case 'c':
                read_message(cplayer_sock, chat_msg);
                if (strcmp(chat_msg, "EOC") != 0) {
                    /*send_chat_sign(cplayer_sock);*/
                    if (send_message(cplayer_sock)) {
                        msg_sign = 't';
                        continue;
                    }
                } else {
                    msg_sign = 't';
                    continue;
                }
                break;
        }
    }
end_game: /* we jump here if the while loop end or if a winner is detected */
    if (conn_status == 0) { /* checking if the host killed his process during the game */
        printf("Opponent has abandoned the match\n");
        (gtable.turn == 1 ? printf("X wins\n") : printf("O wins\n"));
    } else {
        display_table(&gtable);
        display_winner(&gtable);
    }

    close(cplayer_sock); /* end connection from player */
    return 0;
}