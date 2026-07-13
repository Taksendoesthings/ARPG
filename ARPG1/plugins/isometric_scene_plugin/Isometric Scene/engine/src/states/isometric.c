// ---------------------------------------------------------
/*
 *  Isometric Scene plugin for GB studio 4.2.1
 *
 *  by sl3DZ, version 2.2
 *
*/
// ---------------------------------------------------------
#pragma bank 255

#include "data/states_defines.h"
#include "states/isometric.h"

#include "actor.h"
#include "camera.h"
#include "collision.h"
#include "data_manager.h"
#include "game_time.h"
#include "input.h"
#include "trigger.h"
#include "math.h"
#include "vm.h"

#ifndef INPUT_ISOMETRIC_INTERACT
#define INPUT_ISOMETRIC_INTERACT INPUT_A
#endif

#ifndef ISOMETRIC_ANGLE
#define ISOMETRIC_ANGLE 0
#endif

#ifndef IS_MIRRORED
#define IS_MIRRORED 0
#endif

#if ISOMETRIC_ANGLE == 0
    #define MOVE_X PLAYER.move_speed
    #define MOVE_Y PLAYER.move_speed
#else
    #define MOVE_X PLAYER.move_speed
    #define MOVE_Y (PLAYER.move_speed >> 1)
#endif

// ---------------------------------------------------------

#define SLOPE_45_RIGHT      0x20u 
#define SLOPE_45_LEFT       0x30u 
#define SLOPE_22_RIGHT_BOT  0x40u 
#define SLOPE_22_RIGHT_TOP  0x60u 
#define SLOPE_22_LEFT_BOT   0x50u 
#define SLOPE_22_LEFT_TOP   0x70u

// ---------------------------------------------------------

#define INV_SLOPE_45_RIGHT  0x80u 
#define INV_SLOPE_45_LEFT   0x90u 
#define INV_SLOPE_22_R_BOT  0xA0u 
#define INV_SLOPE_22_R_TOP  0xC0u 
#define INV_SLOPE_22_L_TOP  0xD0u 
#define INV_SLOPE_22_L_BOT  0xB0u

// ---------------------------------------------------------

#define COLLISION_SLOPE_MASK 0xF0u

#ifndef COLLISION_SLOPE
#define COLLISION_SLOPE           0x20u
#endif

// ---------------------------------------------------------

static point16_t delta;
actor_t *hit_actor;

// ---------------------------------------------------------

void isometric_init(void) BANKED {
    camera_offset_x = 0;
    camera_offset_y = 0;
    camera_deadzone_x = 0;
    camera_deadzone_y = 0;
    delta.x = 0;
    delta.y = 0;
}

// ---------------------------------------------------------

void isometric_update(void) BANKED {
    direction_e new_dir = DIR_NONE;
    
    // ---------------------------------------------------------
    UWORD new_x, new_y;
    UBYTE tile;

    player_moving = FALSE;
    // ---------------------------------------------------------

    #if IS_MIRRORED == 0
        if (INPUT_LEFT)      { 
            new_dir = DIR_LEFT;  
            player_moving = TRUE; 
            delta.x = -MOVE_X; 
            delta.y = MOVE_Y; 
        }
        else if (INPUT_RIGHT){ 
            new_dir = DIR_RIGHT; 
            player_moving = TRUE; 
            delta.x = MOVE_X;  
            delta.y = -MOVE_Y; 
        }
        else if (INPUT_UP)   { 
            new_dir = DIR_UP;    
            player_moving = TRUE; 
            delta.x = -MOVE_X; 
            delta.y = -MOVE_Y; 
        }
        else if (INPUT_DOWN) { 
            new_dir = DIR_DOWN;  
            player_moving = TRUE;
            delta.x = MOVE_X;  
            delta.y = MOVE_Y; 
        }
    #else 
        if (INPUT_RIGHT)     { 
            new_dir = DIR_LEFT;  
            player_moving = TRUE; 
            delta.x = -MOVE_X; 
            delta.y = MOVE_Y; 
        }
        else if (INPUT_LEFT) { 
            new_dir = DIR_RIGHT; 
            player_moving = TRUE; 
            delta.x = MOVE_X;  
            delta.y = -MOVE_Y; 
        }
        else if (INPUT_DOWN) { 
            new_dir = DIR_UP;    
            player_moving = TRUE; 
            delta.x = -MOVE_X; 
            delta.y = -MOVE_Y; 
        }
        else if (INPUT_UP)   { 
            new_dir = DIR_DOWN;  
            player_moving = TRUE; 
            delta.x = MOVE_X;  
            delta.y = MOVE_Y; 
        }
    #endif

    if (player_moving) {
        new_x = PLAYER.pos.x + delta.x;
        new_y = PLAYER.pos.y + delta.y;

        UWORD test_x = new_x + (delta.x > 0 ? PLAYER.bounds.right : PLAYER.bounds.left);
        UWORD test_y = new_y + (delta.y > 0 ? PLAYER.bounds.bottom : PLAYER.bounds.top);

        tile = tile_at(SUBPX_TO_TILE(test_x), SUBPX_TO_TILE(test_y));

        if (tile & (COLLISION_SLOPE_MASK | COLLISION_ALL)) {
            UBYTE slope = (tile & COLLISION_SLOPE_MASK);
            
            if (slope) {
                UBYTE x_off = SUBPX_TILE_REMAINDER(test_x);
                UBYTE y_off = SUBPX_TILE_REMAINDER(test_y);
                UBYTE collide = FALSE;

                switch (slope) {

                    case SLOPE_45_RIGHT:     if (x_off + (PX_TO_SUBPX(8) - y_off) < PX_TO_SUBPX(8)) collide = TRUE; break;
                    case SLOPE_45_LEFT:      if (x_off < y_off) collide = TRUE; break;
                    case SLOPE_22_RIGHT_BOT: if (x_off + (PX_TO_SUBPX(8) - (y_off >> 1)) < PX_TO_SUBPX(8)) collide = TRUE; break;
                    case SLOPE_22_RIGHT_TOP: if (x_off + (PX_TO_SUBPX(8) - ((y_off >> 1) + PX_TO_SUBPX(4))) < PX_TO_SUBPX(8)) collide = TRUE; break;
                    case SLOPE_22_LEFT_BOT:  if (x_off < (y_off >> 1)) collide = TRUE; break;
                    case SLOPE_22_LEFT_TOP:  if (x_off < ((y_off >> 1) + PX_TO_SUBPX(4))) collide = TRUE; break;

                    case INV_SLOPE_45_RIGHT: if (x_off + (PX_TO_SUBPX(8) - y_off) > PX_TO_SUBPX(8)) collide = TRUE; break;
                    case INV_SLOPE_45_LEFT:  if (x_off > y_off) collide = TRUE; break;
                    case INV_SLOPE_22_R_BOT: if (x_off + (PX_TO_SUBPX(8) - (y_off >> 1)) > PX_TO_SUBPX(8)) collide = TRUE; break;
                    case INV_SLOPE_22_R_TOP: if (x_off + (PX_TO_SUBPX(8) - ((y_off >> 1) + PX_TO_SUBPX(4))) > PX_TO_SUBPX(8)) collide = TRUE; break;
                    case INV_SLOPE_22_L_BOT: if (x_off > (y_off >> 1)) collide = TRUE; break;
                    case INV_SLOPE_22_L_TOP: if (x_off > ((y_off >> 1) + PX_TO_SUBPX(4))) collide = TRUE; break;
                }

                if (collide) {
                    new_x = PLAYER.pos.x;
                    new_y = PLAYER.pos.y;
                }
            } else {

                new_x = PLAYER.pos.x;
                new_y = PLAYER.pos.y;
            }
        }
        PLAYER.pos.x = new_x;
        PLAYER.pos.y = new_y;
    }

    if (new_dir != DIR_NONE) {
            actor_set_dir(&PLAYER, new_dir, player_moving);
    } else {
            actor_set_anim_idle(&PLAYER);
    }

    if (hit_actor != NULL && (hit_actor->collision_group & COLLISION_GROUP_MASK)){
        player_register_collision_with(hit_actor);
    }

    else if (INPUT_PRESSED(INPUT_ISOMETRIC_INTERACT)) {
        hit_actor = actor_with_script_in_front_of_player(8);
        if (hit_actor && !(hit_actor->collision_group & COLLISION_GROUP_MASK) && hit_actor->script.bank) {
            actor_set_dir(hit_actor, FLIPPED_DIR(PLAYER.dir), FALSE);
            if (hit_actor->script.bank) {
                script_execute(hit_actor->script.bank, hit_actor->script.ptr, 0, 1, 0);
            }
        }
    }

    if (trigger_activate_at_intersection(&PLAYER.bounds, &PLAYER.pos, FALSE)) {
            return;
        }
}
