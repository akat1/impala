Welcome
=======

Impala is homemade UNIX-like operating system, it was developed as bechelor project at University of Wroclaw, Poland.

The 4.4BSD, FreeBSD, Ultrix and Solaris source codes had big impact on structure of our operating system, so probably it could be a good start to developing real UNIX-based system (not Linux based).

Authors
=======

- Mateusz Kocielski (shm)
- Artur Koninski (takeshi)
- Pawel Wieczorek (wieczyk)

Kernel features
===============

- virtual memory (supports i686 global pages extension)
- virtual file system layer, with FAT12, ext2, device file system (devfs) and memory file system support
- floppy controller driver with ISA DMA support
- ATA controller driver with PIO support
- kernel-level threads
- priority-based scheduler
- inter process communication: SystemV message queues, pipes, signals
- scanning PCI devies :) (not finished driver for PCI bus)

and very small thing:

- GRUB multiboot with command line

User-space features
===================

- library compatible with PTHREADS (maybe in details this compatibility fails)
- our libc
- programs are in AOUT/ZMAGIC format
- ported Almquist Shell (from the FreeBSD sources)
- ported VTTEST program
- ported minigzip program and libz library
- tar (handles only USTAR archives) program which progress bar :-)
- ported nvi (from FreeBSD sources)

Quality
=======

We put many effort to achieve good quality, but we had only few months to develop whole system, so it contains many bugs etc. If you are interrested in hacking operating system then Impala could be good sandbox for you.

Building
========

To build operating system you need binutils and gcc for i386-pc-aout and i386-pc-elf targets. Run the tools/install_sdk.sh script in empty directory, it will automaticly install toolchains (in the ${HOME}/ImpalaSDK directory).

When you have SDK, type those commands to build whole system:

- `source conf/DEFS.sh`
- `sh prepare.sh`
- `make build`

Running
=======

To prepare floppy images you need to type this command:

- `make distribution`

Then in the distribution directory you will find those images:

- `distribution/default/floppy.img` - our system, with demos and all commands
- `distribution/minimal/floppy.img` - contains only programs needed to run system
    
This repository was forked from https://bitbucket.org/wieczyk/impala/ in 2016. Not it lives it's own life...
