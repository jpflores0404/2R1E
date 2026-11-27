#ifndef GAME_H
#define GAME_H

#include <stddef.h>
#include <stdint.h>

#define MAP_WIDTH 29
#define MAP_HEIGHT 13
#define TEXT_SIZE 192
#define CLUE_SIZE 256
#define SCREEN_SIZE 4096
#define ACTIONS_PER_TURN 10
#define TURN_START_MS 10000
#define TURN_ACTION_MS 30000

enum { PROMPT_NONE, PROMPT_WORDS, PROMPT_CABINET, PROMPT_HATCH, PROMPT_DIALS };
enum { MODE_PLAY, MODE_ANSWER, MODE_QUIT };
enum { KEY_ESCAPE = 256, KEY_ENTER, KEY_BACKSPACE };

typedef struct {
    int x, y, dx, dy, prompt;
    unsigned int reply;
    char clue[CLUE_SIZE];
    int map;
    int key;
    int words, cabinet, hatch; // map 1
    int passage, calibrated; // map 2
    int dials[3];
    int clearance, authorization, release, trap_used; // map 3
} Player;

typedef struct {
    int winner;
    int finished;
    int active_player;
    unsigned int turn_id;
    int actions_left;
    int64_t deadline_ms;
    int timer_started;
    Player players[2];
} Game;

typedef struct {
    int map, prompt, finished;
    unsigned int reply;
    char screen[SCREEN_SIZE];
    unsigned int turn_id;
    int can_act;
} View;

typedef struct {
    int mode, prompt;
    char text[TEXT_SIZE];
    size_t length;
} Controls;

typedef int Socket;
#define NO_SOCKET ((Socket)-1)
typedef struct {
    Socket socket;
    char incoming[8192], outgoing[8192], latest[SCREEN_SIZE + 64];
    size_t received, queued, sent, latest_size;
} Peer;

int request_number(const char *text, char **end, unsigned int *number);
void sync_controls(Controls *controls, unsigned int *pending, int old_map, unsigned int old_turn, const View *view);
int usable_key(int key, const Controls *controls, int finished);
void flush_and_close(Peer *peer);
void notice(const char *message);
int ask_connection(int hosting, char address[TEXT_SIZE], unsigned short *port);

char map_cell(int map, int player, int x, int y, int passage);
const char *map_room(int map, int player, int x, int y);
const char *map_description(int map, int player, char object);

void move_player(Game *game, int player, char key);
char facing_object(const Game *game, int player);
int control_key(Controls *controls, int key, char command[TEXT_SIZE]);

void start_game(Game *game, int64_t now_ms);
void expire_turn(Game *game, int64_t now_ms);
void apply_action(Game *game, int player, const char *command, int64_t now_ms);

int terminal_open(void);
void terminal_close(void);
int terminal_key(int timeout_ms);
int terminal_size(int *width, int *height);
int terminal_draw(const char *text);
int terminal_stopping(void);

int valid_address(const char *address);
Socket socket_listen(unsigned short port);
int parse_port(const char *text, unsigned short *port);
const char *net_error(void);
Socket socket_accept(Socket listener);
Socket socket_connect(const char *address, unsigned short port);
void socket_close(Socket socket);
void peer_open(Peer *peer, Socket socket);
void peer_close(Peer *peer);
int peer_pump(Peer *peer);
int peer_line(Peer *peer, char line[TEXT_SIZE]);
int peer_queue(Peer *peer, const char *text);
int peer_screen(Peer *peer, const View *view);

void make_view(const Game *game, int player, View *view, int64_t now_ms);
int show_view(const View *view, const Controls *controls);

#endif
