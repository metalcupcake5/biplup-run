#include "tonc.h"

#include "input.h"
#include "kinematics.h"
#include "object_utils.h"
#include "system_utils.h"
#include "text.h"
#include "types.h"

#include <stdbool.h>
#include <stdlib.h>

extern Game_State game_state;
extern int frame_counter;
extern int end_game_frame;

int main(void) {
  // initialize key variables and GBA screen
  init_main();

  // player
  Player *player = &(Player){0};
  player->obj_args = &(Object){0};
  player_constructor(player);

  // my obstacles :) TODO: wrap into for loops per obstacle type, which clears
  // out last magic numbers in main
  Obstacle *dactyl = &(Obstacle){0};
  dactyl->obj_args = &(Object){0};

  obstacle_constructor(dactyl, 1, FLOOR_LEVEL + DACTYL_HEIGHT_DIFF,
                       DACTYL_FRAME_SPAWN_THRESHOLD, AERODACTYL);
  Obstacle *cactus_1 = &(Obstacle){0};
  cactus_1->obj_args = &(Object){0};

  obstacle_constructor(cactus_1, 2, FLOOR_LEVEL, CACTUS_FRAME_SPAWN_THRESHOLD,
                       CACTUS);

  Obstacle *cactus_2 = &(Obstacle){0};
  cactus_2->obj_args = &(Object){0};
  obstacle_constructor(cactus_2, 3, FLOOR_LEVEL,
                       CACTUS_FRAME_SPAWN_THRESHOLD * 2, SUDOWOODO);

  Obstacle *cloud = &(Obstacle){0};
  cloud->obj_args = &(Object){0};
  obstacle_constructor(cloud, 4, FLOOR_LEVEL + CLOUD_HEIGHT_DIFF,
                       CLOUD_FRAME_SPAWN_THRESHOLD * 2, CLOUD);

  Obstacle *obstacles[OBSTACLE_AMOUNT] = {dactyl, cactus_1, cactus_2, cloud};

  blob_1.attr->attr0 =
      ATTR0_Y((int)blob_1.y) | ATTR0_SQUARE | ATTR0_4BPP | ATTR0_REG;
  blob_1.attr->attr1 = ATTR1_X((int)blob_1.x) | ATTR1_SIZE_16x16;
  blob_1.attr->attr2 = ATTR2_ID(0) | ATTR2_PRIO(0) | ATTR2_PALBANK(0);

  Object blob_2 = (Object){
      .attr = &oam_mem[1], .x = 250, .y = floor_level, .is_active = false};

  blob_2.attr->attr0 = ATTR0_Y((int)blob_2.y) | ATTR0_SQUARE | ATTR0_4BPP |
                       ATTR0_MODE(ATTR0_REG);
  blob_2.attr->attr1 = ATTR1_X((int)blob_2.x) | ATTR1_SIZE_16x16;
  blob_2.attr->attr2 = ATTR2_ID(0) | ATTR2_PRIO(1) | ATTR2_PALBANK(0);

  Object blob_3 = (Object){
      .attr = &oam_mem[2], .x = 250, .y = floor_level - 10, .is_active = false};

  blob_3.attr->attr0 = ATTR0_Y((int)blob_3.y) | ATTR0_SQUARE | ATTR0_4BPP |
                       ATTR0_MODE(ATTR0_REG);
  blob_3.attr->attr1 = ATTR1_X((int)blob_3.x) | ATTR1_SIZE_16x16;
  blob_3.attr->attr2 = ATTR2_ID(0) | ATTR2_PRIO(2) | ATTR2_PALBANK(0);

  despawn(&blob_2);
  despawn(&blob_3);

  REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0;
  REG_BG0CNT = BG_CBB(0) | BG_SBB(31) | BG_4BPP | BG_REG_32x32;

  direction offscreen;

  update_obj_x(&blob_1);
  update_obj_y(&blob_1);

  update_obj_x(&blob_2);
  update_obj_y(&blob_2);

  update_obj_x(&blob_3);
  update_obj_y(&blob_3);

  int obj_2_timer = 0;
  int obj_3_timer = 0;
  int frame_counter = 0;

  int obj_2_threshold = 100;
  int obj_3_threshold = 250;

  while (1) {
    vid_vsync();

    // the whole game is wrapped into a switch-case based on game state
    switch (game_state) {
    case PRE_GAME: { // game has not started
      pregame_key_input();
      break;
    }
    case GAME: { // main game

      // receive player input and update physics
      game_key_input(player);
      update_player_physics(player);

      // allow each object to move, spawn, or wait
      update_obstacles(obstacles);

      animation(player->obj_args, frame_counter, BIPLUP);

      // check if the player is colliding with each object
      if (!check_player_collision(player, obstacles)) {
        // score update
        if (frame_counter % COUNT_SCORE_ON_FRAME == 0) {
          score_update();
        }
      } else {
        // game ending consequence
        end_game();
      }
      // while loop loops is equivalent to amount of frames displayed
      break;
    }
    case POST_GAME: {
      end_text();
      if (frame_counter - end_game_frame > KEY_DEBOUNCE) {
        postgame_key_input(obstacles);
      }
      break;
    }
    }
    frame_counter++;
  }
  return EXIT_SUCCESS;
}
