#ifndef __chat_api_h
#define __chat_api_h

#define MAX_BUF_SIZE 200

extern void read_sign(int __sockfd, char *msg_sign);
extern void send_chat_sign(int __sockfd);
extern bool send_message(int __sockfd);
extern void read_message(int __sockfd, char *__to_buf);

#endif