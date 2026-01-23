#ifdef FSL_SUPER_DEBUGGER
#include "include/raylib.h"

#include "h/super_debugger.h"

#define SDB_ROW_HEIGHT  22
#define SDB_BASE_SIZE   5
#define SDB_BUTTON_SIZE 14

typedef struct debug_rect
{
    Rectangle corner_00;
    Rectangle corner_10;
    Rectangle corner_01;
    Rectangle corner_11;
    Rectangle edge_left;
    Rectangle edge_right;
    Rectangle edge_top;
    Rectangle edge_bottom;
    Rectangle rect_center;
    Vector2 pos;
    Vector2 scl;
} debug_rect;

debug_rect debug_rectangle =
{
    .corner_00 =    {0, 0, SDB_BASE_SIZE, SDB_BASE_SIZE},
    .corner_10 =    {SDB_BASE_SIZE, 0, SDB_BASE_SIZE, SDB_BASE_SIZE},
    .corner_01 =    {0, SDB_BASE_SIZE, SDB_BASE_SIZE, SDB_BASE_SIZE},
    .corner_11 =
    {SDB_BASE_SIZE, SDB_BASE_SIZE, SDB_BASE_SIZE, SDB_BASE_SIZE},

    .edge_left =    {0, SDB_BASE_SIZE, SDB_BASE_SIZE, 1},
    .edge_right =   {SDB_BASE_SIZE, SDB_BASE_SIZE, SDB_BASE_SIZE, 1},
    .edge_top =     {SDB_BASE_SIZE, 0, 1, SDB_BASE_SIZE},
    .edge_bottom =  {SDB_BASE_SIZE, SDB_BASE_SIZE, 1, SDB_BASE_SIZE},
    .rect_center =  {SDB_BASE_SIZE - 1, SDB_BASE_SIZE - 1,  2, 2},
};

Texture2D texture_super_debugger;
void init_super_debugger()
{
    texture_super_debugger =
        LoadTexture("resources/gui/container/super_debugger.png");

    debug_rectangle.scl.x = 300;
    debug_rectangle.scl.y = w_height - ((MARGIN + SDB_BASE_SIZE)*2);

    debug_rectangle.pos.x =
        w_width - debug_rectangle.scl.x - MARGIN - (SDB_BASE_SIZE*2);

    debug_rectangle.pos.y = MARGIN + SDB_BASE_SIZE;
}

void free_super_debugger()
{
    UnloadTexture(texture_super_debugger);
}

Rectangle debug_button_add =
{SDB_BASE_SIZE*2,                      0, SDB_BUTTON_SIZE, SDB_BUTTON_SIZE};

Rectangle debug_button_sub =
{(SDB_BASE_SIZE*2) + SDB_BUTTON_SIZE,  0, SDB_BUTTON_SIZE, SDB_BUTTON_SIZE};

u8 button_state_add = BUTTON_ACTIVE;
u8 button_state_sub = BUTTON_ACTIVE;

void draw_super_debugger()
{
    debug_rectangle.scl.y =
        w_height - ((MARGIN + SDB_BASE_SIZE)*2);
    debug_rectangle.pos.x =
        w_width - debug_rectangle.scl.x - MARGIN - (SDB_BASE_SIZE*2);

    /* ---- draw base ------------------------------------------------------- */
    draw_texture_simple(texture_super_debugger, debug_rectangle.rect_center,
            (Vector2){debug_rectangle.pos.x, debug_rectangle.pos.y},
            (Vector2){debug_rectangle.scl.x, debug_rectangle.scl.y},
            COL_TRANS_MENU);
    
    /* ---- draw edges ------------------------------------------------------ */
    draw_texture_simple(texture_super_debugger, debug_rectangle.edge_left,
            (Vector2){
            debug_rectangle.pos.x - SDB_BASE_SIZE,
            debug_rectangle.pos.y},
            (Vector2){
            SDB_BASE_SIZE,
            debug_rectangle.scl.y}, COL_TRANS_MENU);
    draw_texture_simple(texture_super_debugger, debug_rectangle.edge_right,
            (Vector2){
            debug_rectangle.pos.x + debug_rectangle.scl.x,
            debug_rectangle.pos.y},
            (Vector2){
            SDB_BASE_SIZE,
            debug_rectangle.scl.y}, COL_TRANS_MENU);
    draw_texture_simple(texture_super_debugger, debug_rectangle.edge_top,
            (Vector2){
            debug_rectangle.pos.x,
            debug_rectangle.pos.y - SDB_BASE_SIZE},
            (Vector2){
            debug_rectangle.scl.x,
            SDB_BASE_SIZE}, COL_TRANS_MENU);
    draw_texture_simple(texture_super_debugger, debug_rectangle.edge_bottom,
            (Vector2){
            debug_rectangle.pos.x,
            debug_rectangle.pos.y + debug_rectangle.scl.y},
            (Vector2){
            debug_rectangle.scl.x,
            SDB_BASE_SIZE}, COL_TRANS_MENU);

    /* ---- draw corners ---------------------------------------------------- */
    draw_texture(texture_super_debugger, debug_rectangle.corner_00,
            (Vector2){
            debug_rectangle.pos.x - SDB_BASE_SIZE,
            debug_rectangle.pos.y - SDB_BASE_SIZE},
            (Vector2){1, 1}, COL_TRANS_MENU);
    draw_texture(texture_super_debugger, debug_rectangle.corner_10,
            (Vector2){
            debug_rectangle.pos.x + debug_rectangle.scl.x,
            debug_rectangle.pos.y - SDB_BASE_SIZE},
            (Vector2){1, 1}, COL_TRANS_MENU);
    draw_texture(texture_super_debugger, debug_rectangle.corner_01,
            (Vector2){
            debug_rectangle.pos.x - SDB_BASE_SIZE,
            debug_rectangle.pos.y + debug_rectangle.scl.y},
            (Vector2){1, 1}, COL_TRANS_MENU);
    draw_texture(texture_super_debugger, debug_rectangle.corner_11,
            (Vector2){
            debug_rectangle.pos.x + debug_rectangle.scl.x,
            debug_rectangle.pos.y + debug_rectangle.scl.y},
            (Vector2){1, 1}, COL_TRANS_MENU);

    /* ---- draw the rest of the stuff -------------------------------------- */
    draw_button(texture_super_debugger, debug_button_add,
            (Vector2){
            debug_rectangle.pos.x + ((debug_rectangle.scl.x/3)*2),
            debug_rectangle.pos.y + (SDB_BASE_SIZE*2)}, &button_state_add, 0);

    draw_button(texture_super_debugger, debug_button_sub,
            (Vector2){
            debug_rectangle.pos.x + ((debug_rectangle.scl.x/3)*2) +
            (debug_button_add.width*2),
            debug_rectangle.pos.y + (SDB_BASE_SIZE*2)}, &button_state_sub, 0);
}
#endif /* FSL_SUPER_DEBUGGER */
