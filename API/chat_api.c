#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>


#define MAX_BUF_SIZE 200


extern void read_sign(int __sockfd, char *msg_sign)
{
    read(__sockfd, msg_sign, sizeof(char));
    printf("%c\n", *msg_sign);
}

extern void send_chat_sign(int __sockfd)
{
    const char chat_sign = 'c';
    write(__sockfd, &chat_sign, sizeof(char));
}

extern bool send_message(int __sockfd)
{
    char msg_buff[MAX_BUF_SIZE];

    printf("Send message > ");
    scanf("%200s", msg_buff);

    if (strcmp(msg_buff, "EOC") != 0)
        send_chat_sign(__sockfd);
    
    write(__sockfd, msg_buff, strlen(msg_buff));
    return (msg_buff[0] == 'E' && msg_buff[1] == 'O' && msg_buff[2] == 'C');
}

extern void read_message(int __sockfd, char *__to_buf)
{
    read(__sockfd, __to_buf, sizeof (__to_buf));
    printf("IN: %s\n", __to_buf);

    for (int i = 0; i < MAX_BUF_SIZE; i++) {
        __to_buf[i] = '\0';
    }
}
