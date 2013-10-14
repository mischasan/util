# Shell functions implementing TAP protocol.
PATH=:$PATH
tap_plan=0 tap_done=0 tap_fail=0

plan_tests() { tap_plan=$1; echo 1..$1; }

diag() { echo >&2 "# $*"; }

ok() {  # cmd [name]
    local cmd="$1"; test $# = 1 || shift
    let tap_done=tap_done+1
    if eval "$cmd"
    then    echo "ok $tap_done - $*"
    else    echo "not ok $tap_done - $*"
            case "$*" in *TODO*) ;; *) let tap_fail=tap_fail+1; esac
    fi
}
    
exit_status() {
    if   [ $1 != 0 ]
    then    diag "Test script ${0##*/} failed unexpectedly: $?"
    elif [ "$tap_fail" != 0 ]
    then    diag "Looks like you failed $tap_fail tests of $tap_plan."
    elif [ "$tap_done" -lt "$tap_plan" ]
    then    diag "Looks like you planned $tap_plan tests but only ran $tap_done."
    elif [ "$tap_plan" -a "$tap_done" -gt "$tap_plan" ]
    then    let tap_done=tap_done-tap_plan
            diag "Looks like you planned $tap_plan tests but ran $tap_done extra."
    else exit 0  # The non-failure case!
    fi
    exit 1
}

trap 'Q=$?; trap "" 0; exit_status $Q' 0
