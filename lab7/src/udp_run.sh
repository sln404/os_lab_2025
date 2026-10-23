stdbuf -oL -eL ./bin/udpserver --port 40004 --bufsize 16 > >(sed -u 's/^/[SERVER] /') 2>&1 &
pid1=$!

trap 'kill "$pid1" "$pid2" 2>/dev/null; wait' EXIT

sleep 1

./bin/udpclient --ip 127.0.0.1 --port 40004 --bufsize 16
