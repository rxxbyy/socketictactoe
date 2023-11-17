#ifndef __game_api_h
#define __game_api_h

const char *game_addr_path = "/home/mangolito/final.txt";
extern void play(struct __game_table_t* gtable);
extern void display_table(struct __game_table_t* gtable);
extern void set_winner(struct __game_table_t *gtable);
extern void display_winner(struct __game_table_t *gtable);

typedef struct __game_table_t {
    unsigned winner : 2;
    unsigned turn : 1;
    unsigned short x : 9;
    unsigned short o : 9;
} __game_table_t;

#endif