for x in 4 _ _1 _12 _127 2 20 200 2006 2006- 2006-0 2006-09 2006-09- '|t=' '|t=<' '|Size=' \
        '|p=' '|p=0' '|a' '|a=' '|a=a' '|a=a/' '|a=a/e' '|a=a/eom' postgresql.org
do echo -n "$x	"; fgrep "$x" 88.tab >perf.inp; ./str_x perf.inp 10 "$x"; done
