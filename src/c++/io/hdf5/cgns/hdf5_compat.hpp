#pragma once

#include <hdf5.h>

#include <stdexcept>
#include <string>

namespace io::hdf5::cgns::hdf5_detail {

inline hsize_t group_link_count(const hid_t group, const std::string& context) {
    H5G_info_t info{};
    if (H5Gget_info(group, &info) < 0) {
        throw std::runtime_error("HDF5 error: " + context);
    }
    return info.nlinks;
}

inline std::string link_name_by_index(
    const hid_t group,
    const hsize_t index,
    const std::string& context) {
    const ssize_t nameLength = H5Lget_name_by_idx(
        group,
        ".",
        H5_INDEX_NAME,
        H5_ITER_INC,
        index,
        nullptr,
        0,
        H5P_DEFAULT);
    if (nameLength < 0) {
        throw std::runtime_error("HDF5 error: " + context);
    }

    std::string name(static_cast<size_t>(nameLength) + 1, '\0');
    const ssize_t readLength = H5Lget_name_by_idx(
        group,
        ".",
        H5_INDEX_NAME,
        H5_ITER_INC,
        index,
        name.data(),
        name.size(),
        H5P_DEFAULT);
    if (readLength < 0) {
        throw std::runtime_error("HDF5 error: " + context);
    }
    name.resize(static_cast<size_t>(readLength));
    return name;
}

inline H5O_type_t object_type_by_index(
    const hid_t group,
    const hsize_t index,
    const std::string& context) {
#if H5_VERSION_GE(1, 12, 0)
    H5O_info2_t info{};
    if (H5Oget_info_by_idx3(
            group,
            ".",
            H5_INDEX_NAME,
            H5_ITER_INC,
            index,
            &info,
            H5O_INFO_BASIC,
            H5P_DEFAULT) < 0) {
#else
    // HDF5 1.10 uses the pre-1.12 signature and H5O_info_t structure.
    H5O_info_t info{};
    if (H5Oget_info_by_idx(
            group,
            ".",
            H5_INDEX_NAME,
            H5_ITER_INC,
            index,
            &info,
            H5P_DEFAULT) < 0) {
#endif
        throw std::runtime_error("HDF5 error: " + context);
    }
    return info.type;
}

} // namespace io::hdf5::cgns::hdf5_detail
