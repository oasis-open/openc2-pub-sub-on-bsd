openc2broadcast
===============

License: BSD 3-Clause License

This simple application sends OpenC2 messages to the OpenC2 broadcaster.

Usage: ```openc2broadcast -s server [-m message] [-m message] ...```

Note: The -s option needs to come first.

Building openc2broadcast
------------------------

Requirements:

1. Though the C code is operating system agnostic, the Makefiles are
   BSD Makefiles. The code was developed and tested on HardenedBSD.
1. Dependencies: libzmq4

Building and installing:

1. make depend
1. make all
2. make install
