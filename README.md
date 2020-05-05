dir2list
========
shuffle subdirs and write filenames to stdout

Dependencies
------------
* POSIX Compliant C Library
* ANSI/C89 C compiler

Tested on `x86_64-linux-musl` but it should work elsewhere.

Install
-------
Edit `config.mk` and `config.h` to your liking and run make. Then to
install, using elevated privileges if neccesary:
```
make
doas make install
```

Limitations
-----------
* Speed
	- Probably gets slow with very large deeply nested directories.
* Race Conditions
	- No mechanism is put in place to handle the addition and
	  removal of files from working directories during operation.

