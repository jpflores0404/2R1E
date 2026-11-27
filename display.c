#include "game.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void append(char *text, size_t capacity, const char *format, ...)
{
    size_t used = strlen(text);
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(text + used, capacity - used, format, arguments);
    va_end(arguments);
}

static void paragraph(char *screen, const char *label, const char *text)
{
    append(screen, SCREEN_SIZE, "--- %s ---\n", label);
    size_t length = strlen(text);
    for (size_t start = 0; start < length;) {
        size_t count = length - start;
        if (count > 74) {
            count = 74;
            if (text[start + count] != ' ') {
                size_t split = count;
                while (split > 0 && text[start + split] != ' ') --split;
                if (split > 0) count = split;
            }
        }
        /* A wrapped full stop must not become the network's end-of-screen line. */
        append(screen, SCREEN_SIZE, "  %.*s\n", (int)count, text + start);
        start += count;
        while (text[start] == ' ') ++start;
    }
}

void make_view(const Game *game, int player, View *view, int64_t now_ms)
{
    const Player *person = &game->players[player];
    const char *direction = "South";
    if (person->dx < 0) direction = "West";
    if (person->dx > 0) direction = "East";
    if (person->dy < 0) direction = "North";
    *view = (View){0};
    view->map = person->map;
    view->turn_id = game->turn_id;
    view->can_act = !game->finished && !game->winner &&
                    player == game->active_player && game->actions_left > 0;
    view->prompt = person->prompt;
    view->finished = game->finished;
    view->reply = person->reply;

    append(view->screen, SCREEN_SIZE, "2R1E | Map %d | Player %d | %s\n",
           person->map, player + 1,
           map_room(person->map, player, person->x, person->y));
    if (!game->finished) {
        int64_t remaining_ms = game->deadline_ms - now_ms;
        int seconds = remaining_ms > 0 ? (int)((remaining_ms + 999) / 1000) : 0;
        append(view->screen, SCREEN_SIZE,
               "Turn: P%d | %s %ds | AP: %d | P1 Map %d | P2 Map %d | %s\n",
               game->active_player + 1, game->timer_started ? "Time" : "Start",
               seconds, game->actions_left,
               game->players[0].map, game->players[1].map,
               view->can_act ? "Your turn" : "Wait for opponent");
    }
    if (person->map == 2)
        append(view->screen, SCREEN_SIZE,
               "Facing %s | Dials A=%d B=%d C=%d | @ you # wall E exit",
               direction, person->dials[0], person->dials[1], person->dials[2]);
    else
        append(view->screen, SCREEN_SIZE,
               "Facing %s | @ you # wall E exit", direction);
    append(view->screen, SCREEN_SIZE, " | Key: %s\n", person->key ? "earned" : "needed");
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        char row[MAP_WIDTH + 1];
        for (int x = 0; x < MAP_WIDTH; ++x) {
            row[x] = map_cell(person->map, player, x, y, person->passage);
            if (x == person->x && y == person->y) row[x] = '@';
        }
        row[MAP_WIDTH] = '\0';
        append(view->screen, SCREEN_SIZE, "%s\n", row);
    }
    append(view->screen, SCREEN_SIZE, "\n");
    if (view->finished)
        paragraph(view->screen, "Result",
                  game->winner == 0 ? person->clue :
                  game->winner == player + 1
                  ? "You win. You escaped first." : "Defeat. Your opponent escaped first.");
    else
        paragraph(view->screen, "Clue / feedback", person->clue);
    append(view->screen, SCREEN_SIZE, "--- Controls ---\n");
    append(view->screen, SCREEN_SIZE, "%s\n", game->finished
           ? "Enter: return to main menu"
           : "WASD: Move | E: Use / inspect | T: Pass | N: Quit");
}

int show_view(const View *view, const Controls *controls)
{
    int width, height;
    if (!terminal_size(&width, &height)) return 0;
    char screen[SCREEN_SIZE + 128];
    snprintf(screen, sizeof screen, "%s", view->screen);
    if (!view->finished && controls->mode == MODE_ANSWER) {
        const char *text = controls->text;
        if (controls->length > 50) text += controls->length - 50;
        append(screen, sizeof screen, "Answer> %s_  [Enter / Esc]\n", text);
    } else if (!view->finished && controls->mode == MODE_QUIT) {
        append(screen, sizeof screen, "Quit match? [Y: Quit | N / Esc: Cancel]\n");
    }
    int rows = 1;
    for (size_t i = 0; screen[i]; ++i)
        if (screen[i] == '\n') ++rows;
    if (rows < 30) rows = 30;
    if (width < 80 || height < rows) {
        char message[192];
        snprintf(message, sizeof message,
                 "Enlarge the terminal to at least 80 columns and %d rows.\n"
                 "%s\n", rows,
                 controls->mode == MODE_QUIT
                     ? "Quit match? [Y: Quit | N / Esc: Cancel]"
                     : "Network updates continue. N leaves gameplay.");
        return terminal_draw(message);
    }
    if (!view->map) return terminal_draw(
        controls->mode == MODE_QUIT
            ? "Quit match? [Y: Quit | N / Esc: Cancel]\n"
            : "Connecting to your room...\nN: cancel\n");
    return terminal_draw(screen);
}
