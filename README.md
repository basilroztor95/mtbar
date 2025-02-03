# mtbar

Multithreaded status bar for dwm.

## Configure

Add scripts to `config.hpp`

## Install

```sh
sudo make clean install
```

This will put the `mtbar` binary in `/usr/local/bin/` and assumes
gcc is the compiler on the system.

If you have llvm instead, use `make CXX=c++`.

## Dependencies

The project depends on a C++ compiler that understands C++11. It also requires
`libX11` for printing to the root window.
