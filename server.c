#include "game.h"

#include <stdio.h>
#include <string.h>

static Socket wait_for_guest(unsigned short port)
{
    Socket listener = socket_listen(port);
    if (listener == NO_SOCKET) { notice(net_error()); return NO_SOCKET; }
    char screen[512];
    snprintf(screen, sizeof screen,
         "2R1E\n\nWaiting for Player 2\n\nTCP port: %u\n\n"
         "Find this computer's LAN IPv4 address in its network settings.\n"
         "Tell Player 2 to enter that address and this port in Join.\n"
         "Q or Escape: cancel\n",
         (unsigned int)port);
    if (!terminal_draw(screen)) { socket_close(listener); return NO_SOCKET; }
    Socket guest = NO_SOCKET;
    while (!terminal_stopping()) {
        guest = socket_accept(listener);
        if (guest != NO_SOCKET) break;
        int key = terminal_key(20);
        if (key < 0 || key == 'q' || key == 'Q' || key == KEY_ESCAPE) break;
    }
    socket_close(listener);
    return guest;
}

static int valid_command(const char *command)
{
    if (strlen(command) == 1 && strchr("wasdWASDeE", command[0]))
        return 1;

    if (strcmp(command, "pass") == 0 || strcmp(command, "cancel") == 0)
        return 1;

    if (strncmp(command, "answer ", 7) == 0 &&
        strlen(command + 7) <= 150)
        return 1;

    return 0;
}

static void host_game(Socket socket)
{
    Game game;
    Peer peer;
    Controls controls = {0};
    View current = {0}, previous = {0}, guest = {0}, previous_guest = {0};
    unsigned int pending = 0;
    int connected = 1, dirty = 1, last_width = 0, last_height = 0;
    start_game(&game);
    peer_open(&peer, socket);
    while (!terminal_stopping()) {
        int old_map = current.map;
        unsigned int old_turn = current.turn_id;
        make_view(&game, 0, &current);
        sync_controls(&controls, &pending, old_map, old_turn, &current);
        make_view(&game, 1, &guest);
        if (connected && memcmp(&guest, &previous_guest, sizeof guest) != 0) {
            if (!peer_screen(&peer, &guest)) break;
            previous_guest = guest;
        }
        int width, height;
        if (!terminal_size(&width, &height)) break;
        if (dirty || memcmp(&current, &previous, sizeof current) != 0 ||
            width != last_width || height != last_height) {
            if (!show_view(&current, &controls)) break;
            previous = current;
            last_width = width;
            last_height = height;
            dirty = 0;
        }
        if (connected && !peer_pump(&peer)) {
            connected = 0;
            peer_close(&peer);
            if (!game.finished) {
                notice("Player 2 disconnected. Start a new Host and Join session.");
                break;
            }
        }
        char line[TEXT_SIZE];
        int received;
        while (connected && (received = peer_line(&peer, line)) != 0) {
            if (received < 0 || strcmp(line, "quit") == 0) {
                connected = 0;
                peer_close(&peer);
                break;
            }
            if (!game.finished && line[0] == '0' + game.players[1].map && line[1] == ' ') {
                char *end;
                unsigned int turn;
                unsigned int request;
                if (request_number(line + 2, &end, &turn) && *end == ' ' &&
                        request_number(end + 1, &end, &request) && *end == ' ' &&
                        request > game.players[1].reply &&
                        turn == game.turn_id &&
                        valid_command(end + 1)) {
                    apply_action(&game, 1, end + 1);
                    game.players[1].reply = request;
                }
            }
        }
        if (!connected && !game.finished) {
            notice("Player 2 left the session.");
            break;
        }
        old_map = current.map;
        old_turn = current.turn_id;
        make_view(&game, 0, &current);
        sync_controls(&controls, &pending, old_map, old_turn, &current);
        int key = usable_key(terminal_key(10), &controls, game.finished);
        if (key < 0) break;
        if (game.finished) {
            if (current.finished && (key == KEY_ENTER || key == 'n' || key == 'N')) break;
            continue;
        }
        char command[TEXT_SIZE];
        if (key == KEY_ESCAPE) pending = 0;
        int action = control_key(&controls, key, command);
        if (action < 0) break;
        if (action > 0 && current.can_act == 1) {
            pending = command[1] == '\0' && (command[0] == 'e' || command[0] == 'E');
            game.players[0].reply = 1;
            apply_action(&game, 0, command);
        }
        if (key) dirty = 1;
    }
    flush_and_close(&peer);
}

int main(void)
{
    if (!terminal_open()) {
        terminal_close();
        fputs("Open 2R1E in an interactive terminal.\n", stderr);
        return 1;
    }

    while (!terminal_stopping()) {
        char unused_address[TEXT_SIZE] = "";
        unsigned short port;
        if (!ask_connection(1, unused_address, &port)) break;
        Socket guest = wait_for_guest(port);
        if (guest != NO_SOCKET) host_game(guest);
    }
    terminal_close();
    return 0;
}