#!/usr/bin/sh

SCRIPT_DIR=valgrind
SUPP_DIR=$SCRIPT_DIR/suppressions

#   --show-reachable=yes \
#   --track-origins=yes \

valgrind \
   --leak-check=full \
   --error-limit=no \
   --suppressions=$SUPP_DIR/issues.supp \
   -v \
   $1
