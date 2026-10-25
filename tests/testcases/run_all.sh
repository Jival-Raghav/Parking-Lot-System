#!/bin/bash
# usage: ./run_all.sh ./parking
BIN=${1:-./parking}
mkdir -p out
for i in $(seq 1 25); do
  f=input$i.txt
  timeout 10 "$BIN" "$f" > out/out$i.txt 2> out/err$i.txt
  code=$?
  want=0; [ "$i" = "23" ] && want=1
  status="OK"; [ "$code" != "$want" ] && status="UNEXPECTED EXIT CODE (wanted $want)"
  echo "$f exit=$code $status"
done
echo
echo "--- valid files: compare with expected (whitespace amount ignored) ---"
for i in 1 2 3 4 5; do
  if diff -b -q out/out$i.txt expected/expected$i.txt > /dev/null; then echo "input$i: MATCH"; else echo "input$i: DIFFERENT"; fi
done
