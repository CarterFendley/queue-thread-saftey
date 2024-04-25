# Queue Thread Saftey

Testing thread saftey of C++ data structures

## Setup instructions

Pulling submodules

```
git submodule init
git submodule update
```

Building and testing

```
./build.sh
./test.sh --verbose
```


## Results
- `MacOSX14.4.sdk`
    - **Count:** Yes
        - Seen almost consistenly after 4 workers and 250 iters
    - **Duplicates:** Yes
        - Seen almost consistenly after 4 workers and 250 iters
    - **Seg Faults:** Yes
        - Seen almost consistenly after 4 workers and 500 iters
