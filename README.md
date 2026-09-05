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
- scanning PCI devices :) (not finished driver for PCI bus)

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
- tar (handles only USTAR archives) program with progress bar :-)
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

Host prerequisites for distribution image generation:

- `mtools` package (for `mcopy`)

Then in the distribution directory you will find those images:

- `distribution/default/floppy.img` - our system, with demos and all commands
- `distribution/minimal/floppy.img` - contains only programs needed to run system

amd64 status
============

The amd64 port is under active bring-up and is not marked complete yet.

Completion gate for amd64 support:

- [ ] boot to multiuser shell from generated image
- [ ] stable interrupts and scheduler under sustained load
- [ ] syscall ABI correctness for integer and pointer-sized values
- [ ] working filesystem, process, signal, and thread paths
- [ ] reproducible clean build and run procedure

Current implemented baseline:

- amd64 build defaults target 64-bit ELF toolchains
- amd64 trap/interrupt vectors use 64-bit entry stubs
- amd64 syscall entry/return path is enabled
- amd64 context switching and low-level CPU stubs use 64-bit registers
- amd64 libc has architecture-specific setjmp/longjmp assembly

Known limitations still being audited:

- runtime stress validation (interrupt, scheduler, and signal-heavy workloads)
- full end-to-end boot/runtime regression matrix for amd64
     
This repository was forked from https://bitbucket.org/wieczyk/impala/ in 2016. Now it lives it's own life...
