Sample File System Driver
------------------------

The sample file system driver code provided here can be used to
design and implement your own file system driver product.

!!!!!!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

The code provided here is for illustration purposes only. It is not
to be used as-is. Substantial modifications, enhancements, and testing
are required before installing any portion of the code on any machine.

DO NOT EXPECT THIS CODE TO WORK. IT IS ONLY TO BE USED AS A GUIDE
IN CONJUNCTION WITH THE TEXT CONTAINED IN THE BOOK IN DESIGNING YOUR
OWN FSD. IT IS SEVERELY LACKING IN FUNCTIONALITY AND HAS NEVER BEEN
INSTALLED OR TESTED AS IS COMMONLY REQUIRED.

TO COMPILE THE SOURCES PROVIDED HERE, YOU MUST HAVE A COPY OF THE
"ntifs.h" HEADER FILE PROVIDED BY MICROSOFT AS PART OF THE WINDOWS NT
IFS KIT.

!!!!!!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Files provided:
--------------

src\sfsdinit.c  - File system driver initialization functions
src\create.c    - Routines to support create/open requests
src\cleanup.c   - Stub functions for the cleanup dispatch entry point
src\close.c     - Stub functions for the close dispatch entry point
src\dircntrl.c  - Routines to support directory control requests
src\read.c      - Support for the read dispatch routine
src\write.c     - Support for the write dispatch routine
src\fastio.c    - The fast I/O entry points (and Cache Manager callbacks)
src\flush.c     - Support for the flush entry point
src\devcntrl.c  - Device IOCTL support
src\fileinfo.c  - Query/modify file attributes
src\shutdown.c  - Support for the shutdown notification call
src\misc.c      - Everything that didn't fit into the above, but was required
                  for compilation purposes
inc\protos.h    - Prototypes for all functions in files listed above
inc\sfsd.h      - The "main" include file; also contains useful macros
inc\struct.h    - Structure type definitions
inc\errmsg.h    - Automatically generated include file
                  (by the message compiler) that allows reporting of
                  FSD-specific error/warning/informational
                  messages in the event log (see the event subdirectory)
src\makefile    - makefile!
src\sources     - Contains list of files to be built; used by build.exe
README.TXT      - What you are reading


