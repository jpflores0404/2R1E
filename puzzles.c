#include "game.h"

#include <stdio.h>
#include <string.h>

static void tell(Game *game, int player, const char *text)
{
    snprintf(game->players[player].clue, CLUE_SIZE, "%s", text);
}

static void next_turn(Game *game)
{
    game->players[game->active_player].prompt = PROMPT_NONE;
    game->active_player = 1 - game->active_player;
    ++game->turn_id;
    game->actions_left = ACTIONS_PER_TURN;
}

static void enter_map(Game *game, int number, int map)
{
    Player *player = &game->players[number];
    unsigned int reply = player->reply;
    *player = (Player){0};
    player->reply = reply;
    player->map = map;
    player->x = 1;
    player->y = 1;
    player->dy = 1;
    tell(game, number, "Solve your puzzles, earn your key, then face the E exit and press E.");
}

void start_game(Game *game)
{
    if (!game) return;
    *game = (Game){0};
    game->turn_id = 1;
    game->actions_left = ACTIONS_PER_TURN;
    for (int i = 0; i < 2; ++i) enter_map(game, i, 1);
}

static int normalize(const char *input, char output[TEXT_SIZE])
{
    size_t length = 0;
    int space = 0;
    for (size_t i = 0; input[i]; ++i) {
        unsigned char letter = (unsigned char)input[i];
        if (letter == ' ') {
            if (length > 0) space = 1;
            continue;
        }
        if (letter < 32 || letter > 126) return 0;
        if (letter >= 'A' && letter <= 'Z') letter += 'a' - 'A';
        if (length + (size_t)space + 1 >= TEXT_SIZE) return 0;
        if (space) output[length++] = ' ';
        output[length++] = (char)letter;
        space = 0;
    }
    output[length] = '\0';
    return 1;
}

static int use_exit(Game *game, int number)
{
    Player *player = &game->players[number];
    if (game->finished || game->winner) return 0;
    if (!player->key) {
        tell(game, number, "Exit locked. Earn this map's key first.");
        return 0;
    }

    if (player->map == 1 || player->map == 2) {
        enter_map(game, number, player->map + 1);
    } else if (player->map == 3) {
        game->winner = number + 1;
        game->finished = 1;
        for (int i = 0; i < 2; ++i) {
            game->players[i].prompt = PROMPT_NONE;
            tell(game, i, i == number
                 ? "You escaped first. You win! Press Enter to return to the main menu."
                 : "Your opponent escaped first. Press Enter to return to the main menu.");
        }
    } else {
        return 0;
    }
    return 1;
}

static int inspect(Game *game, int number)
{
    Player *player = &game->players[number];
    char object = facing_object(game, number);
    const char *text = map_description(player->map, number, object);
    int cost = 0;
    player->prompt = PROMPT_NONE;

    if (object == 'E') return use_exit(game, number);

    if (player->map == 1 && object == 'N') {
        if (player->words) text = "Order restored: light, means of passage, watcher.";
        else {
            player->prompt = PROMPT_WORDS;
            text = "First comes the ___. Then the means of ___. The ___ remains last. Clues: banishes shadows, a way through, a silent guardian. Type the three missing words in order.";
        }
    } else if (player->map == 1 && object == 'K') {
        if (player->cabinet) text = "Cabinet note: use Z, X, Y. Keep only the final digit of each result.";
        else if (!player->words) text = "Reconstruct the Three Blanks note first.";
        else {
            player->prompt = PROMPT_CABINET;
            text = "Cabinet padlock: type the three painting digits in the note's order.";
        }
    } else if (player->map == 1 && object == 'H') {
        if (player->hatch) text = "Hatch authorized. Your map key is ready. Use your E exit.";
        else if (!player->cabinet) text = "Hatch inactive. Open your cabinet first.";
        else {
            player->prompt = PROMPT_HATCH;
            text = "Hatch keypad: use your equations and the cabinet instruction. Type three digits.";
        }
    } else if (player->map == 2 && object == 'B') {
        if (!player->passage) {
            player->passage = 1;
            cost = 1;
        }
        text = "Statue B faces north beside the crack. Your service passage is open.";
    } else if (player->map == 2 && (object == 'D' || object == 'T')) {
        if (player->calibrated) text = "Calibration complete. Use your E exit.";
        else if (!player->passage) text = "Open your service passage before calibrating the receiver.";
        else {
            player->prompt = PROMPT_DIALS;
            text = "Signal calibration: type A B C as three digits from 0 to 4 separated by spaces. Feedback appears on your screen.";
        }
    } else if (player->map == 3 && object >= '1' && object <= '4') {
        if (object != '4') {
            snprintf(player->clue, CLUE_SIZE, "%s This valve is excluded from routine clearance. Compare your maintenance records and try again.", text);
            return 1;
        }
        if (!player->clearance) {
            player->clearance = 1;
            cost = 1;
        }
        text = "White Return valve accepted. Clearance granted. Activate authorization and the safety release.";
    } else if (player->map == 3 && object == 'A') {
        if (!player->clearance) text = "Authorization locked. Obtain containment clearance first.";
        else {
            if (!player->authorization) {
                player->authorization = 1;
                cost = 1;
            }
            text = "Authorization accepted. Activate your safety release too.";
        }
    } else if (player->map == 3 && object == 'S') {
        if (!player->clearance) text = "Safety release locked. Obtain containment clearance first.";
        else {
            if (!player->release) {
                player->release = 1;
                cost = 1;
            }
            text = "Mechanical safety released. Complete authorization and use your E exit.";
        }
    } else if (player->map == 3 && object == '!') {
        if (!player->trap_used) {
            player->trap_used = 1;
            cost = 3;
            text = "Trap triggered. You lose up to three AP. This trap cannot trigger again.";
        } else text = "The trap is spent.";
    }
    if (player->map == 3 && player->clearance && player->authorization && player->release) {
        player->key = 1;
        if (object == 'A' || object == 'S') text = "Both controls accepted. Your map key is ready. Use your E exit.";
    }
    tell(game, number, text);
    return cost;
}

static int calibrate(Game *game, int number, const char *text)
{
    Player *player = &game->players[number];
    if (strlen(text) != 5 || text[1] != ' ' || text[3] != ' ') {
        tell(game, number, "Type three single digits from 0 to 4, separated by spaces.");
        return 0;
    }
    for (int i = 0; i < 3; ++i) {
        if (text[2 * i] < '0' || text[2 * i] > '4') {
            tell(game, number, "Each dial must be from 0 to 4.");
            return 0;
        }
    }

    const char *feedback[3];
    int aligned = 1;
    for (int i = 0; i < 3; ++i) {
        player->dials[i] = text[2 * i] - '0';
        int difference = player->dials[i] - (i + 2);
        if (difference != 0) aligned = 0;
        feedback[i] = difference < 0 ? "below target" : difference > 0 ? "above target" : "aligned";
    }
    snprintf(player->clue, CLUE_SIZE, "Receiver feedback: A %s, B %s, C %s.%s",
             feedback[0], feedback[1], feedback[2],
             aligned ? " Your map key is ready. Use your E exit." : " Adjust the values and try again.");
    if (aligned) {
        player->calibrated = 1;
        player->key = 1;
        player->prompt = PROMPT_NONE;
    }
    return 1;
}

static int answer_prompt(const Game *game, int number)
{
    const Player *player = &game->players[number];
    char object = facing_object(game, number);
    if (player->map == 1) {
        return (object == 'N' && player->prompt == PROMPT_WORDS && !player->words) ||
               (object == 'K' && player->prompt == PROMPT_CABINET && player->words && !player->cabinet) ||
               (object == 'H' && player->prompt == PROMPT_HATCH && player->cabinet && !player->hatch);
    }
    return player->map == 2 && (object == 'D' || object == 'T') &&
           player->prompt == PROMPT_DIALS && player->passage && !player->calibrated;
}

static int submit_answer(Game *game, int number, const char *input)
{
    Player *player = &game->players[number];
    char text[TEXT_SIZE];
    if (!normalize(input, text)) {
        tell(game, number, "Answer is too long or contains unsupported characters.");
        return 0;
    }
    if (!text[0]) {
        tell(game, number, "Type an answer before pressing Enter.");
        return 0;
    }
    if (player->prompt == PROMPT_DIALS) return calibrate(game, number, text);

    if (player->prompt == PROMPT_WORDS) {
        if (strcmp(text, "light passage watcher") != 0) {
            tell(game, number, "Try again. Use the words for the shining start, the way through, and the watching guardian.");
            return 1;
        }
        player->words = 1;
        tell(game, number, "Order restored: light, means of passage, watcher. Inspect the cabinet next.");
    } else if (player->prompt == PROMPT_CABINET) {
        if (strcmp(text, "246") != 0) {
            tell(game, number, "Wrong cabinet code. Compare your note's order with the painting numbers.");
            return 1;
        }
        player->cabinet = 1;
        tell(game, number, "Cabinet opened. Use Z, X, Y. Keep only the final digit of each result.");
    } else if (player->prompt == PROMPT_HATCH) {
        if (strcmp(text, "259") != 0) {
            tell(game, number, "Wrong hatch code. Use the final digit of each result in the cabinet's order.");
            return 1;
        }
        player->hatch = 1;
        player->key = 1;
        tell(game, number, "Hatch authorized. Your map key is ready. Use your E exit.");
    }
    player->prompt = PROMPT_NONE;
    return 1;
}

void apply_action(Game *game, int number, const char *command)
{
    if (!game || number < 0 || number > 1 || !command || game->finished || game->winner) return;
    Player *player = &game->players[number];
    if (number != game->active_player || game->actions_left <= 0) return;

    int cost = 0;
    if (strlen(command) == 1 && strchr("wasdWASD", command[0])) {
        if (player->prompt != PROMPT_NONE) return;
        int old_x = player->x;
        int old_y = player->y;
        move_player(game, number, command[0]);
        if (player->x != old_x || player->y != old_y) {
            cost = 1;
        }
    } else if (strcmp(command, "e") == 0 || strcmp(command, "E") == 0) {
        cost = inspect(game, number);
    } else if (strcmp(command, "cancel") == 0) {
        player->prompt = PROMPT_NONE;
    } else if (strncmp(command, "answer ", 7) == 0) {
        if (!answer_prompt(game, number)) return;
        cost = submit_answer(game, number, command + 7);
    } else if (strcmp(command, "pass") == 0) {
        next_turn(game);
        return;
    } else {
        return;
    }

    if (cost > game->actions_left) cost = game->actions_left;
    game->actions_left -= cost;
    if (!game->finished && game->actions_left == 0) next_turn(game);
}
