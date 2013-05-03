====> Install SDK

Copy the tools/install_sdk script into an empty directory and run it there. It would fetch
and install to ${HOME}/ImpalaSDK required compilers.

====> define environment

Build Impala Operating System require suitable environment:
	IMPALA_SRCROOT      root source catalog
	IMPALA_ARCH         architecture (x86)
	IMPALA_MK           $IMAPAL_SRCROOT/mk
	IMPALA_OUTPUT	    output catalog

Additional variables:
	ELF_PREFIX     	    Default "i386-elf" - prefix for ELF tools
	AOUT_PREFIX         Default "i386-aout-" - prefix for a.out tools


Definitions can be found in conf/DEFS.sh. In case of shells compatible with
Bourne Shell you can use command:
	$ source conf/DEFS.sh
In order to define required environment.

====> prepare source tree

Run the prepare.sh script and the run "make depend"

====> build system

Run the "make build"

====> build images

Run the "make distribution", then you can find floppy images in the distribution subdirectory.


