#!/usr/bin/env bash
. tap.sh

SCRIPT=/tmp/concurs
trap 'Q=$?; trap "" 0; kill $CONCPID; exit $Q' 0
set -ex
PATH=${0%/*}:$PATH
TEST='single event'
concurs >$SCRIPT & CONCPID=$$

# testy:<hostname> would enable a remote concurs host.

CONCURS=testy concur_t

if [ $(cat $SCRIPT) = event ]
then echo ok - $TEST
else echo not ok - $TEST
fi
