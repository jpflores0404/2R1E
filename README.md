# 2R1E: Two Rooms, One Exit

A two-player terminal escape game in C17. Both players race to explore three maps and the first to escape wins.

## Run on Linux

Use a terminal with at least 80 columns and 30 rows on each computer.

To build from source, install GCC and the standard Linux development headers, then run:

compile server:

```sh
gcc -std=c17 -O2 -Wall -Wextra -Werror \
  server.c session.c maps.c movement.c puzzles.c display.c network.c terminal.c \
  -o 2R1E-server
```

compile client:

```sh
gcc -std=c17 -O2 -Wall -Wextra -Werror \
  client.c session.c maps.c movement.c display.c network.c terminal.c \
  -o 2R1E-client
```

Run:

```sh
./2R1E-server
```

```sh
./2R1E-client
```

## Connect

1. Player 1/Server chooses an unused TCP port from `1025` to `65535`.
2. Player 1/Server tells Player 2/Client the address and port and waits for the connection.
3. Player 2 enters that same host address and port.

Use the host's LAN address. If either player disconnects, start a new session.

## Controls

The active player has 10 seconds to start moving or interacting. Their first
movement or interaction starts a 30-second countdown shown on both screens.
When time runs out, unused AP are forfeited and the other player takes over.
Puzzle progress is kept, and unfinished answers must be entered again.

| Key           | Action                                           |
| ------------- | ------------------------------------------------ |
| W / A / S / D | Move and face that direction                     |
| E             | Inspect or use the object directly ahead         |
| Enter         | Confirm a menu choice or submit an answer        |
| Backspace     | Delete a character while typing                  |
| Escape        | Cancel text entry or connection setup            |
| N             | Leave gameplay (`Y` / `N` or `Esc` confirmation) |
| T             | Pass Turn                                        |

Stand beside an object, face it, and press E. When `Answer>` appears, type the answer and press Enter. Movement keys and Q become text while typing. Wrong answers can be retried.

Each turn starts with 10 action points (AP). A successful move costs 1 AP.
Facing a wall or another blocked tile costs no AP. Some puzzle actions also
cost AP. Press T to pass the rest of your turn.

Each player solves their own puzzles and earns a key for each map. Use your
own E exit to advance. The first player to escape Map 3 wins.

After the match, press Enter on each terminal to return to the connection prompt.

## Answer keys to puzzles

Both players complete these steps in their own areas.

| Map | Object                            | Answer or action              |
| --- | --------------------------------- | ----------------------------- |
| 1   | N, Three Blanks                   | Enter `light passage watcher` |
| 1   | K, cabinet                        | Enter `246`                   |
| 1   | H, hatch                          | Enter `259`                   |
| 1   | E, exit                           | Use after earning the key     |
| 2   | B, statue                         | Use to open the passage       |
| 2   | D, dial panel, or T, test console | Enter `2 3 4`                 |
| 2   | E, exit                           | Use after earning the key     |
| 3   | 4, White Return valve             | Use to obtain clearance       |
| 3   | A, authorization terminal         | Use after clearance           |
| 3   | S, mechanical safety release      | Use after clearance           |
| 3   | E, exit                           | Use after earning the key     |
