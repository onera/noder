#ifndef IO_HPP
#define IO_HPP

#include "node/node.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

#include "io/yaml/native_noder_yaml_io.hpp"

#ifdef ENABLE_HDF5_IO
#include "io/hdf5/cgns/cgns_io.hpp"
#endif

namespace io {

enum class FileFormat {
    Hdf5Cgns,
    Yaml
};

inline std::string normalized_extension(const std::string& filename) {
    std::string extension = std::filesystem::path(filename).extension().string();
    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char value) {
            return static_cast<char>(std::tolower(value));
        });
    return extension;
}

inline FileFormat detect_format(const std::string& filename) {
    const std::string extension = normalized_extension(filename);
    if (extension == ".yaml" || extension == ".yml") {
        return FileFormat::Yaml;
    }
    return FileFormat::Hdf5Cgns;
}

inline bool format_flattens_cgns_tree_root(const FileFormat format) {
    return format == FileFormat::Hdf5Cgns;
}

inline bool file_flattens_cgns_tree_root(const std::string& filename) {
    return format_flattens_cgns_tree_root(detect_format(filename));
}

/**
 * @brief Write a Node hierarchy to disk using the format inferred from @p filename.
 * @param filename Output file path.
 * @param node Root node to serialize.
 * @param cgnsVersion CGNS version metadata used by the CGNS/HDF5 backend.
 */
inline void write_node(
    const std::string& filename,
    std::shared_ptr<Node> node,
    const float& cgnsVersion = 3.1f) {

    switch (detect_format(filename)) {
        case FileFormat::Yaml:
            io::yaml::write_node(filename, std::move(node));
            return;
        case FileFormat::Hdf5Cgns:
#ifdef ENABLE_HDF5_IO
            io::hdf5::cgns::write_node(filename, std::move(node), cgnsVersion);
            return;
#else
            (void)node;
            (void)cgnsVersion;
            throw std::runtime_error(
                "io::write_node: HDF5/CGNS support is disabled. Use a '.yaml' filename or enable HDF5.");
#endif
    }

    throw std::runtime_error("io::write_node: unsupported file format");
}

/**
 * @brief Read a Node hierarchy from disk using the format inferred from @p filename.
 * @param filename Input file path.
 * @return Root node of the loaded hierarchy.
 */
inline std::shared_ptr<Node> read(const std::string& filename) {
    switch (detect_format(filename)) {
        case FileFormat::Yaml:
            return io::yaml::read(filename);
        case FileFormat::Hdf5Cgns:
#ifdef ENABLE_HDF5_IO
            return io::hdf5::cgns::read(filename);
#else
            throw std::runtime_error(
                "io::read: HDF5/CGNS support is disabled. Use a '.yaml' file or enable HDF5.");
#endif
    }

    throw std::runtime_error("io::read: unsupported file format");
}

} // namespace io

#endif // IO_HPP
