# stdbuf -oL forces the server to output line-by-line even when piped.
# > >(...) sends the output to sed, but leaves the server as the main background process so $! gets the server's PID.

stdbuf -oL -eL ./bin/server --port 40004 --tnum 2 > >(sed -u 's/^/[SERVER 1] /') 2>&1 &
pid1=$!
stdbuf -oL -eL ./bin/server --port 40005 --tnum 2 > >(sed -u 's/^/[SERVER 2] /') 2>&1 &
pid2=$!

trap 'kill "$pid1" "$pid2" 2>/dev/null; wait' EXIT

sleep 1

./bin/client --k 7000 --mod 7919 --servers ./servers
./bin/client --k 17000 --mod 17389 --servers ./servers
