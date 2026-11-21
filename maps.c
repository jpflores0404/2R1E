#include "game.h"

static const char layouts[3][2][MAP_HEIGHT][MAP_WIDTH + 1] = {
    {
        {
            "#############################",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#..................#........#",
            "###......#.........#........#",
            "###......#..................#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "#############################",
        },
        {
            "#############################",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#..................#",
            "#.#......#.........#........#",
            "#..................#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "#############################",
        },
    },
    {
        {
            "#############################",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#..................#........#",
            "###......#.........#........#",
            "###......#.........P........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "#############################",
        },
        {
            "#############################",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........P........#",
            "#.#......#.........#........#",
            "#..................#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "#############################",
        },
    },
    {
        {
            "#############################",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#..................#........#",
            "###......#.........#........#",
            "###......#..................#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "#############################",
        },
        {
            "#############################",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#.........#........#",
            "#.#......#..................#",
            "#.#......#.........#........#",
            "#..................#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "###......#.........#........#",
            "#############################",
        },
    },
};

typedef struct {
    int map, player, x, y;
    char symbol;
    const char *description;
} Object;

static const Object objects[] = {
    {1, 0, 3, 5, 'A', "Eye painting: the watcher bears the number 6."},
    {1, 0, 4, 5, 'B', "Lantern painting: the light bears the number 2."},
    {1, 0, 5, 5, 'C', "Key painting: the means of passage bears the number 4."},
    {1, 0, 7, 5, 'N', "Three Blanks: first comes the light, then the means of passage, and finally the watcher."},
    {1, 0, 12, 5, 'K', "The cabinet has a three-digit padlock."},
    {1, 0, 15, 5, 'D', "Equation desk: X = 14 - 9. Y = 3 x 3. Z = 8 + 4."},
    {1, 0, 22, 7, 'H', "The hatch has a three-digit keypad."},
    {1, 0, 28, 8, 'E', "The exit requires this map's key."},

    {1, 1, 3, 7, 'A', "Eye painting: the watcher bears the number 6."},
    {1, 1, 4, 7, 'B', "Lantern painting: the light bears the number 2."},
    {1, 1, 5, 7, 'C', "Key painting: the means of passage bears the number 4."},
    {1, 1, 7, 7, 'N', "Three Blanks: first comes the light, then the means of passage, and finally the watcher."},
    {1, 1, 12, 7, 'K', "The cabinet has a three-digit padlock."},
    {1, 1, 15, 7, 'D', "Equation desk: X = 14 - 9. Y = 3 x 3. Z = 8 + 4."},
    {1, 1, 22, 5, 'H', "The hatch has a three-digit keypad."},
    {1, 1, 28, 4, 'E', "The exit requires this map's key."},

    {2, 0, 3, 5, 'i', "Blueprint: use the north-facing statue nearest the uniquely cracked floor tile."},
    {2, 0, 4, 5, 'A', "Statue A faces east and stands away from the cracked tile."},
    {2, 0, 6, 5, 'B', "Statue B faces north, directly beside the cracked tile."},
    {2, 0, 6, 4, '*', "A single cracked floor tile marks the service latch."},
    {2, 0, 8, 5, 'C', "Statue C faces north, several tiles from the crack."},
    {2, 0, 22, 7, 'R', "Receiver rule: A begins at two. B is one above A. C is one above B."},
    {2, 0, 24, 7, 'D', "The dial panel has channels A, B and C, each numbered from 0 to 4."},
    {2, 0, 26, 7, 'T', "The test console checks your current dial settings."},
    {2, 0, 28, 8, 'E', "The exit requires this map's key."},

    {2, 1, 3, 7, 'i', "Blueprint: use the north-facing statue nearest the uniquely cracked floor tile."},
    {2, 1, 4, 7, 'A', "Statue A faces east and stands away from the cracked tile."},
    {2, 1, 6, 7, 'B', "Statue B faces north, directly beside the cracked tile."},
    {2, 1, 6, 8, '*', "A single cracked floor tile marks the service latch."},
    {2, 1, 8, 7, 'C', "Statue C faces north, several tiles from the crack."},
    {2, 1, 22, 5, 'R', "Receiver rule: A begins at two. B is one above A. C is one above B."},
    {2, 1, 24, 5, 'D', "The dial panel has channels A, B and C, each numbered from 0 to 4."},
    {2, 1, 26, 5, 'T', "The test console checks your current dial settings."},
    {2, 1, 28, 4, 'E', "The exit requires this map's key."},

    {3, 0, 3, 5, 'M', "Maintenance note: red and blue valves are excluded from routine clearance."},
    {3, 0, 6, 5, 'R', "Maintenance record: operate the Return line for routine clearance. Feed lines must stay closed."},
    {3, 0, 11, 5, '1', "Valve 1: Red, Supply line."},
    {3, 0, 13, 5, '2', "Valve 2: Blue, Coolant line."},
    {3, 0, 15, 5, '3', "Valve 3: Yellow, Feed line."},
    {3, 0, 17, 5, '4', "Valve 4: White, Return line."},
    {3, 0, 16, 7, '!', "Damaged floor: using this tile can cost up to three AP."},
    {3, 0, 21, 7, 'A', "The authorization terminal confirms containment clearance."},
    {3, 0, 24, 7, 'S', "The mechanical safety release requires containment clearance."},
    {3, 0, 28, 8, 'E', "The exit requires this map's key."},

    {3, 1, 3, 7, 'M', "Maintenance note: red and blue valves are excluded from routine clearance."},
    {3, 1, 6, 7, 'R', "Maintenance record: operate the Return line for routine clearance. Feed lines must stay closed."},
    {3, 1, 11, 7, '1', "Valve 1: Red, Supply line."},
    {3, 1, 13, 7, '2', "Valve 2: Blue, Coolant line."},
    {3, 1, 15, 7, '3', "Valve 3: Yellow, Feed line."},
    {3, 1, 17, 7, '4', "Valve 4: White, Return line."},
    {3, 1, 16, 5, '!', "Damaged floor: using this tile can cost up to three AP."},
    {3, 1, 21, 5, 'A', "The authorization terminal confirms containment clearance."},
    {3, 1, 24, 5, 'S', "The mechanical safety release requires containment clearance."},
    {3, 1, 28, 4, 'E', "The exit requires this map's key."},
};

char map_cell(int map, int player, int x, int y, int passage)
{
    if (map < 1 || map > 3 || player < 0 || player > 1 ||
        x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return '#';

    char tile = layouts[map - 1][player][y][x];
    if (tile == 'P') return passage ? '.' : 'P';

    size_t count = sizeof objects / sizeof objects[0];
    for (size_t i = 0; i < count; ++i) {
        const Object *object = &objects[i];
        if (object->map == map && object->player == player &&
            object->x == x && object->y == y) return object->symbol;
    }
    /* Treat padding as wall while the shared header still permits larger grids. */
    return tile == '\0' ? '#' : tile;
}

const char *map_room(int map, int player, int x, int y)
{
    static const char *names[3][3] = {
        {"Gallery", "Equation Study", "Hatch Antechamber"},
        {"Statue Hall", "Service Corridor", "Receiver Chamber"},
        {"Maintenance Records", "Valve Room", "Safety Control"}
    };
    if (map < 1 || map > 3 || player < 0 || player > 1 ||
        x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
        return "Unknown room";

    int room = x < 9 ? 0 : x < 19 ? 1 : 2;
    return names[map - 1][room];
}

const char *map_description(int map, int player, char symbol)
{
    if (symbol == 'P') return "The service passage is sealed. A statue mechanism may open it.";
    size_t count = sizeof objects / sizeof objects[0];
    for (size_t i = 0; i < count; ++i) {
        const Object *object = &objects[i];
        if (object->map == map && object->player == player && object->symbol == symbol)
            return object->description;
    }
    return "Nothing to inspect directly ahead. Face a marked object and press E.";
}
