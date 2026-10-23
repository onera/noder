#ifndef IO_HDF5_CGNS_CGNS_NAME_LIMITS_HPP
#define IO_HDF5_CGNS_CGNS_NAME_LIMITS_HPP

#include <cstddef>

namespace io::hdf5::cgns {

// These limits count std::string bytes. CGNS's 32-byte name limit is the
// standard path; Noder's longer names use a non-standard HDF5 name attribute.
constexpr std::size_t kCGNSNameSoftLimitBytes = 32;
constexpr std::size_t kCGNSNameHardLimitBytes = 64;

} // namespace io::hdf5::cgns

#endif // IO_HDF5_CGNS_CGNS_NAME_LIMITS_HPP
