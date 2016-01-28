The setup source file provided here is for use with...

* the NSIS compiler (http://nsis.sourceforge.net) -> the preferred version, or
* the InnoSetup compiler (http://www.jrsoftware.org).

NOTE: InnoSetup does not allow files from system dirs
(like windows/system32) to be added to the setup
(for security reasons).
So you must copy the wxwidgets, crash handler and mingwm10
DLL's to this directory, in order for setup to compile.

More setup compilers might be added at a later time.
