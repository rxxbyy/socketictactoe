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

int main(void)
{
    int cplayer_sock;
    struct sockaddr_un server_address;
    size_t address_len;
    
    FILE *game_buff;
    char read_byte;

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
    
    while (!is_full(&gtable) && gtable.winner == 0) {
        printf("Waiting for host to play...\n");
        read_table(&gtable, cplayer_sock);

        if (is_full(&gtable) || gtable.winner != 0)
            break;

        display_table(&gtable);
        play(&gtable);
        set_winner(&gtable);

        send_table(&gtable, cplayer_sock);
    }
    display_table(&gtable);
    display_winner(&gtable);

    close(cplayer_sock);
    return 0;
}