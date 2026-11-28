#include "game.h"

#include <string.h>

enum { TILE_VOID, TILE_WALL, TILE_FLOOR, TILE_GOAL };

enum { HISTORY_PUSH_FLAG = 0x4, HISTORY_DIR_MASK = 0x3 };

static const int DX[] = {0, 0, -1, 1};
static const int DY[] = {-1, 1, 0, 0};

static bool in_bounds(const Game *game, int x, int y) {
  return x >= 0 && y >= 0 && x < game->width && y < game->height;
}

static bool is_wall(const Game *game, int x, int y) {
  return !in_bounds(game, x, y) || game->tiles[y][x] == TILE_WALL ||
         game->tiles[y][x] == TILE_VOID;
}

static bool set_error(const char **err, const char *msg) {
  if (err != NULL) {
    *err = msg;
  }
  return false;
}

static bool mark_interior(Game *game) {
  static int stack[GAME_MAX_WIDTH * GAME_MAX_HEIGHT];
  bool seen[GAME_MAX_HEIGHT][GAME_MAX_WIDTH];
  int top = 0;

  memset(seen, 0, sizeof seen);
  seen[game->player_y][game->player_x] = true;
  stack[top++] = game->player_y * GAME_MAX_WIDTH + game->player_x;

  while (top > 0) {
    int pos = stack[--top];
    int x = pos % GAME_MAX_WIDTH;
    int y = pos / GAME_MAX_WIDTH;
    for (int d = 0; d < 4; d++) {
      int nx = x + DX[d];
      int ny = y + DY[d];
      if (!in_bounds(game, nx, ny) || game->tiles[ny][nx] == TILE_VOID) {
        return false;
      }
      if (game->tiles[ny][nx] == TILE_WALL || seen[ny][nx]) {
        continue;
      }
      seen[ny][nx] = true;
      stack[top++] = ny * GAME_MAX_WIDTH + nx;
    }
  }

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      if (game->tiles[y][x] != TILE_WALL && !seen[y][x]) {
        game->tiles[y][x] = TILE_VOID;
        game->boxes[y][x] = false;
      }
    }
  }
  return true;
}

bool game_load(Game *game, const char *level, const char **err) {
  int players = 0;
  int x = 0;
  int y = 0;

  memset(game, 0, sizeof *game);

  for (const char *c = level; *c != '\0'; c++) {
    if (*c == '\n') {
      if (x > 0) {
        y++;
      }
      x = 0;
      continue;
    }
    if (x >= GAME_MAX_WIDTH || y >= GAME_MAX_HEIGHT) {
      return set_error(err, "level is too large");
    }
    switch (*c) {
    case '#':
      game->tiles[y][x] = TILE_WALL;
      break;
    case ' ':
    case '-':
    case '_':
      game->tiles[y][x] = TILE_FLOOR;
      break;
    case '.':
      game->tiles[y][x] = TILE_GOAL;
      break;
    case '$':
      game->tiles[y][x] = TILE_FLOOR;
      game->boxes[y][x] = true;
      break;
    case '*':
      game->tiles[y][x] = TILE_GOAL;
      game->boxes[y][x] = true;
      break;
    case '@':
    case '+':
      game->tiles[y][x] = *c == '+' ? TILE_GOAL : TILE_FLOOR;
      game->player_x = x;
      game->player_y = y;
      players++;
      break;
    default:
      return set_error(err, "level contains an unknown character");
    }
    x++;
    if (x > game->width) {
      game->width = x;
    }
    if (y + 1 > game->height) {
      game->height = y + 1;
    }
  }

  if (players != 1) {
    return set_error(err, "level must contain exactly one player");
  }

  if (!mark_interior(game)) {
    return set_error(err, "level is not enclosed by walls");
  }

  int goals = 0;
  for (int row = 0; row < game->height; row++) {
    for (int col = 0; col < game->width; col++) {
      if (game->tiles[row][col] == TILE_GOAL) {
        goals++;
      }
      if (game->boxes[row][col]) {
        game->box_count++;
      }
    }
  }
  if (game->box_count == 0) {
    return set_error(err, "level has no boxes");
  }
  if (goals != game->box_count) {
    return set_error(err, "number of boxes and goals differ");
  }
  return true;
}

static void history_push(Game *game, Direction dir, bool pushed) {
  unsigned char entry =
      (unsigned char)((unsigned)dir | (pushed ? HISTORY_PUSH_FLAG : 0u));
  if (game->history_len == GAME_MAX_HISTORY) {
    game->history_start = (game->history_start + 1) % GAME_MAX_HISTORY;
    game->history_len--;
  }
  int index = (game->history_start + game->history_len) % GAME_MAX_HISTORY;
  game->history[index] = entry;
  game->history_len++;
}

MoveResult game_move(Game *game, Direction dir) {
  int nx = game->player_x + DX[dir];
  int ny = game->player_y + DY[dir];

  if (is_wall(game, nx, ny)) {
    return MOVE_BLOCKED;
  }

  bool pushed = false;
  if (game->boxes[ny][nx]) {
    int bx = nx + DX[dir];
    int by = ny + DY[dir];
    if (is_wall(game, bx, by) || game->boxes[by][bx]) {
      return MOVE_BLOCKED;
    }
    game->boxes[ny][nx] = false;
    game->boxes[by][bx] = true;
    game->pushes++;
    pushed = true;
  }

  game->player_x = nx;
  game->player_y = ny;
  game->moves++;
  history_push(game, dir, pushed);
  return pushed ? MOVE_PUSH : MOVE_STEP;
}

bool game_undo(Game *game) {
  if (game->history_len == 0) {
    return false;
  }
  game->history_len--;
  int index = (game->history_start + game->history_len) % GAME_MAX_HISTORY;
  unsigned char entry = game->history[index];
  int dir = entry & HISTORY_DIR_MASK;
  int x = game->player_x;
  int y = game->player_y;

  if (entry & HISTORY_PUSH_FLAG) {
    game->boxes[y + DY[dir]][x + DX[dir]] = false;
    game->boxes[y][x] = true;
    game->pushes--;
  }
  game->player_x = x - DX[dir];
  game->player_y = y - DY[dir];
  game->moves--;
  return true;
}

int game_boxes_on_goal(const Game *game) {
  int count = 0;
  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      if (game->boxes[y][x] && game->tiles[y][x] == TILE_GOAL) {
        count++;
      }
    }
  }
  return count;
}

bool game_is_solved(const Game *game) {
  return game->box_count > 0 && game_boxes_on_goal(game) == game->box_count;
}

Cell game_cell(const Game *game, int x, int y) {
  if (!in_bounds(game, x, y)) {
    return CELL_VOID;
  }
  bool goal = game->tiles[y][x] == TILE_GOAL;
  if (x == game->player_x && y == game->player_y) {
    return goal ? CELL_PLAYER_ON_GOAL : CELL_PLAYER;
  }
  if (game->boxes[y][x]) {
    return goal ? CELL_BOX_ON_GOAL : CELL_BOX;
  }
  switch (game->tiles[y][x]) {
  case TILE_WALL:
    return CELL_WALL;
  case TILE_FLOOR:
    return CELL_FLOOR;
  case TILE_GOAL:
    return CELL_GOAL;
  default:
    return CELL_VOID;
  }
}
