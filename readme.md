# ccalc-lib (libccalc-rel.a, libccalc-dbg.a)
## Synopsis
This is the backend library for my advanced text-based calculator apps.
libccalc-rel.a is the static library for release builds and libccalc-dbg.a is
the static library for debugging builds.
- Supports complex arithmetic, integer arithmetic and bitwise operations
- Supports implied multiplication (multiplication by justaposition)
- Supports 8, 16, 32, 64 and 128 bit integer types
- Supports binary, octal, decimal and hexadecimal numbers, both integer and
floating point (and complex)
- Supports floating point numbers with 100 decimal significant digits (+ guard
digits)
## Dependencies
This project depends on Boost 1.74.0. Note: newer versions produce a slightly
incorrect result for the expression sin(pi), which should be 0 and which 1.74.0
produces; by extension the expression exp(pi*i) where i is the imaginary unit is
also likewise erroneous; thus version 1.74.0 is statically asserted for in the
code.

The Boost 1.74.0 header file directory "boost" is assumed to be either under
/usr/local/include/boost_1_74_0 or in the system include search path. The
Boost binaries are not used.

This project uses the GNU __int128 type with GNU extensions enabled.
## Frontends
- ccalc-cli is the project for the command line interface frontend
- ccalc-gtk is the project for the GUI frontend developed using the GTK toolkit
(gtkmm-4 for C++)
- ccalc_gtk3 is the project for the GUI frontend developed using the GTK toolkit
(gtkmm-3 for C++)
## Build Quick Help
- 'make' or 'make release' builds the release static library libccalc-rel.a in a
'lib' directory under the current working directory, with the object files built
in a 'release' directory under the current working directory
- 'make debug' builds the debug static library libccalc-dbg.a in a 'lib'
directory under the current working directory, with the object files built in a
'debug' directory under the current working directory
- 'make install' builds the release static library as described above, unless
it's already so, and installs the header files to /usr/local/include/ccalc and
the library file to /usr/local/lib
- 'make installdbg' builds the debug static library as described above, unless
it's already so, and installs the header files to /usr/local/include/ccalc and
the library file to /usr/local/lib
- 'make clean' deletes the lib, release and debug directories under the current
working directory
- 'make uninstall' deletes the installed ccalc include directory and the
installed library files
- Note: the header files are the same for the release and debug libraries