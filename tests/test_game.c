#include "game.h"
#include "levels.h"
#include "test.h"

#include <ctype.h>
#include <stddef.h>

static Game game;

static const char *const SOLUTIONS[] = {
    "RurD",
    "uruurrDLulD",
    "uUrrDuluLdddrR",
    "rrruulllllddRRUdRD",
    "lUdrruLLUlDrrruulL",
    "drrUlulLulDururrDDD",
    "UdlddrUluRRRDrdLuuRuulDrdLLLLuuR",
    "rUruLLLDldddrrrUUUruLddlLDldRuuulDD",
    "ululldRurDrrurrdLLdddrruLdlUlldlluRR",
};

static Direction letter_to_dir(char c) {
  switch (tolower((unsigned char)c)) {
  case 'u':
    return DIR_UP;
  case 'd':
    return DIR_DOWN;
  case 'l':
    return DIR_LEFT;
  default:
    return DIR_RIGHT;
  }
}

static void test_load_valid(void) {
  const char *err = NULL;
  EXPECT(game_load(&game, "#####\n#@$.#\n#####\n", &err));
  EXPECT_EQ(game.width, 5);
  EXPECT_EQ(game.height, 3);
  EXPECT_EQ(game.player_x, 1);
  EXPECT_EQ(game.player_y, 1);
  EXPECT_EQ(game.box_count, 1);
  EXPECT_EQ(game_cell(&game, 0, 0), CELL_WALL);
  EXPECT_EQ(game_cell(&game, 1, 1), CELL_PLAYER);
  EXPECT_EQ(game_cell(&game, 2, 1), CELL_BOX);
  EXPECT_EQ(game_cell(&game, 3, 1), CELL_GOAL);
  EXPECT_EQ(game_cell(&game, 99, 99), CELL_VOID);
  EXPECT(!game_is_solved(&game));
}

static void test_load_marks_outside_as_void(void) {
  EXPECT(game_load(&game, "  ####\n###@ #\n#.$  #\n######\n", NULL));
  EXPECT_EQ(game_cell(&game, 0, 0), CELL_VOID);
  EXPECT_EQ(game_cell(&game, 4, 1), CELL_FLOOR);
}

static void test_load_errors(void) {
  const char *err = NULL;
  EXPECT(!game_load(&game, "#####\n#$.#\n#####\n", &err));
  EXPECT(err != NULL);
  EXPECT(!game_load(&game, "#####\n#@@$.#\n#####\n", NULL));
  EXPECT(!game_load(&game, "#####\n#@$$.#\n#####\n", NULL));
  EXPECT(!game_load(&game, "#####\n#@  #\n#####\n", NULL));
  EXPECT(!game_load(&game, "#####\n#@$.\n#####\n", NULL));
  EXPECT(!game_load(&game, "#####\n#@$.x#\n#####\n", NULL));
}

static void test_move_and_push(void) {
  EXPECT(game_load(&game, "######\n#@$ .#\n#    #\n######\n", NULL));
  EXPECT_EQ(game_move(&game, DIR_UP), MOVE_BLOCKED);
  EXPECT_EQ(game_move(&game, DIR_LEFT), MOVE_BLOCKED);
  EXPECT_EQ(game_move(&game, DIR_RIGHT), MOVE_PUSH);
  EXPECT_EQ(game_cell(&game, 3, 1), CELL_BOX);
  EXPECT_EQ(game_move(&game, DIR_RIGHT), MOVE_PUSH);
  EXPECT_EQ(game_cell(&game, 4, 1), CELL_BOX_ON_GOAL);
  EXPECT(game_is_solved(&game));
  EXPECT_EQ(game_move(&game, DIR_RIGHT), MOVE_BLOCKED);
  EXPECT_EQ(game_move(&game, DIR_DOWN), MOVE_STEP);
  EXPECT_EQ(game.moves, 3);
  EXPECT_EQ(game.pushes, 2);
}

static void test_cannot_push_two_boxes(void) {
  EXPECT(game_load(&game, "#######\n#@$$..#\n#######\n", NULL));
  EXPECT_EQ(game_move(&game, DIR_RIGHT), MOVE_BLOCKED);
  EXPECT_EQ(game.moves, 0);
}

static void test_undo(void) {
  EXPECT(game_load(&game, "######\n#@$ .#\n#    #\n######\n", NULL));
  EXPECT(!game_undo(&game));
  game_move(&game, DIR_RIGHT);
  game_move(&game, DIR_DOWN);
  EXPECT(game_undo(&game));
  EXPECT_EQ(game.player_x, 2);
  EXPECT_EQ(game.player_y, 1);
  EXPECT(game_undo(&game));
  EXPECT_EQ(game.player_x, 1);
  EXPECT_EQ(game_cell(&game, 2, 1), CELL_BOX);
  EXPECT_EQ(game_cell(&game, 3, 1), CELL_FLOOR);
  EXPECT_EQ(game.moves, 0);
  EXPECT_EQ(game.pushes, 0);
  EXPECT(!game_undo(&game));
}

static void test_history_overflow(void) {
  EXPECT(game_load(&game, "######\n#@  .#\n#$   #\n######\n", NULL));
  for (int i = 0; i < GAME_MAX_HISTORY + 10; i++) {
    game_move(&game, i % 2 == 0 ? DIR_RIGHT : DIR_LEFT);
  }
  int undone = 0;
  while (game_undo(&game)) {
    undone++;
  }
  EXPECT_EQ(undone, GAME_MAX_HISTORY);
}

static void test_builtin_levels_solvable(void) {
  int count = levels_count();
  EXPECT_EQ(count, (int)(sizeof SOLUTIONS / sizeof SOLUTIONS[0]));
  EXPECT(levels_get(-1) == NULL);
  EXPECT(levels_get(count) == NULL);
  for (int i = 0; i < count && SOLUTIONS[i] != NULL; i++) {
    const char *err = NULL;
    EXPECT(game_load(&game, levels_get(i), &err));
    int pushes = 0;
    for (const char *c = SOLUTIONS[i]; *c != '\0'; c++) {
      MoveResult r = game_move(&game, letter_to_dir(*c));
      EXPECT_EQ(r, isupper((unsigned char)*c) ? MOVE_PUSH : MOVE_STEP);
      pushes += isupper((unsigned char)*c) ? 1 : 0;
    }
    EXPECT(game_is_solved(&game));
    EXPECT_EQ(game.pushes, pushes);
  }
}

int main(void) {
  RUN_TEST(test_load_valid);
  RUN_TEST(test_load_marks_outside_as_void);
  RUN_TEST(test_load_errors);
  RUN_TEST(test_move_and_push);
  RUN_TEST(test_cannot_push_two_boxes);
  RUN_TEST(test_undo);
  RUN_TEST(test_history_overflow);
  RUN_TEST(test_builtin_levels_solvable);
  return TEST_REPORT();
}
