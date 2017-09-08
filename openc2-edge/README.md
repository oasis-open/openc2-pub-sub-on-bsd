openc2-edge
===========

License: BSD 3-Clause License

This program receives OpenC2 messages and passes them on to the
configured plugins. The plugins (dynamically-loaded shared objects)
may optionally perform the requested operation.

Usage: ```edge -c config.json```

Building and installing openc2-edge
-----------------------------------

Requirements:

1. Though the C code is operating system agnostic, the Makefiles are
   BSD Makefiles. The code was developed and tested on HardenedBSD.
1. Dependencies: lizmq4, libucl

Building and installing:

1. make depend
1. make all
1. make install

A sample config file is located in the etc directory of this project.
Since libucl is used to parse it, the libucl syntax should be
followed.

Plugins
-------

As of 21 December 2016, there are two sample plugins:

1. openc2-edge-allow. This receives OpenC2 ALLOW messages and removes
   IP addresses from the firewall block list.
1. openc2-edge-deny. This receives OpenC2 DENY messages and adds IP
   address to the firewall block list.

The plugins live in a separate repo and stand independent of the
openc2-edge project. Building and installing them, however, requires
openc2-edge to be installed.
