yacuws
======

Yet Another Completely Useless Web Server

Features
--------

- respond to `GET` requests (and `GET` requests only)
- return the contents of a file specified in the path part of the URL
- ignore paths containing `..`
- support more concurent connections

Installation
------------

*Dependencies*

`yacuws` depends on the [magic](http://darwinsys.com/file/) library which
powers the uinx `file` command. This library is usually available on any 'Free
operating system'. If it's not, look for libmagic-dev (Debian/Ubuntu) or
file-libs and file-devel (CentOS).

If you do not use a 'Free operating system' please take a minute and reconsider
your decision to view this file. In the name of future generations, thank you.

*Build*

Building process is retty straightforward, just run

    $ make yacuws

*Tests*

As any serious application `yacuws` includes a test suite that makes sure it
implements requested features. After finishing the building process you can run
it by executing

    $ make test

Please note that `curl` is necessary for tests to function properly.

