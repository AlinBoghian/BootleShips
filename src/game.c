#include <efiprot.h>
#include <efi.h>
#include <efilib.h>


#define SHIPS_PER_PLAYER 5
#define BOARD_HEIGHT 30
#define BOARD_LENGTH 40

struct graphics_context {
    int width;
    int height;
};

enum tile_t {HIT, NOT_HIT};

enum game_stage {
    POSITIONING,
    FIGHTING
};

struct position_t {
    int x;
    int y;
};

enum active_player {
    PLAYER_1,
    PLAYER_2,
};

#define MAX_TILES_SHIP 6

enum ship_shape_t {
    TWO_TILE = 0,
    THREE_TILE = 1,
    FAT_THREE_TILE = 2,
};

enum rotation_t {
    ROT_0, ROT_90, ROT_180, ROT_270
};

int coordinates_len[3] = {2, 3, 6};



struct position_t shape_coordinates[3][MAX_TILES_SHIP] = {
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
    enum tile_t board[BOARD_HEIGHT][BOARD_LENGTH];
    struct ship_t ships[SHIPS_PER_PLAYER];
    int selected_ship_idx;
    struct position_t selection;
    enum rotation_t rotation;
};

#define sum_pos(p1, p2) ((struct position_t) {.x = ((p1).x + (p2).x), .y = ((p1).y + (p2).y)})
#define rot_pos_90(pos) ((struct position_t) {.x = -((pos).y), .y = (pos).x })
#define rot_pos_180(pos) ((struct position_t) {.x = -(pos).x, .y = -((pos).y) })
#define rot_pos_270(pos) ((struct position_t) {.x = (pos).y, .y = -((pos).x) })

struct position_t rotate_position(struct position_t position, enum rotation_t rotation) {
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

void init_game(struct game *game) {
    game->stage = POSITIONING;
    game->selected_ship_idx = 0;
    game->selection = (struct position_t) {.x = 0, .y = 0};
    game->rotation = ROT_0;
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_LENGTH; j++) {
            game->board[i][j] = NOT_HIT;
        }
    }
    for (int i = 0; i < SHIPS_PER_PLAYER; i++) {
        game->ships[i] = (struct ship_t) {.shape = THREE_TILE};
    }
}


int position_inside_ship(struct position_t position, struct game *game) {
    enum ship_shape_t shape = game->ships[game->selected_ship_idx].shape;
    for (int i = 0; i < coordinates_len[shape]; i++) {
        struct position_t rotated_ship_coordinate = rotate_position(shape_coordinates[shape][i], game->rotation);
        struct position_t ship_coordinate = sum_pos(game->selection, rotated_ship_coordinate);
        if (position.x == ship_coordinate.x && position.y == ship_coordinate.y) {
            return 1;
        }
    }
    return 0;
}

void drawBoard(struct graphics_context *ctx, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, struct game *game) {
    int downLeftX, downLeftY, upRightX, upRightY;
    downLeftX = ctx->width * 2 / 10;
    downLeftY = ctx->height * 2 / 10;
    upRightX = ctx->width * 8 / 10;
    upRightY = ctx->height * 8 / 10;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixelBackground = {.Blue = 100, .Green = 0, .Red = 0};
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixelUnselected = {.Blue = 0, .Green = 0, .Red = 100};
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixelSelected = {.Blue = 0, .Green = 100, .Red = 0};

    uefi_call_wrapper(gop->Blt, 10, gop, &pixelBackground, EfiBltVideoFill, 0 ,0, downLeftX, downLeftY, upRightX - downLeftX, upRightY - downLeftY, 0);

    int width = (upRightX - downLeftX) / (BOARD_LENGTH * 12 / 10);
    int height = (upRightY - downLeftY) / (BOARD_HEIGHT * 12 / 10);;
    int driftX = (upRightX - downLeftX)  / BOARD_LENGTH - width;
    driftX /= 2;
    int driftY = (upRightY - downLeftY)  / BOARD_HEIGHT - height;
    driftY /= 2;
    for (int i = 0; i < BOARD_LENGTH; i++) {
        for (int j = 0; j < BOARD_HEIGHT; j++) {
            int selected = position_inside_ship((struct position_t) {.x = i, .y = j}, game);
            int posX = driftX + downLeftX + (upRightX - downLeftX) * i / BOARD_LENGTH;
            int posY = driftY + downLeftY + (upRightY - downLeftY) * j / BOARD_HEIGHT;
            uefi_call_wrapper(gop->Blt, 10, gop, selected ? &pixelSelected : &pixelUnselected, EfiBltVideoFill, 0 ,0, posX, posY, width, height, 0);
        }
    }
}

#define incBound(val, bound) (((val) >= (bound)) ? (bound) : (val + 1))
#define decBound(val, bound) (((val) <= (bound)) ? (bound) : (val - 1))

enum move_t {UP, DOWN, LEFT, RIGHT};

struct position_t applyMove(struct position_t pos, enum move_t move) {
    struct position_t newPos;
    if (move == UP) {
        return (struct position_t) {.x = pos.x, .y = pos.y - 1};
    } else if (move == DOWN) {
        return (struct position_t) {.x = pos.x, .y = pos.y + 1};
    } else if (move == LEFT) {
        return (struct position_t) {.x = pos.x - 1, .y = pos.y};
    } else if (move == RIGHT) {
        return (struct position_t) {.x = pos.x + 1, .y = pos.y};
    }
}


int position_oob(struct position_t position, enum ship_shape_t shape, enum rotation_t rotation) {
    for (int i = 0; i < coordinates_len[shape]; i++) {
        struct position_t rotated_ship_coordinate = rotate_position(shape_coordinates[shape][i], rotation);
        struct position_t checkedPosition = sum_pos(position, rotated_ship_coordinate);
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
    struct position_t newPos = applyMove(game->selection, move);
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

    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    int valid_cmd = 0;
    enum move_t move;
    int moveChanged = 0;
    while (!valid_cmd) {
        while (Status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &Key) == EFI_NOT_READY);
        valid_cmd = 1;
        if (Key.UnicodeChar == L'w') {
            move = UP;
            moveChanged = 1;
        } else if (Key.UnicodeChar == L'a') {
            move = LEFT;
            moveChanged = 1;
        } else if (Key.UnicodeChar == L's') {
            move = DOWN;
            moveChanged = 1;
        } else if (Key.UnicodeChar == L'd') {
            move = RIGHT;
            moveChanged = 1;
        } else if (Key.UnicodeChar == L'r') {
            game->rotation = cycle_rotation(game->rotation);
        } else if (Key.UnicodeChar == L'f') {
            game->ships[game->selected_ship_idx].shape = cycle_shape(game->ships[game->selected_ship_idx].shape);
        } else {
            valid_cmd = 0;
        }
    }
    if (moveChanged) {
        movePosition(move, game);
    }
}

void doPositioning(struct graphics_context ctx, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    struct game game;
    init_game(&game);
    drawBoard(&ctx, gop, &game);
    while (1) {    
        processInput(&game);
        drawBoard(&ctx, gop, &game);
    }
}

void start_game(struct graphics_context ctx, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    doPositioning(ctx, gop);
}