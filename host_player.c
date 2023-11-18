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
    int host_sock;
    int conn;
    size_t host_size;
    socklen_t isocklen = sizeof (struct sockaddr_un);
    struct sockaddr_un host_address;
    struct sockaddr_un incoming_socketaddr;

    FILE *game_buff;
    char read_byte;

    __game_table_t gtable = {0, 0, 0, 0};
    char buffered_table[21] = {'\0'};

    if ( (host_sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("error while creating socket");
        exit(1);
    }
    host_address.sun_family = AF_UNIX;
    strncpy(host_address.sun_path, game_addr_path, sizeof( host_address.sun_path ));
    host_address.sun_path[sizeof (host_address.sun_path) - 1] = '\0';

    host_size = offsetof(struct sockaddr_un, sun_path) + strlen(host_address.sun_path);

    unlink(game_addr_path);
    if ( bind(host_sock, (struct sockaddr*) &host_address, host_size) < 0) {
        perror("error while binding host socket");
        exit(1);
    }

    if (listen(host_sock, 5) < 0) {
        perror("listen: host socket listen error!");
        exit(EXIT_FAILURE);
    }

    conn = accept(host_sock, (struct sockaddr *) &incoming_socketaddr, &isocklen );
    if (conn < 0) {
        perror("accept: host socket accept connection error!");
        exit(EXIT_FAILURE);
    }
    game_buff = fdopen(conn, "r");

    /* Initializing game and starting game loop */
    while (!is_full(&gtable) && gtable.winner == 0) {
        display_table(&gtable);
        play(&gtable);
        set_winner(&gtable);

        send_table(&gtable, conn);

        if (gtable.winner != 0)
            break;
        
        printf("Waiting for opponent to play...\n");
        read_table(&gtable, conn);
    }
    display_table(&gtable);
    display_winner(&gtable);


    close(host_sock);
    return 0;
}