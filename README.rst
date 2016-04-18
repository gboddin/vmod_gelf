============
vmod_gelf : wip don't use
============

----------------------
Varnish 4.0 Gelf Module
----------------------

:Author: Matthew M. Boedicker
:Author: Maksim Naumov
:Author: Gregory Boddin 
:Date: 2016-04-18
:Version: 0.1
:Manual section: 3

SYNOPSIS
========

import gelf;

sub vcl_deliver {
  gelf.send("{...json formated packet...}", "127.0.0.1", 12345);
}

DESCRIPTION
===========

Varnish module to send gelf formatted packets over UDP from VCL.

The module goal is to log varnish events in Logstash/Graylog or any logging software supporting the GELF protocol.

FUNCTIONS
=========

gelf
-----

Prototype
        ::

                send(STRING S, STRING HOST, INT PORT)
Return value
	VOID
Description
        Sends S as UDP to HOST:PORT.
Example
        ::

                gelf.send("Hello world", "127.0.0.1", 12345);

INSTALLATION
============

If you received this package without a pre-generated configure script, you must
have the GNU Autotools installed, and can then run the 'autogen.sh' script. If
you received this package with a configure script, skip to the second
command-line under Usage to configure.

Usage::

 # Generate configure script
 ./autogen.sh

 # Execute configure script
 ./configure [PKG_CONFIG=PATH]

Make targets:

* make - builds the vmod
* make install - installs your vmod
* make check - runs the unit tests in ``src/tests/*.vtc``

HISTORY
=======

COPYRIGHT
=========

* Copyright (c) 2013 Matthew M. Boedicker
* Copyright (c) 2015 Maksim Naumov
