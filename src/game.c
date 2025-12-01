

#include <efiprot.h>
#include <efi.h>
#include <efilib.h>

struct graphics_context {
    int width;
    int height;
};


enum game_stage {
    POSITIONING,
    FIGHTING
};

enum active_player {
    PLAYER_1,
    PLAYER_2,
};

struct game {
    enum game_stage stage;
};


struct board {
    int length;
    int width;

};
int blue = 0;
void drawBoard(struct graphics_context *ctx, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {

    int width = ctx->width / 20;
    int height = ctx->height / 20;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int posX = ctx->width / 10 * i;
            int posY = ctx->height / 10 * j;
            EFI_GRAPHICS_OUTPUT_BLT_PIXEL pixel;
            pixel.Blue = blue;
            pixel.Green = 0;
            pixel.Red = 100;
            uefi_call_wrapper(gop->Blt, 10, gop, &pixel, EfiBltVideoFill, 0 ,0, posX, posY, width, height, 0);
        }
    }
}

void doPositioning(struct graphics_context ctx, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {

    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    while (1) {    
        while (Status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &Key) == EFI_NOT_READY);
        if (Key.UnicodeChar == L'w') {
            blue+= 10;
        }
        drawBoard(&ctx, gop);
    }
}

void start_game(struct graphics_context ctx, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {

    // drawBoard(&ctx, gop);
    doPositioning(ctx, gop);
    
}