#!/bin/bash

python3 -m http.server --directory / 9006 &
srcsrv=$!

cleanup() {
    echo "Exiting watcher..."
    kill -9 $srcsrv
    exec 3>&-
    exit 0
}

exec 3<> >(websockifier 0.0.0.0 9005)
trap cleanup INT TERM

while true; do
    make -j $(nproc) -l $(nproc) $MAKEOPTS && echo "Build OK" && echo reload 1>&3
    inotifywait -r -q -e modify --excludei "^\\." "${@}" &
    wait $!
done
