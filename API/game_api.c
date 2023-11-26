/* ===========================================================================
 *  SockeTicTacToe - API/game_api.c
 * ===========================================================================
 * The game_api contains all the functions that can be performed from the 
 * both host and client to the game table, it also contains functions such
 * read_table() and send_table(), which are used to send and receive the 
 * table over the connection.
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
 * Note: this allow us to codify the whole table using only 21 bits.
 */
typedef struct __game_table_t {
    unsigned short winner : 2;
    unsigned short turn : 1;
    unsigned short x : 9;
    unsigned short o : 9;
} __game_table_t;


/* _check_patterns: internal use function, it returns true if
    the positions on 'sym' match with a winning position, otherwise
    it returns false.
*/
static bool _check_patterns(unsigned short sym)
{
    unsigned short winning_patterns[8] = {ROW1, ROW2, ROW3, COL1, COL2, COL3,
                                          DIAG1, DIAG2};
    int i;

    for (i = 0; i < 8; i++)
        if ( (sym & winning_patterns[i]) == winning_patterns[i])
            return true;
    return false;
}

/* is_full(): return if the table is full, this is, if 
    doing a bitwise or operation between X's positions and
    O's positions it's equal to 0x1ff (0001 1111 1111).
*/
extern bool is_full(struct __game_table_t *gtable)
{
    return (gtable->x | gtable->o) == 0x1ff;
}

/* play(): add a position to a symbol in the table based on the 
    current turn, we can add a position to a symbol performing
    a biwtise or operation between the symbol and a 1-bitmask
    shifted 'position' times to the left.
*/
extern void play(struct __game_table_t* gtable)
{
make_play_again:
    unsigned short position;

    printf("Enter the slot number: ");
    scanf("%hu", &position);

    position--;
    if (gtable->turn == 0) {
        /* checking if adding a position to X's symbols causes a colision
        of positions */
        if (((gtable->x | (UINT16_C(1) << position)) & gtable->o)) {
            printf("slot already in use\n");
            goto make_play_again;
        }
        gtable->x |= (UINT16_C(1) << position);
    } else {
        /* checking if adding a position to O's symbols causes a colision
        of positions */
        if (((gtable->o | (UINT16_C(1) << position)) & gtable->x)) {
            printf("slot already in use\n");
            goto make_play_again;
        }
        gtable->o |= (UINT16_C(1) << position);
    }

    /* we change the turn by inverting the gtable.turn bit field */
    gtable->turn = ~(gtable->turn); 
}

/* display_table(): displays the table checking if the current
    'i' position corresponds to an X's symbol or a O's symbol.
*/
extern void display_table(struct __game_table_t* gtable)
{
    int i;

    unsigned short bitmask = 1u; /* bit mask = 0000 0001 0000 0000 */

    for (i = 0; i < 9; i++) {

        /* if doing an AND bitwise operation to a symbol results in a 
        positive number, then the current position correspond to that
        symbol, also we can have free spaces, in that case we display
        'position' indicating the slot number */
        if ((gtable->x & bitmask)) 
            printf(" X ");
        else if ((gtable->o & bitmask))
            printf(" O ");
        else
            printf(" %d ", i+1);
        bitmask <<= 1;

        if ((i + 1) % 3 == 0)
            putchar('\n');
    }
}

/* set_winner(): set the gtable.winner bit field
    to 1 if X wins, 2 if O wins and stays in 0 if
    no one wins.
*/
extern void set_winner(struct __game_table_t *gtable)
{
    if (_check_patterns(gtable->x))
        gtable->winner = 1;
    else if (_check_patterns(gtable->o))
        gtable->winner = 2;
}

/* display_winner(): displays the game winner based
    on the gtable.winner value.
*/
extern void display_winner(struct __game_table_t *gtable)
{
    switch (gtable->winner) {
        case 0:
            puts("Game end in Tie");
            break;
        case 1:
            puts("X wins");
            break;
        case 2:
            puts("O wins");
            break;
    }
}

/* send_table(): writes the values from the struct bit fields 
    of gtable to the connection stream.
*/
extern int send_table(struct __game_table_t *gtable, int sockfd)
{
    int n;
    char sign = 't';
    unsigned short winner = gtable->winner;
    unsigned short turn = gtable->turn;
    unsigned short x = gtable->x;
    unsigned short o = gtable->o;

    /* disconnection detected, connection status: bad */
    if ( (n = write(sockfd, &sign, sizeof(char))) == -1)
        return 0;

    write(sockfd, &winner, sizeof(unsigned short));
    write(sockfd, &turn, sizeof(unsigned short));
    write(sockfd, &x, sizeof(unsigned short));
    write(sockfd, &o, sizeof(unsigned short));

    return 1; /* connection status: ok */
}

/* read_table(): read the table bit fields from the connection stream
    and modify the current game table '*gtable'. 
    The 'sign' tells if we are going to receive a message sign before
    read the table.
*/
extern int read_table(struct __game_table_t *gtable, int sockfd, bool sign)
{
    int n;
    char table_sign;
    unsigned short winner;
    unsigned short turn;
    unsigned short x;
    unsigned short o;

    if (sign) {
        n = read(sockfd, &table_sign, sizeof(char));
        if (n == 0) /* disconnection detected, connection status: bad */
            return n;
    }
    read(sockfd, &winner, sizeof(unsigned short));
    read(sockfd, &turn, sizeof(unsigned short));
    read(sockfd, &x, sizeof(unsigned short));
    read(sockfd, &o, sizeof(unsigned short));

    gtable->winner = winner;
    gtable->turn = turn;
    gtable->x = x;
    gtable->o = o;

    return 1; /* connection status: ok */
}

/* display_menu(): displays the main game menu where you can
    start the game server, this is, create the host socket 
    and listen for a connection or just exit the game.
*/
extern void display_menu(void)
{
    char *ttt_logo = "\t\tWelcome to\n"
" _____  _         _____              _____             \n"
"|_   _||_| ___   |_   _| ___  ___   |_   _| ___  ___   \n"
"  | |  | ||  _|    | |  | .'||  _|    | |  | . || -_|  \n"
"  |_|  |_||___|    |_|  |__,||___|    |_|  |___||___|  \n"
"                                                       \n\n";
    write(STDOUT_FILENO, ttt_logo, strlen(ttt_logo));
    fflush(stdout);
    printf("=============================================\n");
    printf("\t\tMenu\n");
    printf("=============================================\n");
    printf("[1]: Start the game server\n");
    printf("[2]: Exit\n");
    printf("Select an option: ");
}
