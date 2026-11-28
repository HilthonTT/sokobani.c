#ifndef SOKOBANI_GAME_H
#define SOKOBANI_GAME_H

#include <stdbool.h>

enum {
  GAME_MAX_WIDTH = 40,
  GAME_MAX_HEIGHT = 24,
  GAME_MAX_HISTORY = 4096,
};

typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;

typedef enum { MOVE_BLOCKED, MOVE_STEP, MOVE_PUSH } MoveResult;

typedef enum {
  CELL_VOID,
  CELL_WALL,
  CELL_FLOOR,
  CELL_GOAL,
  CELL_BOX,
  CELL_BOX_ON_GOAL,
  CELL_PLAYER,
  CELL_PLAYER_ON_GOAL,
} Cell;

typedef struct {
  int width;
  int height;
  unsigned char tiles[GAME_MAX_HEIGHT][GAME_MAX_WIDTH];
  bool boxes[GAME_MAX_HEIGHT][GAME_MAX_WIDTH];
  int player_x;
  int player_y;
  int box_count;
  int moves;
  int pushes;
  unsigned char history[GAME_MAX_HISTORY];
  int history_start;
  int history_len;
} Game;

bool game_load(Game *game, const char *level, const char **err);

MoveResult game_move(Game *game, Direction dir);

bool game_undo(Game *game);

bool game_is_solved(const Game *game);

int game_boxes_on_goal(const Game *game);

Cell game_cell(const Game *game, int x, int y);

#endif
