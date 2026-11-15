# 2R1E: Two Rooms, One Exit

A two-player terminal escape game in C17. Explore separate areas, share clues by speaking to your partner, and solve three maps.

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

| Key           | Action                                    |
| ------------- | ----------------------------------------- |
| W / A / S / D | Move and face that direction              |
| E             | Inspect or use the object directly ahead  |
| Enter         | Confirm a menu choice or submit an answer |
| Backspace     | Delete a character while typing           |
| Escape        | Cancel text entry or connection setup     |
| Q             | Leave gameplay                            |

Stand beside an object, face it, and press E. When `Answer>` appears, type the answer and press Enter. Movement keys and Q become text while typing. Wrong answers can be retried.

Both players must use their own `E` exit object to advance on each map. After winning, press Enter on each terminal to return to connection prompt.

## Answer keys to puzzles

Player 1 is Host; Player 2 is Join.

| Map | Player | Object                            | Answer or action                        |
| --- | ------ | --------------------------------- | --------------------------------------- |
| 1   | 2      | N, Three Blanks                   | Enter `light passage watcher`           |
| 1   | 2      | K, cabinet                        | Enter `246`                             |
| 1   | 1      | H, hatch                          | Enter `259`                             |
| 1   | Both   | Their own E exit                  | Face it and press E                     |
| 2   | 1      | B, statue                         | Face it and press E to open the passage |
| 2   | 2      | D, dial panel, or T, test console | Enter `2 3 4` with spaces               |
| 2   | Both   | Their own E exit                  | Face it and press E                     |
| 3   | 1      | 4, White Return valve             | Face it and press E to obtain clearance |
| 3   | 1      | A, authorization terminal         | Face it and press E after clearance     |
| 3   | 2      | S, mechanical safety release      | Face it and press E after clearance     |
| 3   | Both   | Their own E exit                  | Face it and press E to win              |
