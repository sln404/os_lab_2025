stdbuf -oL -eL ./bin/server --port 40004 --tnum 2 > >(sed -u 's/^/[SERVER 1] /') 2>&1 &
pid1=$!
stdbuf -oL -eL ./bin/server --port 40005 --tnum 2 > >(sed -u 's/^/[SERVER 2] /') 2>&1 &
pid2=$!

trap 'kill "$pid1" "$pid2" 2>/dev/null; wait' EXIT

read -n 1 -s -r
echo