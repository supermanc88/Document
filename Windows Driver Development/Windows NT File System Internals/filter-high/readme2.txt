Sample Filter Driver
-------------------

The sample filter driver provided here can be used to design
and implement kernel-mode filter drivers. This driver has been designed and
implemented to allow filtering file system requests.
Note that you should use the driver source provided here in conjunction with
the chapters in the text to better understand the subtleties of implementing
such kernel-mode filter drivers.

Building and Installing
-----------------------

1) Set the environment variables by running the SDK SETENV.BAT and the DDK
   SETENV.BAT.

2) Run BUILD.EXE with the -cef option.

3) The driver is built in the OBJ\I386\CHECKED (or FREE) directory; copy
   it to %WINDIR%\SYSTEM32\DRIVERS.

4) Run REGINI.EXE with SFILTER.INI as an argument.

5) Reboot the machine (be careful to have a debugger installed and
   executing for the target machine).

Restrictions:
------------

This sample driver attaches itself to all FSDs that register with
the I/O Manager. It doesn't attempt to attach itself to the RAW file system,
neither does it concern itself with network redirectors.

If you wish to use the code with Windows NT 3.51, you may need to
modify the source to open specific FSDs by name.

How to use:
----------

You should modify the source provided here (e.g., add appropriate print
statements) that will aid in understanding how FSD requests are
issued and processed. Use this source to serve as a template in
designing and implementing your own commercial filter driver.

Files provided:
--------------

src\sfilinit.c - Contains DriverEntry function that initializes the driver
src\fastio.c   - Contains fast I/O function calls implemented by the filter
src\misc.c     - Misc. routines (e.g., to initialize a device extension)
src\fsctrl.c   - Code to intercept FSD mount requests (and initiate attach)
src\dispatch.c - The "pass-thru" mode, i.e., we send everything through
src\create.c   - Special intercept routine for create/open requests
src\close.c    - Special intercept routine for cleanup and close requests
src\attach.c   - Contains routines to perform an attach to target and^M
                 a corresponding detach
inc\protos.h   - Prototypes for all functions in files listed above
inc\sfilter.h  - The "main" include file; also contains useful macros
inc\struct.h   - Structure type definitions.
src\makefile   - makefile!
src\sources    - Contains list of files to be built; used by build.exe
README.TXT     - What you are reading
sfilter.ini    - Use this file as an argument to regini.exe to modify
                 your registry and install the driver.
 

