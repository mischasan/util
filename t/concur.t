#!/usr/bin/env bash

SCRIPT=/tmp/concurs
trap 'Q=$?; trap "" 0; kill $CONCPID; exit $Q' 0

set -ex
TEST='single event'
concurs >$SCRIPT & CONCPID=$$

# testy:<hostname> would enable a remote concurs host.

CONCURS=testy
t.concur

if [ $(cat $SCRIPT) = event ]
then ok - $TEST
else not ok - $TEST
fi
