# Exact floating-point representations

This program takes numbers in string form, converts them to floating-point type, and then displays the exact decimal values they represent.
For example:

```bash
$ display-float 0.2
0.2 = +0.200000000000000000002710505431213761085018632002174854278564453125
0.2 = +0.200000000000000011102230246251565404236316680908203125
0.2 = +0.20000000298023223876953125
```

Output appears using 80-, 64-, and 32-bit float formats.

# Building

To build the program, clone the repository and run `autogen.sh` to set up the build environment.
Then run `configure` and ` make`:

```bash
$ git clone https://github.com/rkennedy/exact-float.git
$ cd exact-float
$ ./autogen.sh
$ ./configure
$ make
```

## Dependencies

This project uses [Google Test][gtest] and [Google Mock][gmock].
To build the test program, run `configure` with the `--with-gmock` option or set `GMOCK_ROOT` to the directory where you have Google Mock source code.
(To use a version of Google Test other than what comes with Mock, use `--with-gtest` or set `GTEST_ROOT` separately.)
For example:

```bash
$ ./configure --with-gmock=$HOME/src/gmock-1.7.0
```

# Credit

Much of this code is inspired by the [ExactFloatToStr_JH0][1] project for Delphi by John Herbster.

[gtest]: https://code.google.com/p/googletest/
[gmock]: https://code.google.com/p/googlemock/
[1]: http://codecentral.embarcadero.com/Item/19421
