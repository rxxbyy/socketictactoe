#ifndef __game_api_h
#define __game_api_h

/* path associated to the names of the both host and client 
sockets */
const char *game_addr_path = "/home/mangolito/ttt.txt";

typedef struct __game_table_t {
    unsigned winner : 2;
    unsigned turn : 1;
    unsigned short x : 9;
    unsigned short o : 9;
} __game_table_t;

extern void play(struct __game_table_t* gtable);
extern void display_table(struct __game_table_t* gtable);
extern void set_winner(struct __game_table_t *gtable);
extern void display_winner(struct __game_table_t *gtable);
extern bool is_full(struct __game_table_t *gtable);
extern int send_table(struct __game_table_t *gtable, int sockfd);
extern int read_table(struct __game_table_t *gtable, int sockfd, bool sign);
extern void display_menu(void);

#endif