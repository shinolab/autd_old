# autd3

## Dependencies

* Eigen3
* Boost
* Doxygen (optional)

## Build

```
mkdir build
cd build
cmake ..
make
```

## Installation Notes

* On windows, be sure to set environment variable `BOOST_ROOT` to let cmake find boost.

## Usage

See sample programs at `example/`.

### simple.cpp

It demostrates how to open server, add devices, set gain, set modulation, and close connection.
