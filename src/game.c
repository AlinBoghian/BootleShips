#include "game.h"
#include "grid.h"

#define SHIPS_PER_PLAYER 5
#define BOARD_HEIGHT 30
#define BOARD_LENGTH 40
#define MAX_TILES_SHIP 6

enum tile_t {HIT, NOT_HIT};

enum move_t {UP, DOWN, LEFT, RIGHT};

enum game_stage {
    POSITIONING,
    FIGHTING
};

enum active_player {
    PLAYER_1,
    PLAYER_2,
};

enum ship_shape_t {
    TWO_TILE = 0,
    THREE_TILE = 1,
    FAT_THREE_TILE = 2,
};

enum rotation_t {
    ROT_0, ROT_90, ROT_180, ROT_270
};

int coordinates_len[3] = {2, 3, 6};

vec2 shape_coordinates[3][MAX_TILES_SHIP] = {
    {{.x = 0, .y = 0}, {.x = 1, .y = 0}}, // [][] shape
    {{.x = 0, .y = 0}, {.x = 1, .y = 0}, {.x = 2, .y = 0}}, // [][][] shape
    {{.x = 0, .y = 0}, {.x = 1, .y = 0}, {.x = 2, .y = 0},                // [][][] shape
    {.x = 0, .y = 1}, {.x = 1, .y = 1}, {.x = 2, .y = 1},},               // [][][]
};

struct ship_t {
    enum ship_shape_t shape;
    // struct position_t position;   
};

struct game {
    enum game_stage stage;
    read_key_blocking_t *read_key_blocking;
    enum tile_t board[BOARD_HEIGHT][BOARD_LENGTH];
    enum rotation_t rotation;
    struct ship_t ships[SHIPS_PER_PLAYER];
    struct grid_t grid;
    int selected_ship_idx;
    vec2 selection;
};

#define sum_pos(p1, p2) ((vec2) {.x = ((p1).x + (p2).x), .y = ((p1).y + (p2).y)})
#define rot_pos_90(pos) ((vec2) {.x = -((pos).y), .y = (pos).x })
#define rot_pos_180(pos) ((vec2) {.x = -(pos).x, .y = -((pos).y) })
#define rot_pos_270(pos) ((vec2) {.x = (pos).y, .y = -((pos).x) })

vec2 rotate_position(vec2 position, enum rotation_t rotation) {
    if (rotation == ROT_0) {
        return position;
    } else if (rotation == ROT_90) {
        return rot_pos_90(position);
    } else if (rotation == ROT_180) {
        return rot_pos_180(position);
    } else if (rotation == ROT_270) {
        return rot_pos_270(position);
    }
}

color shipstate2pixel(uint8_t state) {
    if (state == HIT) {
        return (color) {255, 0, 0};
    }
    return (color) {0, 255, 0};
}

void init_game(struct game *game, struct graphics_context gc, read_key_blocking_t *read_key_blocking) {
    game->stage = POSITIONING;
    game->selected_ship_idx = 0;
    game->selection = (vec2) {.x = 0, .y = 0};
    game->rotation = ROT_0;
    game->read_key_blocking = read_key_blocking;
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_LENGTH; j++) {
            game->board[i][j] = NOT_HIT;
        }
    }
    for (int i = 0; i < SHIPS_PER_PLAYER; i++) {
        game->ships[i] = (struct ship_t) {.shape = THREE_TILE};
    }
    init_grid(&game->grid, gc, (vec2){0,0}, (vec2){100, 100}, NOT_HIT, shipstate2pixel);
}

int position_inside_ship(vec2 position, struct game *game) {
    enum ship_shape_t shape = game->ships[game->selected_ship_idx].shape;
    for (int i = 0; i < coordinates_len[shape]; i++) {
        vec2 rotated_ship_coordinate = rotate_position(shape_coordinates[shape][i], game->rotation);
        vec2 ship_coordinate = sum_pos(game->selection, rotated_ship_coordinate);
        if (position.x == ship_coordinate.x && position.y == ship_coordinate.y) {
            return 1;
        }
    }
    return 0;
}

void change_tile_state(struct game *game) {
    for (int i = 0; i < BOARD_LENGTH; i++) {
        for (int j = 0; j < BOARD_HEIGHT; j++) {
            int selected = position_inside_ship((vec2) {.x = i, .y = j}, game);
            change_state(&game->grid, (vec2) {i, j}, selected);
        }
    }
}

vec2 applyMove(vec2 pos, enum move_t move) {
    vec2 newPos;
    if (move == UP) {
        return (vec2) {.x = pos.x, .y = pos.y - 1};
    } else if (move == DOWN) {
        return (vec2) {.x = pos.x, .y = pos.y + 1};
    } else if (move == LEFT) {
        return (vec2) {.x = pos.x - 1, .y = pos.y};
    } else if (move == RIGHT) {
        return (vec2) {.x = pos.x + 1, .y = pos.y};
    }
}

int position_oob(vec2 position, enum ship_shape_t shape, enum rotation_t rotation) {
    for (int i = 0; i < coordinates_len[shape]; i++) {
        vec2 rotated_ship_coordinate = rotate_position(shape_coordinates[shape][i], rotation);
        vec2 checkedPosition = sum_pos(position, rotated_ship_coordinate);
        if (checkedPosition.x >= BOARD_LENGTH || checkedPosition.x < 0) {
            return 1;
        }
        if (checkedPosition.y >= BOARD_HEIGHT || checkedPosition.y < 0) {
            return 1;
        }
    }
    return 0;
}

void movePosition(enum move_t move, struct game *game) {
    vec2 newPos = applyMove(game->selection, move);
    if (position_oob(newPos, game->ships[game->selected_ship_idx].shape, game->rotation)) {
        return;
    }
    game->selection = newPos;
};

enum rotation_t cycle_rotation(enum rotation_t rotation) {
    switch (rotation) {
        case ROT_0:
            return ROT_90;
        case ROT_90:
            return ROT_180;
        case ROT_180:
            return ROT_270;
        case ROT_270:
            return ROT_0;
    } 
}

enum ship_shape_t cycle_shape(enum ship_shape_t shape) {
    switch (shape) {
        case TWO_TILE:
            return THREE_TILE;
        case THREE_TILE:
            return FAT_THREE_TILE;
        case FAT_THREE_TILE:
            return TWO_TILE;
    }
}

void processInput(struct game *game) {
    int valid_cmd = 0;
    enum move_t move;
    int moveChanged = 0;
    while (!valid_cmd) {
        uint16_t keycode = game->read_key_blocking();
        valid_cmd = 1;
        if (keycode == L'w') {
            move = UP;
            moveChanged = 1;
        } else if (keycode == L'a') {
            move = LEFT;
            moveChanged = 1;
        } else if (keycode == L's') {
            move = DOWN;
            moveChanged = 1;
        } else if (keycode == L'd') {
            move = RIGHT;
            moveChanged = 1;
        } else if (keycode == L'r') {
            game->rotation = cycle_rotation(game->rotation);
        } else if (keycode == L'f') {
            game->ships[game->selected_ship_idx].shape = cycle_shape(game->ships[game->selected_ship_idx].shape);
        } else {
            valid_cmd = 0;
        }
    }
    if (moveChanged) {
        movePosition(move, game);
    }
}

void doPositioning(struct game *game, struct graphics_context ctx) {
    draw_grid(&game->grid);
    while (1) {
        // processInput(game);
        change_tile_state(game);
        draw_grid(&game->grid);
    }
}

void start_game(struct graphics_context ctx, read_key_blocking_t read_key_blocking) {
    struct game game;
    init_game(&game, ctx, read_key_blocking);
    doPositioning(&game, ctx);
}