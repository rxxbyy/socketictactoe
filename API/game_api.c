/*  ==========================================================================
 *  game_api.c
 *  ==========================================================================
 *  It has all the functions that can be performed on the Tic Tac Toe table,
 *  also it has 
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


/* Defines all winning patterns for symbols */
#define ROW1    0x1c0           /* 0000 0001 1100 0000 */
#define ROW2    0x38            /* 0000 0000 0011 1000 */
#define ROW3    0x7             /* 0000 0000 0000 0111 */
#define COL1    0x124           /* 0000 0001 0010 0100 */
#define COL2    0x92            /* 0000 0000 1001 0010 */
#define COL3    0x49            /* 0000 0000 0100 1001 */
#define DIAG1   0x111           /* 0000 0001 0001 0001 */
#define DIAG2   0x54            /* 0000 0000 0101 0100 */

/*
 * Codifies a Tic Tac Toe table using struct bit fields. Where
 * - winner: 2-bit field, if his value is 0 mean that no player
 *   has won yet, 1 mean X wins and 2 mean O wins.
 * - turn: 1-bit field (boolean), if his value is zero, then it's X's turn,
 *   otherwise, it's O's turn.
 * - x, y: they're 9-bit fields indicating the position of his corresponding
 *   symbol, respectively.
 * Note: this allow us to codify the whole table using only 19 bits.
*/
typedef struct __game_table_t {
    unsigned short winner : 2;
    unsigned short turn : 1;
    unsigned short x : 9;
    unsigned short o : 9;
} __game_table_t;

bool _check_patterns(unsigned short sym)
{
    unsigned short winning_patterns[8] = {ROW1, ROW2, ROW3, COL1, COL2, COL3,
                                          DIAG1, DIAG2};
    int i;

    for (i = 0; i < 8; i++)
        if ( (sym & winning_patterns[i]) == winning_patterns[i])
            return true;
    return false;
}

extern bool is_full(struct __game_table_t *gtable)
{
    return (gtable->x | gtable->o) == 0x1ff;
}

extern void play(struct __game_table_t* gtable)
{
make_play_again:
    unsigned short position;

    printf("Make a play: ");
    scanf("%hu", &position);

    position--;
    if (gtable->turn == 0) {
        if (((gtable->x | (UINT16_C(1) << position)) & gtable->o)) {
            printf("slot already in use\n");
            goto make_play_again;
        }
        gtable->x |= (UINT16_C(1) << position);
    } else {
        if (((gtable->o | (UINT16_C(1) << position)) & gtable->x)) {
            printf("slot already in use\n");
            goto make_play_again;
        }
        gtable->o |= (UINT16_C(1) << position);
    }
    gtable->turn = ~(gtable->turn);
}

extern void display_table(struct __game_table_t* gtable)
{
    int i;

    unsigned short bitmask = 1u; /* bit mask = 0000 0001 0000 0000 */

    for (i = 0; i < 9; i++) {
        if ((gtable->x & bitmask))
            printf(" X ");
        else if ((gtable->o & bitmask))
            printf(" O ");
        else
            printf(" - ");
        bitmask <<= 1;

        if ((i + 1) % 3 == 0)
            putchar('\n');
    }
}

extern void set_winner(struct __game_table_t *gtable)
{
    if (_check_patterns(gtable->x))
        gtable->winner = 1;
    else if (_check_patterns(gtable->o))
        gtable->winner = 2;
}

extern void display_winner(struct __game_table_t *gtable)
{
    switch (gtable->winner) {
        case 0:
            puts("tie");
            break;
        case 1:
            puts("O wins");
            break;
        case 2:
            puts("X wins");
            break;
    }
}

extern void send_table(struct __game_table_t *gtable, int sockfd)
{
    unsigned short winner = gtable->winner;
    unsigned short turn = gtable->turn;
    unsigned short x = gtable->x;
    unsigned short o = gtable->o;

    write(sockfd, &winner, sizeof(unsigned short));
    write(sockfd, &turn, sizeof(unsigned short));
    write(sockfd, &x, sizeof(unsigned short));
    write(sockfd, &o, sizeof(unsigned short));
}

extern void read_table(struct __game_table_t *gtable, int sockfd)
{
    unsigned short winner;
    unsigned short turn;
    unsigned short x;
    unsigned short o;

    read(sockfd, &winner, sizeof(unsigned short));
    read(sockfd, &turn, sizeof(unsigned short));
    read(sockfd, &x, sizeof(unsigned short));
    read(sockfd, &o, sizeof(unsigned short));

    gtable->winner = winner;
    gtable->turn = turn;
    gtable->x = x;
    gtable->o = o;
}

/*
int main(void)
{
    __game_table_t gtable = {0, 0, 0, 0};

    while (!_is_full(&gtable) && gtable.winner == 0) {
        display_table(&gtable);
        play(&gtable);
        set_winner(&gtable);
    }
    display_table(&gtable);
    display_winner(&gtable);

    return 0;
}
*/