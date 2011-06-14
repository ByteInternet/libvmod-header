===========
vmod_header
===========

---------------------
Varnish Header Module
---------------------

:Manual section: 3
:Author: Kristian Lyngstøl
:Date: 2011-06-14
:Version: 0.1

SYNOPSIS
========

::

        import header;

        header.append(<header>, <content>)
        header.get(<header>, <regular expression>)
        header.remove(<header>, <regular expression>)
        header.copy(<source header>, <destination header>)

DESCRIPTION
===========

Varnish Module (vmod) for manipulation of duplicated headers (for instance
multiple set-cookie headers).

FUNCTIONS
=========

Example VCL::

	backend foo { ... };

	import header;

	sub vcl_fetch {
		header.append(beresp.http.Set-Cookie,"foo=bar");
                header.remove(beresp.http.Set-Cookie,"dontneedthiscookie");
	}


append
------

Prototype
        header.append(<header>, <content>)
Returns
        void
Description
        Append lets you add an extra occurrence of an existing header.
Example
        ``header.append(beresp.http.Set-Cookie,"foo=bar")``

get
---

Prototype
        header.get(<header>, <regular expression>)
Returns
        String
Description
        Get fetches the value of the first `header` that matches the given
        regular expression.
Example
        ``set beresp.http.xusr = header.get(beresp.http.set-cookie,"user=");``

remove
------

Prototype
        header.remove(<header>, <regular expression>)
Returns
        void
Description
        remove() removes all occurences of `header` that matches the given
        regular expression.
Example
        ``header.remove(beresp.http.set-cookie,".*silly.*")``

copy
----

.. warning::

   Not implemented yet.

Prototype
        header.copy(<source header>, <destination header>)
Returns
        void
Description
        Copies all of the source headers to a new header.
Example
        ``header.copy(beresp.http.set-cookie, beresp.http.x-old-cookie);``

INSTALLATION
============

Installation requires the Varnish source tree (only the source matching the
binary installation).

1. `./autogen.sh`  (for git-installation)
2. `./configure VARNISHSRC=/path/to/your/varnish/source/varnish-cache`
3. `make`
4. `make install` (may require root: sudo make install)
5. `make check` (Optional for regression tests)

VARNISHSRCDIR is the directory of the Varnish source tree for which to
compile your vmod. Both the VARNISHSRCDIR and VARNISHSRCDIR/include
will be added to the include search paths for your module.

Optionally you can also set the vmod install dir by adding VMODDIR=DIR
(defaults to the pkg-config discovered directory from your Varnish
installation).


ACKNOWLEDGEMENTS
================

The development of this plugin was made possible by the sponsorship of 
Softonic, http://en.softonic.com/ .

Author: Kristian Lyngstøl <kristian@bohemians.org>, Varnish Software AS
Skeleton by Martin Blix Grydeland <martin@varnish-software.com>, vmods are
part of Varnish Cache 3.0 and beyond.

BUGS
====

You can't use dynamic regular expressions, which also holds true for normal
regular expressions in regsub(), but VCL isn't able to warn you about this
when it comes to vmods yet.

Header-copying is not yet implemented.

Some overlap with varnishd exists, this will be mended as Varnish 3.0
evolves.

COPYRIGHT
=========

This document is licensed under the same license as the
libvmod-header project. See LICENSE for details.

* Copyright (c) 2011 Varnish Software
