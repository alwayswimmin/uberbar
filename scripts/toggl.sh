#!/usr/bin/bash
PATH="$PATH":/usr/local/bin

RESULTX="$(/usr/local/bin/toggl now 2>/dev/null)"
RESULT="${RESULTX%x}"
NLINE=$(printf "$RESULTX" | wc -l)

if [ $NLINE -gt 1 ]; then
    printf "$RESULT" | head -n 1 | awk -F "#" '{printf $1}'
    printf "| "
    printf "$RESULT" | awk -F "Duration: " '{print $2}' | grep -v "^$" # | sed 's/\(.*\):.*/\1/'
else
    echo No timer is running.
fi
