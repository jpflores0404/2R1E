#include "game.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

int request_number(const char *text, char **end, unsigned int *number)
{
    if (*text < '0' || *text > '9') return 0;
    errno = 0;
    unsigned long value = strtoul(text, end, 10);
    if (errno || value > UINT_MAX) return 0;
    *number = (unsigned int)value;
    return 1;
}

void sync_controls(Controls *controls, unsigned int *pending, int old_map, unsigned int old_turn, const View *view)
{
    if (view->finished) {
        *controls = (Controls){0};
        *pending = 0;
    } else if (controls->mode != MODE_QUIT &&
               (old_map != view->map || !view->can_act || old_turn != view->turn_id)) {
        *controls = (Controls){0};
        *pending = 0;
    }

    if (*pending && view->reply >= *pending) {
        if (view->prompt != PROMPT_NONE) {
            controls->mode = MODE_ANSWER;
            controls->prompt = view->prompt;
            controls->length = 0;
            controls->text[0] = '\0';
        }
        *pending = 0;
    }
}

int usable_key(int key, const Controls *controls, int finished)
{
    int width, height;
    if (!terminal_size(&width, &height)) return -1;
    if (finished && key == KEY_ENTER) return key;
    if (width >= 80 && height >= 30) return key;
    if (key == KEY_ESCAPE) return key;
    if (controls->mode == MODE_PLAY && (key == 'n' || key == 'N')) return key;
    if (controls->mode == MODE_QUIT &&
        (key == 'y' || key == 'Y' || key == 'n' || key == 'N')) return key;
    return 0;
}

void flush_and_close(Peer *peer)
{
    for (int turn = 0; turn < 50 && (peer->queued || peer->latest_size); ++turn) {
        if (!peer_pump(peer)) break;
        terminal_key(10);
    }
    peer_close(peer);
}

void notice(const char *message)
{
    char screen[SCREEN_SIZE];
    snprintf(screen, sizeof screen, "%s\n\nEnter or Escape: connection prompt\n", message);
    terminal_draw(screen);
    while (!terminal_stopping()) {
        int key = terminal_key(20);
        if (key < 0 || key == KEY_ENTER || key == KEY_ESCAPE) return;
    }
}

int ask_connection(int hosting, char address[TEXT_SIZE], unsigned short *port)
{
    for (int field = hosting ? 1 : 0; field < 2; ++field) {
        char input[TEXT_SIZE] = "";
        size_t length = 0;
        const char *error = "";
        while (!terminal_stopping()) {
            char screen[1200];
            const char *title = field ? "TCP port" : "Host IPv4 address";
            const char *hint = field
                ? "Enter a port from 1025 to 65535 (for example 5050)."
                : "Enter the IPv4 address shared by Host.";
            snprintf(screen, sizeof screen,
                "2R1E | %s\n\n%s\n%s\n\n%s%s\n\n%s_\n\n%s\nEnter continues. Escape cancels.\n",
                hosting ? "Host" : "Join", title, hint,
                field && !hosting ? "Host IPv4: " : "",
                field && !hosting ? address : "", input, error);
            if (!terminal_draw(screen)) return 0;
            int key = terminal_key(20);
            if (key < 0 || key == KEY_ESCAPE) return 0;
            if (key == KEY_ENTER) {
                if (field ? parse_port(input, port) : valid_address(input)) {
                    if (!field) snprintf(address, TEXT_SIZE, "%s", input);
                    break;
                }
                error = field ? "Invalid port. Enter digits from 1025 to 65535."
                              : "Invalid IPv4 address. Enter four octets from 0 to 255 for a unicast address.";
            } else if (key == KEY_BACKSPACE && length) {
                input[--length] = '\0';
            } else if (key >= 32 && key <= 126 && length < sizeof input - 1) {
                input[length++] = (char)key;
                input[length] = '\0';
            }
        }
        if (terminal_stopping()) return 0;
    }
    return 1;
}