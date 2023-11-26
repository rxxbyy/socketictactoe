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
    int cplayer_sock;
    struct sockaddr_un server_address;
    size_t address_len;
    
    FILE *game_buff;
    char user_op;

    __game_table_t gtable = {0, 0, 0, 0};


    if ( (cplayer_sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("error while creating socket");
        exit(1);
    }

    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, game_addr_path, sizeof(server_address.sun_path));
    server_address.sun_path[sizeof(server_address.sun_path) - 1] = '\0';

    address_len = sizeof (server_address.sun_family)
                    + strlen(server_address.sun_path);

    if ((connect(cplayer_sock, (struct sockaddr *) &server_address, address_len)) < 0) {
        perror("connect: client socket connection error!");
        exit(EXIT_FAILURE);
    }
    game_buff = fdopen(cplayer_sock, "r");

    char msg_sign;
    char chat_msg[MAX_BUF_SIZE];
    int conn_status = 1;
    
    while (!is_full(&gtable) && gtable.winner == 0 && conn_status) {
        printf("Waiting for host...\n");
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
end_game:
    if (conn_status == 0) {
        printf("Opponent has abandoned the match\n");
        (gtable.turn == 1 ? printf("X wins\n") : printf("O wins\n"));
    } else {
        display_table(&gtable);
        display_winner(&gtable);
    }

    close(cplayer_sock); /* end connection from player */
    return 0;
}