# Exact floating-point representations

This program takes numbers in string form, converts them to floating-point type, and then displays the exact decimal values they represent. For example:

```bash
$ display-float 0.2
0.2 = + 0.20000 00000 00000 00000 27105 05431 21376 10850 18632 00217 48542 78564 45312 5
0.2 = + 0.20000 00000 00000 01110 22302 46251 56540 42363 16680 90820 3125
0.2 = + 0.20000 00029 80232 23876 95312 5
```

Output appears using 80-, 64-, and 32-bit float formats.

# Building

To build the program, clone the repository and run `autogen.sh` to set up the build environment. Then run `configure` and ` make`:

```bash
$ git clone https://github.com/rkennedy/exact-float.git
$ cd exact-float
$ ./autogen.sh
$ ./configure
$ make
```
