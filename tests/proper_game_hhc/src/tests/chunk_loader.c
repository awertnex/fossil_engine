#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>

#include <raylib.h>
#include <rlgl.h>

#include <engine/h/defines.h>
#include <engine/logger.c>
#include "../chunking.c"
#include "../dir.c"

u8 active = 1;
u8 text_vertical_spacing = 25;
u8 font_size = 18;

enum ChunkTokens
{
    TOKEN_VERSION =             2,    // bytes for chunk_file format version
    TOKEN_ID =                  4,    // bytes for chunk->id
    TOKEN_BLOCK_PARSE_LENGTH =  4,    // bytes for chunk->block_parse_limit
    TOKEN_BLOCK =               2,    // bytes for each chunk->i
    TOKEN_INFO =                2,    // bytes for each chunk->info
};

str **tokens_loaded;

FILE *chunk_file;
DIR *dir;
struct dirent *drnt;
str files[32][NAME_MAX + 1] = {0};
u16 file_count;

Chunk chunk =
{
    .pos = {-1, 4095},
    .block = {3, 35, 45, 34, 8, 11, 30},
    .flag = 1,
};

// -----------------------------------------------------------------------------
void tokenize_chunk();
void update_chunk();
void update_chunk_directory();
void unload_chunk();
void input();
void gui();

void serialize_chunk_test(Chunk *chunk, str *world_name)
{

}

void deserialize_chunk_test(Chunk *chunk, str *world_name)
{
    chunk_file = fopen(files[0], "rb");
    if (chunk_file)
    {
        tokenize_chunk();
        fclose(chunk_file);
        LOGINFO("%s", "Chunk Loaded 'chunk'");
    }
    else
    {
        LOGINFO("%s", "Chunk Created 'chunk'");
        update_chunk();
    }
}

void tokenize_chunk()
{
    struct stat buf;
    stat(files[0], &buf);

    if (buf.st_size < 1)
    {
        LOGERROR("%s", "Chunk file empty");
        return;
    }

    str *save_file_contents = (str*)malloc(sizeof(Chunk));
    for (u32 i = 0; i < sizeof(Chunk) && i < (u64)buf.st_size; ++i)
        save_file_contents[i] = getc(chunk_file);

    u8 stage = 1, parse = 1;
    for (u64 i = 0, c = 0; i < (u64)buf.st_size; ++i)
    {
        switch (stage)
        {
            case 1:
                for (u8 j = 0; j < strlen(tokens_default[stage - 1]); ++j ,++c)
                {
                    if (save_file_contents[c] != tokens_default[stage - 1][j])
                    {
                        parse = 0;
                        break;
                    }
                }
                if (parse)
                    snprintf(tokens_loaded[stage - 1],
                            strlen(tokens_default[stage - 1]),
                            "%s", tokens_default[stage - 1]);
                stage = 2;
                break;

            case 2:
                for (u8 j = 0; j < strlen(tokens_default[stage - 1]); ++j ,++c)
                {
                    if (save_file_contents[c] == '\n') ++c;
                    if (save_file_contents[c] != tokens_default[stage - 1][j])
                    {
                        parse = 0;
                        active = 0;
                        break;
                    }
                }
                if (parse)
                    snprintf(tokens_loaded[stage - 1],
                            strlen(tokens_default[stage - 1]),
                            "%s", tokens_default[stage - 1]);
                stage = 3;
                break;

            case 3:
                stage = 4;
                break;

            case 4:
                stage = 5;
                break;

            case 5:
                stage = 0;
                break;
        }

        if (!parse)
        {
            LOGERROR("%s", "Chunk file corrupted, process terminated");
            free(save_file_contents);
            return;
        }
    }

    LOGINFO("%s", "Chunk Loaded 'chunk'");
    printf("  CONTENTS: %s\n", save_file_contents);
    free(save_file_contents);
}

void update_chunk()
{
    chunk_file = fopen(files[0], "wb");
    fwrite(&chunk, sizeof(chunk), 1, chunk_file);
    fclose(chunk_file);
    return;
    printf("tokens loaded: %s\n", tokens_loaded[0]);
    chunk_file = fopen(files[0], "rb");
    fread(tokens_loaded[0], sizeof(tokens_loaded[0]), 1, chunk_file);
    fclose(chunk_file);
    printf("tokens loaded: %s\n", tokens_loaded[0]);
}

void update_chunk_directory()
{
    dir = opendir(mc_c_chunkpath);
    if (dir)
    {
        file_count = 0;
        for (u16 i = 0; i < 264 && files[i][0]; ++i)
            memset(files[i], 0, NAME_MAX);

        while ((drnt = readdir(dir)))
        {
            snprintf(files[file_count], NAME_MAX, "%s%s",
                    mc_c_chunkpath, drnt->d_name);
            ++file_count;
        }

        closedir(dir);
    }
}

void unload_chunk()
{
}

int main(void)
{
    mc_c_grandpath = (char*) malloc(PATH_MAX);
    mc_c_subpath = (char*) malloc(PATH_MAX);
    mc_c_chunkpath = (char*) malloc(PATH_MAX);
    tokens_loaded = (char**) calloc(10, 8);

    memset(mc_c_grandpath, 0, PATH_MAX);
    memset(mc_c_subpath, 0, PATH_MAX);
    memset(mc_c_chunkpath, 0, PATH_MAX);
    memset(tokens_loaded, 0, 80);

    snprintf(mc_c_grandpath, 64, "%s/Heaven-Hell Continuum/", getenv("HOME"));
    snprintf(mc_c_subpath, 128, "%stest_instance/", mc_c_grandpath);
    snprintf(mc_c_chunkpath, 256, "%schunk/", mc_c_subpath);

    mc_c_mkdir(mc_c_grandpath, 0775);
    mc_c_mkdir(mc_c_subpath, 0775);
    mc_c_mkdir(mc_c_chunkpath, 0775);

    InitWindow(1280, 720, "test: chunk_loader");
    SetTargetFPS(60);

    while (active)
    {
        input();
        gui();
    }

    free(mc_c_grandpath);
    free(mc_c_subpath);
    free(mc_c_chunkpath);
    free(tokens_loaded);
    unload_chunk();
    CloseWindow();
    return 0;
}

void input()
{
    if (IsKeyPressed(KEY_Q))
        active = 0;

    if (IsKeyPressed(KEY_ONE))
        update_chunk_directory();

    if (IsKeyPressed(KEY_TWO))
        load_chunk();

    if (IsKeyPressed(KEY_THREE))
        update_chunk();

    if (IsKeyPressed(KEY_FOUR))
        unload_chunk();
}

void gui()
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    DrawText("Controls:", 10, 10, font_size, RAYWHITE);
    DrawText("Loaded Chunk:", 500, 10, font_size, RAYWHITE);
    DrawText("Directory:", 1100, 10, font_size, RAYWHITE);
    DrawText("1: read chunk directory", 10, 10 + text_vertical_spacing,
            font_size, RAYWHITE);

    DrawText("2: load & read chunk file", 10, 10 + (text_vertical_spacing*2),
            font_size, RAYWHITE);

    DrawText("3: read chunk file", 10, 10 + (text_vertical_spacing*3),
            font_size, RAYWHITE);

    DrawText("4: unload chunk", 10, 10 + (text_vertical_spacing*4),
            font_size, RAYWHITE);

    for (u8 i = 0; i < 10; ++i)
        DrawText(tokens_loaded[i], 500, 10 + ((i + 1)*text_vertical_spacing),
                font_size, RAYWHITE);

    for (u16 i = 0; i < 32; ++i)
        DrawText(files[i], 1100, 10 + ((i + 1)*text_vertical_spacing),
                font_size, RAYWHITE);

    rlBegin(RL_QUADS);

    DrawRectangle(0, 670, 1280, 50, (Color){70, 70, 70, 255});
    rlEnd();

    EndDrawing();
}
