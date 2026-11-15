#include "game.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static void join_game(Socket socket)
{
    Peer peer;
    Controls controls = {0};
    View current = {0}, incoming = {0};
    unsigned int pending = 0, request = 0;
    int assembling = 0, connected = 1, dirty = 1;
    int last_width = 0, last_height = 0;
    size_t used = 0;
    peer_open(&peer, socket);
    while (!terminal_stopping()) {
        if (connected && !peer_pump(&peer)) {
            connected = 0;
            peer_close(&peer);
            if (!current.finished) {
                notice("Host disconnected. Start a new Host and Join session.");
                break;
            }
        }
        char line[TEXT_SIZE];
        int received;
        while (connected && (received = peer_line(&peer, line)) != 0) {
            if (received < 0) { connected = 0; break; }
            if (!assembling) {
                char *end;
                incoming = (View){0};
                if (strlen(line) < 12 || strncmp(line, "VIEW ", 5) ||
                    line[5] < '1' || line[5] > '3' || line[6] != ' ' ||
                    line[7] < '0' || line[7] > '4' || line[8] != ' ' ||
                    line[9] < '0' || line[9] > '1' || line[10] != ' ' ||
                    !request_number(line + 11, &end, &incoming.reply) || *end) {
                    connected = 0;
                    break;
                }
                incoming.map = line[5] - '0';
                incoming.prompt = line[7] - '0';
                incoming.finished = line[9] - '0';
                assembling = 1;
                used = 0;
            } else if (strcmp(line, ".") == 0) {
                int old_map = current.map;
                current = incoming;
                sync_controls(&controls, &pending, old_map, &current);
                assembling = 0;
                dirty = 1;
            } else {
                size_t length = strlen(line);
                if (used + length + 2 > sizeof incoming.screen) { connected = 0; break; }
                memcpy(incoming.screen + used, line, length);
                used += length;
                incoming.screen[used++] = '\n';
                incoming.screen[used] = '\0';
            }
        }
        if (!connected && !current.finished) {
            notice("The connection ended or sent an invalid screen.");
            break;
        }
        int width, height;
        if (!terminal_size(&width, &height)) break;
        if (dirty || width != last_width || height != last_height) {
            if (!show_view(&current, &controls)) break;
            last_width = width;
            last_height = height;
            dirty = 0;
        }
        int key = usable_key(terminal_key(10), &controls, current.finished);
        if (key < 0) break;
        if (current.finished) {
            if (key == KEY_ENTER || key == 'q' || key == 'Q') break;
            continue;
        }
        char command[TEXT_SIZE];
        if (key == KEY_ESCAPE) pending = 0;
        int action = control_key(&controls, key, command);
        if (action < 0) break;
        if (action > 0 && current.map) {
            char message[TEXT_SIZE + 24];
            if (request == UINT_MAX) break;
            ++request;
            snprintf(message, sizeof message, "%d %u %s\n", current.map, request, command);
            if (!peer_queue(&peer, message)) break;
            pending = command[1] == '\0' && (command[0] == 'e' || command[0] == 'E') ? request : 0;
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
        char address[TEXT_SIZE] = "";
        unsigned short port;
        if (!ask_connection(0, address, &port)) break;

        Socket host = socket_connect(address, port);
        if (host == NO_SOCKET) {
            notice(net_error());
            continue;
        }
        join_game(host);
    }
    terminal_close();
    return 0;
}