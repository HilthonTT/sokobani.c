#include "font_data.h"
#include "game.h"
#include "levels.h"

#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  WINDOW_WIDTH = 1024,
  WINDOW_HEIGHT = 768,
  HUD_TOP = 64,
  HUD_BOTTOM = 40,
  FONT_BASE = 16,
  BOARD_MARGIN = 24,
  MAX_TILE = 80,
};

static const float MOVE_DURATION = 0.09f;
static const float HINT_DURATION = 6.0f;

static const Color COLOR_BACKGROUND = {22, 24, 31, 255};
static const Color COLOR_FLOOR_A = {44, 48, 60, 255};
static const Color COLOR_FLOOR_B = {40, 44, 55, 255};
static const Color COLOR_WALL = {84, 92, 118, 255};
static const Color COLOR_WALL_LIGHT = {112, 121, 152, 255};
static const Color COLOR_WALL_DARK = {60, 66, 86, 255};
static const Color COLOR_GOAL = {242, 176, 64, 255};
static const Color COLOR_BOX = {186, 128, 70, 255};
static const Color COLOR_BOX_DARK = {128, 82, 42, 255};
static const Color COLOR_BOX_DONE = {82, 184, 112, 255};
static const Color COLOR_BOX_DONE_DARK = {44, 120, 70, 255};
static const Color COLOR_PLAYER = {96, 196, 246, 255};
static const Color COLOR_PLAYER_DARK = {46, 126, 182, 255};
static const Color COLOR_TEXT = {214, 218, 228, 255};
static const Color COLOR_TEXT_DIM = {116, 124, 144, 255};
static const Color COLOR_PANEL = {32, 35, 45, 255};
static const Color COLOR_PANEL_EDGE = {56, 61, 78, 255};

typedef struct {
  float tile;
  float x;
  float y;
} Board;

typedef struct {
  bool active;
  float t;
  Vector2 player_from;
  bool pushed;
  int box_x;
  int box_y;
  Vector2 box_from;
} Animation;

typedef struct {
  Game game;
  int level;
  Direction facing;
  Animation anim;
  bool all_done;
  bool show_help;
} App;

static Font ui_font;

static float ease_out(float t) {
  return 1.0f - (1.0f - t) * (1.0f - t);
}

static Vector2 lerp2(Vector2 a, Vector2 b, float t) {
  return (Vector2){a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

static Vector2 cell_vec(int x, int y) {
  return (Vector2){(float)x, (float)y};
}

static Rectangle cell_rect(const Board *board, Vector2 cell, float inset) {
  return (Rectangle){board->x + cell.x * board->tile + inset,
                     board->y + cell.y * board->tile + inset,
                     board->tile - 2.0f * inset, board->tile - 2.0f * inset};
}

static Vector2 cell_center(const Board *board, Vector2 cell) {
  return (Vector2){board->x + (cell.x + 0.5f) * board->tile,
                   board->y + (cell.y + 0.5f) * board->tile};
}

static Board layout_board(const Game *game) {
  float avail_w = (float)(GetScreenWidth() - 2 * BOARD_MARGIN);
  float avail_h =
      (float)(GetScreenHeight() - HUD_TOP - HUD_BOTTOM - 2 * BOARD_MARGIN);
  float tile =
      fminf(avail_w / (float)game->width, avail_h / (float)game->height);
  tile = floorf(fminf(tile, (float)MAX_TILE));
  if (tile < 8.0f) {
    tile = 8.0f;
  }
  Board board;
  board.tile = tile;
  board.x =
      floorf(((float)GetScreenWidth() - tile * (float)game->width) / 2.0f);
  board.y = floorf((float)HUD_TOP + BOARD_MARGIN +
                   (avail_h - tile * (float)game->height) / 2.0f);
  return board;
}

static void draw_floor(const Board *board, int x, int y) {
  Color color = (x + y) % 2 == 0 ? COLOR_FLOOR_A : COLOR_FLOOR_B;
  DrawRectangleRec(cell_rect(board, cell_vec(x, y), 0.0f), color);
}

static void draw_wall(const Board *board, int x, int y) {
  Rectangle r = cell_rect(board, cell_vec(x, y), 0.0f);
  float bevel = floorf(board->tile * 0.14f);
  DrawRectangleRec(r, COLOR_WALL);
  DrawRectangleRec((Rectangle){r.x, r.y, r.width, bevel}, COLOR_WALL_LIGHT);
  DrawRectangleRec((Rectangle){r.x, r.y + r.height - bevel, r.width, bevel},
                   COLOR_WALL_DARK);
  float mid = r.y + floorf(r.height / 2.0f);
  DrawLineEx((Vector2){r.x, mid}, (Vector2){r.x + r.width, mid}, 1.0f,
             COLOR_WALL_DARK);
  float joint = (y % 2 == 0) ? r.x + r.width * 0.5f : r.x;
  DrawLineEx((Vector2){joint, r.y + bevel}, (Vector2){joint, mid}, 1.0f,
             COLOR_WALL_DARK);
}

static void draw_goal(const Board *board, int x, int y, float time) {
  Vector2 c = cell_center(board, cell_vec(x, y));
  float pulse = 0.5f + 0.5f * sinf(time * 3.0f + (float)(x + y));
  float r = board->tile * 0.5f;
  DrawCircleV(c, r * 0.42f, ColorAlpha(COLOR_GOAL, 0.08f + 0.08f * pulse));
  DrawRing(c, r * 0.22f, r * 0.32f, 0.0f, 360.0f, 32, COLOR_GOAL);
  DrawCircleV(c, r * 0.1f, COLOR_GOAL);
}

static void draw_box(const Board *board, Vector2 cell, bool on_goal) {
  Color fill = on_goal ? COLOR_BOX_DONE : COLOR_BOX;
  Color dark = on_goal ? COLOR_BOX_DONE_DARK : COLOR_BOX_DARK;
  float t = board->tile;
  Rectangle shadow = cell_rect(board, cell, t * 0.1f);
  shadow.y += t * 0.05f;
  DrawRectangleRounded(shadow, 0.2f, 6, ColorAlpha(BLACK, 0.35f));

  Rectangle r = cell_rect(board, cell, t * 0.1f);
  DrawRectangleRounded(r, 0.2f, 6, fill);
  DrawRectangleRoundedLinesEx(r, 0.2f, 6, fmaxf(2.0f, t * 0.06f), dark);

  float pad = t * 0.2f;
  Rectangle inner = {r.x + pad, r.y + pad, r.width - 2.0f * pad,
                     r.height - 2.0f * pad};
  float thick = fmaxf(2.0f, t * 0.06f);
  DrawRectangleLinesEx(inner, fmaxf(1.0f, t * 0.04f), dark);
  DrawLineEx((Vector2){inner.x, inner.y},
             (Vector2){inner.x + inner.width, inner.y + inner.height}, thick,
             dark);
  DrawLineEx((Vector2){inner.x + inner.width, inner.y},
             (Vector2){inner.x, inner.y + inner.height}, thick, dark);
}

static void draw_player(const Board *board, Vector2 cell, Direction facing,
                        float bob) {
  float t = board->tile;
  Vector2 c = cell_center(board, cell);
  c.y -= bob * t * 0.03f;

  DrawEllipse((int)c.x, (int)(c.y + t * 0.3f), t * 0.26f, t * 0.08f,
              ColorAlpha(BLACK, 0.35f));
  DrawCircleV(c, t * 0.34f, COLOR_PLAYER_DARK);
  DrawCircleV((Vector2){c.x, c.y - t * 0.02f}, t * 0.3f, COLOR_PLAYER);

  static const float LOOK_X[] = {0.0f, 0.0f, -1.0f, 1.0f};
  static const float LOOK_Y[] = {-1.0f, 1.0f, 0.0f, 0.0f};
  Vector2 look = {LOOK_X[facing] * t * 0.06f, LOOK_Y[facing] * t * 0.05f};
  for (int side = -1; side <= 1; side += 2) {
    Vector2 eye = {c.x + (float)side * t * 0.11f + look.x,
                   c.y - t * 0.06f + look.y};
    DrawCircleV(eye, t * 0.075f, RAYWHITE);
    DrawCircleV((Vector2){eye.x + look.x * 0.5f, eye.y + look.y * 0.5f},
                t * 0.038f, (Color){24, 30, 44, 255});
  }
}

static bool is_box_cell(Cell cell) {
  return cell == CELL_BOX || cell == CELL_BOX_ON_GOAL;
}

static void draw_board(const App *app, const Board *board, float time) {
  const Game *game = &app->game;
  float k = app->anim.active ? ease_out(app->anim.t / MOVE_DURATION) : 1.0f;

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      Cell cell = game_cell(game, x, y);
      if (cell == CELL_VOID) {
        continue;
      }
      if (cell == CELL_WALL) {
        draw_wall(board, x, y);
        continue;
      }
      draw_floor(board, x, y);
      if (cell == CELL_GOAL || cell == CELL_BOX_ON_GOAL ||
          cell == CELL_PLAYER_ON_GOAL) {
        draw_goal(board, x, y, time);
      }
    }
  }

  for (int y = 0; y < game->height; y++) {
    for (int x = 0; x < game->width; x++) {
      Cell cell = game_cell(game, x, y);
      if (!is_box_cell(cell)) {
        continue;
      }
      Vector2 pos = cell_vec(x, y);
      bool moving = app->anim.active && app->anim.pushed &&
                    x == app->anim.box_x && y == app->anim.box_y;
      if (moving) {
        pos = lerp2(app->anim.box_from, pos, k);
      }
      draw_box(board, pos, cell == CELL_BOX_ON_GOAL && !moving);
    }
  }

  Vector2 player = cell_vec(game->player_x, game->player_y);
  float bob = 0.0f;
  if (app->anim.active) {
    player = lerp2(app->anim.player_from, player, k);
    bob = sinf(k * PI);
  }
  draw_player(board, player, app->facing, bob);
}

static float text_width(const char *text, int scale) {
  return MeasureTextEx(ui_font, text, (float)(FONT_BASE * scale), (float)scale)
      .x;
}

static void draw_text(const char *text, float x, float y, int scale,
                      Color color) {
  DrawTextEx(ui_font, text, (Vector2){floorf(x), floorf(y)},
             (float)(FONT_BASE * scale), (float)scale, color);
}

static void draw_text_centered(const char *text, float y, int scale,
                               Color color) {
  draw_text(text, ((float)GetScreenWidth() - text_width(text, scale)) / 2.0f, y,
            scale, color);
}

static void draw_hud(const App *app, const Board *board) {
  const Game *game = &app->game;
  char level[32];
  char total[16];
  char stats[64];
  snprintf(level, sizeof level, "Level %d", app->level + 1);
  snprintf(total, sizeof total, "/ %d", levels_count());
  snprintf(stats, sizeof stats, "%d moves   %d pushes", game->moves,
           game->pushes);

  float board_w = board->tile * (float)game->width;
  float left = board->x;
  float right = board->x + board_w;
  float total_x = text_width(level, 2) + 10.0f;
  float needed = total_x + text_width(total, 2) + text_width(stats, 2) + 48.0f;
  if (board_w < needed) {
    left = (float)BOARD_MARGIN;
    right = (float)(GetScreenWidth() - BOARD_MARGIN);
  }
  float y = board->y - (float)(FONT_BASE * 2) - 14.0f;

  draw_text(level, left, y, 2, COLOR_TEXT);
  draw_text(total, left + total_x, y, 2, COLOR_TEXT_DIM);
  draw_text(stats, right - text_width(stats, 2), y, 2, COLOR_TEXT_DIM);

  float t = (float)GetTime();
  if (t < HINT_DURATION && !app->show_help) {
    float alpha = fminf(1.0f, (HINT_DURATION - t) / 1.5f);
    draw_text_centered("Press ? for controls",
                       (float)(GetScreenHeight() - HUD_BOTTOM), 1,
                       ColorAlpha(COLOR_TEXT_DIM, alpha));
  }
}

static Rectangle draw_panel(float width, float height) {
  int w = GetScreenWidth();
  int h = GetScreenHeight();
  DrawRectangle(0, 0, w, h, ColorAlpha(BLACK, 0.5f));
  Rectangle panel = {floorf(((float)w - width) / 2.0f),
                     floorf(((float)h - height) / 2.0f), width, height};
  DrawRectangleRounded(panel, 0.06f, 8, COLOR_PANEL);
  DrawRectangleRoundedLinesEx(panel, 0.06f, 8, 2.0f, COLOR_PANEL_EDGE);
  return panel;
}

static void draw_help(void) {
  static const char *const ROWS[][2] = {
      {"Arrows, WASD", "Move"},  {"U, Z", "Undo"},
      {"R", "Restart level"},    {"N, P", "Next or previous level"},
      {"?", "Close this panel"}, {"Esc", "Quit"},
  };
  enum { ROW_COUNT = sizeof ROWS / sizeof ROWS[0] };
  const float pad = 40.0f;
  const float gap = 32.0f;
  const float row_h = 30.0f;

  float key_w = 0.0f;
  float action_w = 0.0f;
  for (int i = 0; i < ROW_COUNT; i++) {
    key_w = fmaxf(key_w, text_width(ROWS[i][0], 1));
    action_w = fmaxf(action_w, text_width(ROWS[i][1], 1));
  }
  Rectangle panel = draw_panel(2.0f * pad + key_w + gap + action_w,
                               104.0f + row_h * (float)ROW_COUNT);

  draw_text_centered("Controls", panel.y + 28.0f, 2, COLOR_TEXT);
  for (int i = 0; i < ROW_COUNT; i++) {
    float y = panel.y + 88.0f + row_h * (float)i;
    draw_text(ROWS[i][0], panel.x + pad, y, 1, COLOR_TEXT);
    draw_text(ROWS[i][1], panel.x + pad + key_w + gap, y, 1, COLOR_TEXT_DIM);
  }
}

static void draw_complete(const App *app) {
  const char *title = app->all_done ? "All levels complete" : "Level complete";
  const char *prompt =
      app->all_done ? "Press Enter to play again" : "Press Enter to continue";
  char stats[64];
  snprintf(stats, sizeof stats, "%d moves   %d pushes", app->game.moves,
           app->game.pushes);

  float width = fmaxf(text_width(title, 3), text_width(stats, 2));
  width = fmaxf(460.0f, width + 80.0f);
  Rectangle panel = draw_panel(width, 200.0f);
  draw_text_centered(title, panel.y + 32.0f, 3, COLOR_TEXT);
  draw_text_centered(stats, panel.y + 104.0f, 2, COLOR_TEXT_DIM);
  draw_text_centered(prompt, panel.y + 152.0f, 1, COLOR_TEXT_DIM);
}

static void load_level(App *app, int level) {
  const char *err = NULL;
  if (!game_load(&app->game, levels_get(level), &err)) {
    fprintf(stderr, "sokobani: level %d is invalid: %s\n", level + 1, err);
    CloseWindow();
    exit(EXIT_FAILURE);
  }
  app->level = level;
  app->facing = DIR_DOWN;
  app->all_done = false;
  app->show_help = false;
  memset(&app->anim, 0, sizeof app->anim);
}

static bool key_hit(int key) {
  return IsKeyPressed(key) || IsKeyPressedRepeat(key);
}

static bool read_direction(Direction *dir) {
  static const int KEYS[4][3] = {
      {KEY_UP, KEY_W, KEY_K},
      {KEY_DOWN, KEY_S, KEY_J},
      {KEY_LEFT, KEY_A, KEY_H},
      {KEY_RIGHT, KEY_D, KEY_L},
  };
  for (int d = 0; d < 4; d++) {
    for (int k = 0; k < 3; k++) {
      if (key_hit(KEYS[d][k])) {
        *dir = (Direction)d;
        return true;
      }
    }
  }
  return false;
}

static void try_move(App *app, Direction dir) {
  Game *game = &app->game;
  Vector2 from = cell_vec(game->player_x, game->player_y);
  app->facing = dir;

  MoveResult result = game_move(game, dir);
  if (result == MOVE_BLOCKED) {
    return;
  }
  app->anim.active = true;
  app->anim.t = 0.0f;
  app->anim.player_from = from;
  app->anim.pushed = result == MOVE_PUSH;
  if (app->anim.pushed) {
    int dx = game->player_x - (int)from.x;
    int dy = game->player_y - (int)from.y;
    app->anim.box_x = game->player_x + dx;
    app->anim.box_y = game->player_y + dy;
    app->anim.box_from = cell_vec(game->player_x, game->player_y);
  }
}

static void update(App *app, float dt) {
  if (IsKeyPressed(KEY_SLASH) || IsKeyPressed(KEY_F1)) {
    app->show_help = !app->show_help;
  }
  if (app->show_help) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      app->show_help = false;
    }
    return;
  }

  if (app->anim.active) {
    app->anim.t += dt;
    if (app->anim.t >= MOVE_DURATION) {
      app->anim.active = false;
    }
  }

  bool solved = game_is_solved(&app->game) && !app->anim.active;
  if (solved) {
    app->all_done = app->level + 1 >= levels_count();
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
        IsKeyPressed(KEY_KP_ENTER)) {
      load_level(app, (app->level + 1) % levels_count());
    } else if (key_hit(KEY_U) || key_hit(KEY_Z) || key_hit(KEY_BACKSPACE)) {
      game_undo(&app->game);
    } else if (IsKeyPressed(KEY_R)) {
      load_level(app, app->level);
    }
    return;
  }

  Direction dir;
  if (read_direction(&dir)) {
    app->anim.active = false;
    try_move(app, dir);
  } else if (key_hit(KEY_U) || key_hit(KEY_Z) || key_hit(KEY_BACKSPACE)) {
    app->anim.active = false;
    game_undo(&app->game);
  } else if (IsKeyPressed(KEY_R)) {
    load_level(app, app->level);
  } else if (IsKeyPressed(KEY_N)) {
    load_level(app, (app->level + 1) % levels_count());
  } else if (IsKeyPressed(KEY_P)) {
    load_level(app, (app->level + levels_count() - 1) % levels_count());
  }
}

static void usage(FILE *stream) {
  fprintf(stream,
          "usage: sokobani [LEVEL]\n"
          "Push every box onto a goal. LEVEL is 1-%d (default 1).\n",
          levels_count());
}

int main(int argc, char **argv) {
  static App app;
  int level = 0;

  if (argc > 2) {
    usage(stderr);
    return EXIT_FAILURE;
  }
  if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      usage(stdout);
      return EXIT_SUCCESS;
    }
    char *end = NULL;
    long n = strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0' || n < 1 || n > levels_count()) {
      usage(stderr);
      return EXIT_FAILURE;
    }
    level = (int)n - 1;
  }

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  SetTraceLogLevel(LOG_ERROR);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sokobani");
  SetWindowMinSize(640, 480);
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  ui_font = LoadFontFromMemory(".ttf", FONT_DATA, (int)sizeof FONT_DATA,
                               FONT_BASE, NULL, 0);
  SetTextureFilter(ui_font.texture, TEXTURE_FILTER_POINT);

  load_level(&app, level);

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_ESCAPE) && !app.show_help) {
      break;
    }
    update(&app, GetFrameTime());

    Board board = layout_board(&app.game);
    BeginDrawing();
    ClearBackground(COLOR_BACKGROUND);
    draw_board(&app, &board, (float)GetTime());
    draw_hud(&app, &board);
    if (app.show_help) {
      draw_help();
    } else if (game_is_solved(&app.game) && !app.anim.active) {
      draw_complete(&app);
    }
    EndDrawing();
  }

  UnloadFont(ui_font);
  CloseWindow();
  return EXIT_SUCCESS;
}
