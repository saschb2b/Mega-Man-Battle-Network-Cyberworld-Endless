# Standalone server runtime notices

The native server contains unmodified CPython runtime components and the
PyInstaller bootloader. Python is frozen with the server application; the
project does not change Python's source. No emulator or game data is bundled.

- `Python.txt`: CPython v3.13.16's
  [LICENSE](https://github.com/python/cpython/blob/v3.13.16/LICENSE).
- `Python-third-party.rst`: CPython v3.13.16's full
  [incorporated software notices](https://github.com/python/cpython/blob/v3.13.16/Doc/license.rst),
  including OpenSSL, expat, libffi and zlib.
- `bzip2.txt`: bzip2 1.0.8's
  [LICENSE](https://sourceware.org/git/?p=bzip2.git;a=blob_plain;f=LICENSE;hb=bzip2-1.0.8).
- `liblzma.txt` and `liblzma-0BSD.txt`: XZ Utils 5.8.1's
  [licensing overview](https://github.com/tukaani-project/xz/blob/v5.8.1/COPYING)
  and [0BSD license](https://github.com/tukaani-project/xz/blob/v5.8.1/COPYING.0BSD).
  The server may include liblzma through Python; it does not bundle the
  XZ command-line tools or their GPL scripts.
- `PyInstaller.txt` is copied into each native archive from the pinned
  PyInstaller wheel, including its bootloader exception.

The exact interpreter version and target are recorded in `BUILD.json`.
These notices cover components that can be included by Python's standard
library on the supported platforms; a platform may not need every component.
