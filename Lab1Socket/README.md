#### Install:
- ##### Make sure that you have a windows environment with gcc & cmake installed.
Note that *gcc*, not *clang* or *msvc*.
- ##### Edit the .conf files to set your server dir And put the Resource files into your server dir.

```shell
mkdir build && cd build
cmake .. -G “MinGW Makefiles” && make
./server
./client
```
