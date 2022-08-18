#!/bin/bash

python3 -m http.server --directory / 9006 &
srcsrv=$!

cleanup() {
    echo "Exiting watcher..."
    kill -9 $srcsrv $rlistener
    exec 3>&-
    exit 0
}

exec 3<> >(websockifier 0.0.0.0 9005)
trap cleanup INT TERM
rm -f /tmp/reloader && mkfifo /tmp/reloader
(while cat /tmp/reloader; do :; done) 1>&3 &
rlistener=$!

if [ $AUTOBUILD -eq 1 ]; then
    while true; do
        make -j $(nproc) -l $(nproc) $MAKEOPTS && echo "Build OK" && echo reload 1>&3
        inotifywait -r -q -e modify --excludei "^\\." "${@}" &
        wait $!
    done
else
    wait $rlistener
fi