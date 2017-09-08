openc2-pub-sub-on-bsd
=========

This repository contains a set of projects that were created as the
OpenC2 reference implementation from G2, Inc. It aims to show OpenC2
working within a pub/sub context.

This implementation is written in C and developed on HardenedBSD.
Though the C code is operating system agnostic, the Makefiles are BSD
style Makefiles and will need modification to enable building on
Linux.

This project uses the BSD 3-clause license.

Requirements
------------

* libucl
* libzmq4
* HardenedBSD

Compiling and Installing
------------------------

After downloading this code repository:

```
# pkg install libucl libzmq4
# make depend all install
```

Subprojects
-----------

* openc2-broadcast: This project sends OpenC2 messages to the ZeroMQ
  relay (the openc2-orchestrator subproject).
* openc2-edge: This project receives OpenC2 messages and acts upon
  them by passing the messages to plugins.
* openc2-edge-allow: This project is a plugin to openc2-edge. It
  tells the firewall to allow an IP that was previously blocked.
* openc2-edge-deny: This project is a plugin to openc2-edge. It
  tells the firewall to deny an IP.
* openc2-orchestrator: This project is the ZeroMQ pub/sub relay. It
  simply receives messages to be broadcast and broadcasts them.

Running OpenC2
--------------

First, you will want to read the README.md files in openc2-broadcast
and openc2-edge. Configure openc2-edge by placing the configuration
file at /usr/local/etc/openc2.conf.

Second, ensure pf is running and configured with a blockedpeeps
table. Make sure entries in the blockedpeeps table get blocked.
Configuring pf itself is outside the scope of this document.

Third, start up the orchestrator:

```
# daemon /usr/bin/orchestrator
```

Fourth, start up the edge service:

```
# daemon /usr/bin/edge -c /usr/local/etc/openc2.conf
```

Fifth, send an OpenC2 message to the orchestrator:

```
# openc2broadcast -s localhost -m 'JSONified OpenC2 message goes here'
```
