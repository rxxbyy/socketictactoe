/* ===========================================================================
 *  SockeTicTacToe - API/chat_api.c
 * ===========================================================================
 * The chat api contains all the functions that let the both host and client
 * send and receive messages in form of string of chars.
 * We use the same stream as a bidirectional multipurpose channel, this is
 * we send messages and the table over the same channel. Thus, we need to send
 * a sign before the actual message indicating what we are gonna read.
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
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>


#define MAX_BUF_SIZE 200 /* Maxmimum size of the message buffer */


/* read_sign(): read one the message sign from the connection
    stream and put it in the out identifier `msg_sign`.
*/
extern int read_sign(int __sockfd, char *msg_sign)
{
    int n;

    n = read(__sockfd, msg_sign, sizeof(char));
    return n;
}

/* send_chat_sign(): send the char sign 'c' (1 byte)
    to the peer indicating that a chat message is expected
*/
extern void send_chat_sign(int __sockfd)
{
    const char chat_sign = 'c';
    write(__sockfd, &chat_sign, sizeof(char));
}

/* send_message(): ask for the user to type a message and send
    it over the connection stream.
    returns 1 if we send the string "EOC" indicating chat
    mode is about to close.
*/
extern bool send_message(int __sockfd)
{
    char msg_buff[MAX_BUF_SIZE];

    printf("Send message > ");
    scanf("%200s", msg_buff);

    if (strcmp(msg_buff, "EOC") != 0)
        send_chat_sign(__sockfd);
    
    write(__sockfd, msg_buff, MAX_BUF_SIZE);
    return (msg_buff[0] == 'E' && msg_buff[1] == 'O' && msg_buff[2] == 'C');
}

/* read_message(): read a whole string from a connection stream
    an write it into `*__to_buf`. There were problems reading
    a whole line from the socket stream (using readline function 
    implemented below), so we use '.' char to indicate a space,
    and we can scape them (\.) to see an actual colon.
*/
extern void read_message(int __sockfd, char *__to_buf)
{
    int i;
    char c;

    read(__sockfd, __to_buf, MAX_BUF_SIZE);
    printf("Opponent sent you a message: ");
    for (; *(__to_buf) != '\0'; __to_buf++) {
        c = *__to_buf;

        if (c == '\\' && *(__to_buf + 1) == '.') {
            putchar('.');
            __to_buf++;
        } else
            putchar(*__to_buf == '.' ? ' ' : *__to_buf);
    }
    putchar('\n');

    for (i = 0; i < MAX_BUF_SIZE; i++)
        __to_buf[i] = '\0';
}

/* readline(): read a line from a stream associated to `fd` and
    put it into `buffer`.
    NOTE: this function is not used in the project (it is just to show
    my attempt to read a line from the stream)
*/
ssize_t readline(int fd, void *buffer, size_t n)
{
    ssize_t num_read;       /* number of bytes fetched by last read() */
    size_t tot_read;        /* total bytes read so far */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;           /* no pointer arithmetic on "void *" */
    tot_read = 0;
    for (;;) {
        num_read = read(fd, &ch, 1);
        putchar(ch);

        if (num_read == -1) {
            if (errno = EINTR)
                continue;
            else
                return -1;
        } else if (num_read == 0) {  /* EOF */
            if (tot_read == 0)
                return 0;
            else
                break;
        } else {
            if (tot_read < (n - 1)) {  /* num_read must be 1 if we get here */
                tot_read++;
                *buf++ = ch;
            }

            if (ch == '\n')
                break;
        }
    }

    *buf = '\0';
    return tot_read;
}