#!/usr/bin/sh

SCRIPT_DIR=valgrind
SUPP_DIR=$SCRIPT_DIR/suppressions

#   --show-reachable=yes \
#   --track-origins=yes \

valgrind \
   --leak-check=full \
   --error-limit=no \
   --gen-suppressions=all \
   --log-file=$SUPP_DIR/suppressions.log \
   --suppressions=$SUPP_DIR/issues.supp \
   $1

cat $SUPP_DIR/suppressions.log | $SCRIPT_DIR/valgrind-parse-suppressions.awk > $SUPP_DIR/current.supp
./valgrind/valgrind-make-fix-list.py $SUPP_DIR/current.supp > $SUPP_DIR/fix.supp
./valgrind/valgrind-make-lib-suppressions.py > $SUPP_DIR/libs.supp
