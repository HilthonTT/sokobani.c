#include "levels.h"

#include <stddef.h>

static const char *const LEVELS[] = {
    "#######\n"
    "#     #\n"
    "# @$  #\n"
    "#   . #\n"
    "#######\n",

    "  ######\n"
    "  #    #\n"
    "### $$ #\n"
    "#  .#. #\n"
    "# @    #\n"
    "########\n",

    "#######\n"
    "#.  # #\n"
    "# $   #\n"
    "#  #$ #\n"
    "##@  .#\n"
    " ######\n",

    "########\n"
    "#  .   #\n"
    "# #$## #\n"
    "# $@ . #\n"
    "## #$# #\n"
    " #  .  #\n"
    " #######\n",

    "######\n"
    "#.   #\n"
    "#  # #\n"
    "# $$ #\n"
    "#. @ #\n"
    "######\n",

    " ######\n"
    "##    #\n"
    "#  #$ ##\n"
    "# $  . #\n"
    "#.#@ $ #\n"
    "#   .  #\n"
    "########\n",

    " #######\n"
    " #  .  #\n"
    "## $#$ ##\n"
    "#. @   .#\n"
    "## $#$ ##\n"
    " #  .  #\n"
    " #######\n",

    "#######\n"
    "#. .  #\n"
    "# $#$ #\n"
    "#  @  #\n"
    "# $#$ #\n"
    "#. .  #\n"
    "#######\n",

    "#########\n"
    "#   #   #\n"
    "# $ . $ #\n"
    "## .@. ##\n"
    "# $ . $ #\n"
    "#   #   #\n"
    "#########\n",
};

int levels_count(void) {
  return (int)(sizeof LEVELS / sizeof LEVELS[0]);
}

const char *levels_get(int index) {
  if (index < 0 || index >= levels_count()) {
    return NULL;
  }
  return LEVELS[index];
}
