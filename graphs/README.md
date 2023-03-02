# Plotting the progress graphs

Enable the graph output in the code. First line to output data points to stderr, second line optionally to print the current board at the moment of the fallback.

```
245                     /* OUTPUT FOR GRAPHING - every fallback, print the current position
246                     fprintf(stderr, "%d %d\n", number_of_fallbacks, iterator);
247                     print_board_with_options();
248                     */
```

and compile..

```
gcc -o all-steps-with-options  solver.c
./all-steps-with-options 2> data-all > boards-all
```

With only the data file:

```
for i in $(seq 1 100); do
    ./solver > /tmp/result${i}
    LINES=$(cat /tmp/result${i} | wc -l )
    DATA=$((LINES-19))
    tail -n 19 /tmp/result${i} > /tmp/solution${i}
    head -n $DATA /tmp/result${i} > /tmp/data${i}
    graph --bitmap-size "2500x1500" -T png /tmp/data${i} > plot${i}
done
```

plotting very detailed graph

```
graph --bitmap-size "100000x1000" -T png data > plot.png
```
