#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "API/game_api.h"

int main(void)
{
    int cplayer_sock;
    struct sockaddr_un server_address;
    size_t address_len;
    
    FILE *game_buff;
    char read_byte;


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
    
    char* message = "Hello, world from Client\n";
    send(cplayer_sock, message, strlen(message), 0);

    while ( (read_byte = fgetc(game_buff)) != EOF ) {
        putchar(read_byte);
    }

    close(cplayer_sock);
    return 0;
}