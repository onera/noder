# Third-party software notices

Noder wheels built with HDF5 I/O include the HDF5 library. Depending on the
platform, Noder either links HDF5 statically or the wheel repair step bundles
the shared HDF5 library and its runtime dependencies. The wheel's
`HDF5-BUILD-INFO.txt` records the detected HDF5 version and linkage mode.

The full HDF5 copyright notice and license terms are included in the wheel.
`HDF5-BUILD-INFO.txt` identifies the applicable file: HDF5 1.x builds use
`HDF5-COPYING.txt`, and HDF5 2.x builds use `HDF5-LICENSE.txt`. HDF5 is
distributed under a 3-clause BSD license.

HDF5 builds may also use zlib and libaec (which provides the SZIP-compatible
`libsz` library). The corresponding license texts are included as
`zlib-LICENSE.txt` and `libaec-LICENSE.txt` to cover platform build variants.
Whether these components are present in a particular wheel depends on the
HDF5 build and the platform's wheel repair policy.

Some HDF5 builds can also use OpenSSL through optional remote-storage
features. Both the OpenSSL 1.1.1 license and the OpenSSL 3.x Apache-2.0 license
are included as `OpenSSL-1.1.1-LICENSE.txt` and `OpenSSL-3-LICENSE.txt`
because the version supplied by the build environment can vary.

The checked-in license texts are from these upstream releases:

- HDF5 1.14.6: <https://github.com/HDFGroup/hdf5/tree/hdf5_1.14.6/COPYING>
- HDF5 2.2.0: <https://github.com/HDFGroup/hdf5/blob/2.2.0/LICENSE>
- zlib 1.3.1: <https://github.com/madler/zlib/blob/v1.3.1/zlib.h>
- libaec 1.1.3: <https://github.com/Deutsches-Klimarechenzentrum/libaec/blob/v1.1.3/LICENSE.txt>
- OpenSSL 1.1.1w: <https://github.com/openssl/openssl/blob/OpenSSL_1_1_1w/LICENSE>
- OpenSSL 3.0.0: <https://github.com/openssl/openssl/blob/openssl-3.0.0/LICENSE.txt>

These upstream components are not modified by Noder. This notice and the
associated license files are included in the installed `noder/licenses`
directory of binary distributions.
