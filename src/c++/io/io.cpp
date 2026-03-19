#ifdef ENABLE_HDF5_IO

#include "io/io.hpp"

#include "array/array.hpp"
#include "array/factory/matrices.hpp"
#include "array/factory/strings.hpp"

#include <hdf5.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace io {

namespace {

constexpr size_t kCGNSLongStringAttrLen = 33;
constexpr size_t kCGNSTypeAttrLen = 3;
constexpr unsigned kCGNSLinkCreationOrderFlags = H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED;

void check_status(herr_t status, const std::string& msg) {
    if (status < 0) {
        throw std::runtime_error("HDF5 error: " + msg);
    }
}

hid_t make_cgns_group_creation_plist() {
    hid_t gcpl = H5Pcreate(H5P_GROUP_CREATE);
    if (gcpl < 0) {
        throw std::runtime_error("HDF5 error: cannot create group creation property list");
    }
    check_status(H5Pset_link_creation_order(gcpl, kCGNSLinkCreationOrderFlags),
                 "set group link creation order");
    return gcpl;
}

hid_t make_cgns_file_creation_plist() {
    hid_t fcpl = H5Pcreate(H5P_FILE_CREATE);
    if (fcpl < 0) {
        throw std::runtime_error("HDF5 error: cannot create file creation property list");
    }
    check_status(H5Pset_link_creation_order(fcpl, kCGNSLinkCreationOrderFlags),
                 "set root group link creation order");
    return fcpl;
}

std::vector<hsize_t> toHSizeShape(const std::vector<size_t>& shape) {
    return std::vector<hsize_t>(shape.begin(), shape.end());
}

hid_t make_dataspace(const std::vector<size_t>& shape) {
    if (shape.empty()) {
        return H5Screate(H5S_SCALAR);
    }
    std::vector<hsize_t> dims = toHSizeShape(shape);
    return H5Screate_simple(static_cast<int>(dims.size()), dims.data(), nullptr);
}

hid_t hdfTypeFromCgnsType(const std::string& cgnsType) {
    if (cgnsType == "I1") return H5T_NATIVE_INT8;
    if (cgnsType == "I2") return H5T_NATIVE_INT16;
    if (cgnsType == "I4") return H5T_NATIVE_INT32;
    if (cgnsType == "I8") return H5T_NATIVE_INT64;
    if (cgnsType == "U1") return H5T_NATIVE_UINT8;
    if (cgnsType == "U2") return H5T_NATIVE_UINT16;
    if (cgnsType == "U4") return H5T_NATIVE_UINT32;
    if (cgnsType == "U8") return H5T_NATIVE_UINT64;
    if (cgnsType == "R4") return H5T_NATIVE_FLOAT;
    if (cgnsType == "R8") return H5T_NATIVE_DOUBLE;
    if (cgnsType == "X1") return H5T_NATIVE_INT8;
    throw std::runtime_error("Unsupported CGNS type: " + cgnsType);
}

std::string cgnsTypeFromArray(const Array& array) {
    if (array.hasDataOfType<int8_t>()) return "I1";
    if (array.hasDataOfType<int16_t>()) return "I2";
    if (array.hasDataOfType<int32_t>()) return "I4";
    if (array.hasDataOfType<int64_t>()) return "I8";
    if (array.hasDataOfType<uint8_t>()) return "U1";
    if (array.hasDataOfType<uint16_t>()) return "U2";
    if (array.hasDataOfType<uint32_t>()) return "U4";
    if (array.hasDataOfType<uint64_t>()) return "U8";
    if (array.hasDataOfType<float>()) return "R4";
    if (array.hasDataOfType<double>()) return "R8";
    if (array.hasDataOfType<bool>()) return "X1";
    if (array.hasString()) return "C1";
    throw std::runtime_error("Unsupported Array dtype for CGNS");
}

bool starts_with(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

bool is_cgns_tree_root(const std::shared_ptr<Node>& node) {
    return node && node->type() == "CGNSTree_t";
}

bool is_cgns_library_version_node(const std::shared_ptr<Node>& node) {
    return node && node->name() == "CGNSLibraryVersion" && node->type() == "CGNSLibraryVersion_t";
}

std::string persisted_cgns_path(const std::string& path, const std::string& cgnsTreeRootName) {
    if (cgnsTreeRootName.empty()) {
        return path;
    }

    const std::string absolutePrefix = "/" + cgnsTreeRootName;
    if (path == absolutePrefix) {
        return "/";
    }
    if (starts_with(path, absolutePrefix + "/")) {
        return path.substr(absolutePrefix.size());
    }

    if (path == cgnsTreeRootName) {
        return "";
    }
    if (starts_with(path, cgnsTreeRootName + "/")) {
        return path.substr(cgnsTreeRootName.size() + 1);
    }

    return path;
}

std::string persisted_link_target_path(
    const std::shared_ptr<Node>& node,
    const std::string& cgnsTreeRootName) {

    return persisted_cgns_path(node->linkTargetPath(), cgnsTreeRootName);
}

float resolved_cgns_version(const std::shared_ptr<Node>& root, const float& fallbackVersion) {
    if (!is_cgns_tree_root(root)) {
        return fallbackVersion;
    }

    for (const auto& child : root->children()) {
        if (!is_cgns_library_version_node(child) || child->noData()) {
            continue;
        }

        const auto array = std::dynamic_pointer_cast<Array>(child->dataPtr());
        if (!array || array->size() == 0) {
            continue;
        }

        if (array->hasDataOfType<float>()) {
            return array->getItemAtIndex<float>(0);
        }
        if (array->hasDataOfType<double>()) {
            return static_cast<float>(array->getItemAtIndex<double>(0));
        }
    }

    return fallbackVersion;
}

void add_string_attr_fixed_utf8(hid_t id, const std::string& key, const std::string& value, size_t length) {
    std::vector<char> buffer(length, '\0');
    std::strncpy(buffer.data(), value.c_str(), length);

    hid_t space = H5Screate(H5S_SCALAR);
    hid_t type = H5Tcopy(H5T_C_S1);
    H5Tset_size(type, length);
    H5Tset_strpad(type, H5T_STR_NULLPAD);
    H5Tset_cset(type, H5T_CSET_UTF8);
    hid_t attr = H5Acreate2(id, key.c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Awrite(attr, type, buffer.data()), "H5Awrite fixed string");
    H5Aclose(attr);
    H5Sclose(space);
    H5Tclose(type);
}

void add_cgns_name_attr(hid_t id, const std::string& value) {
    add_string_attr_fixed_utf8(id, "name", value, kCGNSLongStringAttrLen);
}

void add_cgns_label_attr(hid_t id, const std::string& value) {
    add_string_attr_fixed_utf8(id, "label", value, kCGNSLongStringAttrLen);
}

void add_cgns_type_attr(hid_t id, const std::string& value) {
    add_string_attr_fixed_utf8(id, "type", value, kCGNSTypeAttrLen);
}

void add_flags_attr(hid_t id, int32_t value = 1) {
    hsize_t dims[1] = {1};
    hid_t space = H5Screate_simple(1, dims, nullptr);
    hid_t attr = H5Acreate2(id, "flags", H5T_NATIVE_INT32, space, H5P_DEFAULT, H5P_DEFAULT);
    int32_t values[1] = {value};
    check_status(H5Awrite(attr, H5T_NATIVE_INT32, values), "H5Awrite flags");
    H5Aclose(attr);
    H5Sclose(space);
}

std::string read_string_attr(hid_t id, const std::string& key) {
    if (!H5Aexists(id, key.c_str())) return "";
    hid_t attr = H5Aopen(id, key.c_str(), H5P_DEFAULT);
    hid_t type = H5Aget_type(attr);

    std::string result;
    if (H5Tis_variable_str(type) > 0) {
        char* value = nullptr;
        check_status(H5Aread(attr, type, &value), "H5Aread string");
        result = value ? std::string(value) : std::string();
        if (value) H5free_memory(value);
    } else {
        const size_t size = static_cast<size_t>(H5Tget_size(type));
        std::vector<char> value(size + 1, '\0');
        check_status(H5Aread(attr, type, value.data()), "H5Aread fixed string");
        result = std::string(value.data());
    }

    H5Tclose(type);
    H5Aclose(attr);
    return result;
}

void write_int8_string_dataset(hid_t file, const std::string& dataset_path, const std::string& value) {
    std::vector<int8_t> buffer(value.begin(), value.end());
    buffer.push_back('\0');
    hsize_t dims[1] = {static_cast<hsize_t>(buffer.size())};
    hid_t space = H5Screate_simple(1, dims, nullptr);
    hid_t dset = H5Dcreate2(file, dataset_path.c_str(), H5T_NATIVE_INT8, space,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Dwrite(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
                 "write int8 string dataset: " + dataset_path);
    H5Dclose(dset);
    H5Sclose(space);
}

std::string read_int8_string_dataset(hid_t file, const std::string& dataset_path) {
    if (H5Lexists(file, dataset_path.c_str(), H5P_DEFAULT) <= 0) return "";
    hid_t dset = H5Dopen2(file, dataset_path.c_str(), H5P_DEFAULT);
    hid_t space = H5Dget_space(dset);
    hsize_t dims[1] = {0};
    H5Sget_simple_extent_dims(space, dims, nullptr);
    std::vector<int8_t> buffer(static_cast<size_t>(dims[0]));
    check_status(H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
                 "read int8 string dataset: " + dataset_path);
    H5Sclose(space);
    H5Dclose(dset);
    std::string value(buffer.begin(), buffer.end());
    while (!value.empty() && value.back() == '\0') {
        value.pop_back();
    }
    return value;
}

void write_cgns_file_metadata(hid_t file) {
    add_cgns_name_attr(file, "HDF5 MotherNode");
    add_cgns_label_attr(file, "Root Node of HDF5 File");
    add_cgns_type_attr(file, "MT");

    const std::string formatValue = "IEEE_LITTLE_32";
    std::vector<int8_t> formatData(formatValue.begin(), formatValue.end());
    hsize_t formatDims[1] = {static_cast<hsize_t>(formatData.size())};
    hid_t formatSpace = H5Screate_simple(1, formatDims, nullptr);
    hid_t formatDset = H5Dcreate2(file, " format", H5T_NATIVE_INT8, formatSpace,
                                  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Dwrite(formatDset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, formatData.data()),
                 "write root format dataset");
    H5Dclose(formatDset);
    H5Sclose(formatSpace);

    const std::string versionValue = "HDF5 Version 1.8.17";
    std::vector<int8_t> versionData(versionValue.begin(), versionValue.end());
    versionData.resize(kCGNSLongStringAttrLen, 0);
    hsize_t versionDims[1] = {static_cast<hsize_t>(versionData.size())};
    hid_t versionSpace = H5Screate_simple(1, versionDims, nullptr);
    hid_t versionDset = H5Dcreate2(file, " hdf5version", H5T_NATIVE_INT8, versionSpace,
                                   H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Dwrite(versionDset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, versionData.data()),
                 "write root hdf5version dataset");
    H5Dclose(versionDset);
    H5Sclose(versionSpace);
}

bool is_link_group(hid_t group) {
    return read_string_attr(group, "type") == "LK";
}

void write_array(hid_t loc, const std::string& name, const Array& array, const std::string& cgnsType) {
    if (cgnsType == "C1") {
        std::string str = array.extractString();
        std::vector<int8_t> buffer(str.begin(), str.end());
        hsize_t oneDim = static_cast<hsize_t>(buffer.size());
        hid_t strSpace = H5Screate_simple(1, &oneDim, nullptr);
        hid_t dset = H5Dcreate2(loc, name.c_str(), H5T_NATIVE_INT8, strSpace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        check_status(H5Dwrite(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()), "write string");
        H5Dclose(dset);
        H5Sclose(strSpace);
        return;
    }

    hid_t space = make_dataspace(array.shape());
    const hid_t dtype = hdfTypeFromCgnsType(cgnsType);
    hid_t dset = H5Dcreate2(loc, name.c_str(), dtype, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Dwrite(dset, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "write numeric");
    H5Dclose(dset);
    H5Sclose(space);
}

template <typename T>
Array read_numeric_array(hid_t dset, const std::vector<size_t>& shape) {
    const char order = 'C';
    Array array = arrayfactory::empty<T>(shape, order);
    check_status(
        H5Dread(dset, hdfTypeFromCgnsType(cgnsTypeFromArray(array)), H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()),
        "read numeric dataset");
    return array;
}

void write_node_rec(
    hid_t file,
    hid_t gcpl,
    const std::shared_ptr<Node>& node,
    const std::string& path,
    const std::string& cgnsTreeRootName = "") {

    std::string groupPath = path + "/" + node->name();
    hid_t group = H5Gcreate2(file, groupPath.c_str(), H5P_DEFAULT, gcpl, H5P_DEFAULT);
    add_cgns_name_attr(group, node->name());
    add_cgns_label_attr(group, node->type());

    if (node->hasLinkTarget()) {
        if (!node->noData()) {
            H5Gclose(group);
            throw std::runtime_error("node with link must not carry array data: " + node->path());
        }
        if (!node->children().empty()) {
            H5Gclose(group);
            throw std::runtime_error("node with link must not have children: " + node->path());
        }

        add_cgns_type_attr(group, "LK");
        const std::string targetPath = persisted_link_target_path(node, cgnsTreeRootName);

        write_int8_string_dataset(file, groupPath + "/ file", node->linkTargetFile());
        write_int8_string_dataset(file, groupPath + "/ path", targetPath);

        const std::string linkDatasetPath = groupPath + "/ link";
        if (node->linkTargetFile().empty() || node->linkTargetFile() == ".") {
            check_status(
                H5Lcreate_soft(targetPath.c_str(), file, linkDatasetPath.c_str(), H5P_DEFAULT, H5P_DEFAULT),
                "create soft link at " + linkDatasetPath);
        } else {
            check_status(
                H5Lcreate_external(node->linkTargetFile().c_str(), targetPath.c_str(), file,
                                   linkDatasetPath.c_str(), H5P_DEFAULT, H5P_DEFAULT),
                "create external link at " + linkDatasetPath);
        }

        H5Gclose(group);
        return;
    }

    add_flags_attr(group);

    std::string nodeCgnsDataType = "MT";
    if (!node->noData()) {
        auto array = std::dynamic_pointer_cast<Array>(node->dataPtr());
        if (!array) {
            throw std::runtime_error("Expected Array");
        }
        nodeCgnsDataType = cgnsTypeFromArray(*array);
        write_array(file, groupPath + "/ data", *array, nodeCgnsDataType);
    }
    add_cgns_type_attr(group, nodeCgnsDataType);

    for (auto& child : node->children()) {
        write_node_rec(file, gcpl, child, groupPath, cgnsTreeRootName);
    }

    H5Gclose(group);
}

void write_cgns_library_version(hid_t file, hid_t gcpl, const float& cgnsVersion) {
    std::string groupPath = "/CGNSLibraryVersion";
    hid_t group = H5Gcreate2(file, groupPath.c_str(), H5P_DEFAULT, gcpl, H5P_DEFAULT);
    add_cgns_name_attr(group, "CGNSLibraryVersion");
    add_cgns_label_attr(group, "CGNSLibraryVersion_t");

    add_flags_attr(group);
    add_cgns_type_attr(group, "R4");
    write_array(file, groupPath + "/ data", Array(cgnsVersion), "R4");
    H5Gclose(group);
}

Array readArrayFromDataset(hid_t dset, const std::vector<size_t>& shape, const std::string& cgnsType) {
    if (cgnsType == "C1") {
        hid_t space = H5Dget_space(dset);
        std::vector<hsize_t> dims(1, 0);
        H5Sget_simple_extent_dims(space, dims.data(), nullptr);
        std::vector<int8_t> buffer(static_cast<size_t>(dims[0]));
        check_status(H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()), "read string dataset");
        H5Sclose(space);
        std::string str(buffer.begin(), buffer.end());
        while (!str.empty() && str.back() == '\0') {
            str.pop_back();
        }
        return Array(str);
    }

    if (cgnsType == "I1") {
        Array array = arrayfactory::empty<int8_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read I1");
        return array;
    }
    if (cgnsType == "I2") {
        Array array = arrayfactory::empty<int16_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_INT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read I2");
        return array;
    }
    if (cgnsType == "I4") {
        Array array = arrayfactory::empty<int32_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read I4");
        return array;
    }
    if (cgnsType == "I8") {
        Array array = arrayfactory::empty<int64_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_INT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read I8");
        return array;
    }
    if (cgnsType == "U1") {
        Array array = arrayfactory::empty<uint8_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_UINT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read U1");
        return array;
    }
    if (cgnsType == "U2") {
        Array array = arrayfactory::empty<uint16_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_UINT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read U2");
        return array;
    }
    if (cgnsType == "U4") {
        Array array = arrayfactory::empty<uint32_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read U4");
        return array;
    }
    if (cgnsType == "U8") {
        Array array = arrayfactory::empty<uint64_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_UINT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read U8");
        return array;
    }
    if (cgnsType == "R4") {
        Array array = arrayfactory::empty<float>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read R4");
        return array;
    }
    if (cgnsType == "R8") {
        Array array = arrayfactory::empty<double>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read R8");
        return array;
    }
    if (cgnsType == "X1") {
        Array array = arrayfactory::empty<int8_t>(shape, 'C');
        check_status(H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()), "read X1");
        return array;
    }

    throw std::runtime_error("Unsupported type in read: " + cgnsType);
}

std::shared_ptr<Node> read_node_rec(hid_t file, const std::string& path) {
    hid_t group = H5Gopen2(file, path.c_str(), H5P_DEFAULT);
    if (group < 0) {
        throw std::runtime_error("Failed to open group: " + path);
    }
    std::string name = read_string_attr(group, "name");
    std::string label = read_string_attr(group, "label");
    if (label.empty()) {
        label = "DataArray_t";
    }

    auto node = std::make_shared<Node>(name, label);
    if (is_link_group(group)) {
        std::string targetFile = read_int8_string_dataset(file, path + "/ file");
        std::string targetPath = read_int8_string_dataset(file, path + "/ path");
        if (targetPath.empty()) {
            H5Gclose(group);
            throw std::runtime_error("Link node has empty target path at " + path);
        }
        node->setLinkTarget(targetFile, targetPath);
        H5Gclose(group);
        return node;
    }

    std::string dataPath = path + "/ data";
    if (H5Lexists(file, dataPath.c_str(), H5P_DEFAULT)) {
        hid_t dset = H5Dopen2(file, dataPath.c_str(), H5P_DEFAULT);
        hid_t space = H5Dget_space(dset);
        int ndims = H5Sget_simple_extent_ndims(space);
        std::vector<hsize_t> dims(ndims == 0 ? 0 : static_cast<size_t>(ndims));
        if (ndims > 0) {
            H5Sget_simple_extent_dims(space, dims.data(), nullptr);
        }
        std::vector<size_t> shape(dims.begin(), dims.end());

        std::string cgnsType = read_string_attr(group, "type");
        if (cgnsType.empty()) {
            cgnsType = "MT";
        }

        if (cgnsType != "MT") {
            node->setData(readArrayFromDataset(dset, shape, cgnsType));
        }

        H5Sclose(space);
        H5Dclose(dset);
    }

    hsize_t nObjs;
    H5Gget_num_objs(group, &nObjs);
    for (hsize_t i = 0; i < nObjs; ++i) {
        if (H5Gget_objtype_by_idx(group, i) != H5G_GROUP) {
            continue;
        }
        char nameBuf[256];
        H5Gget_objname_by_idx(group, i, nameBuf, sizeof(nameBuf));
        std::string childName(nameBuf);
        if (childName == " data" || childName == " file" || childName == " path" || childName == " link") {
            continue;
        }
        std::string childPath = path + "/" + childName;
        auto child = read_node_rec(file, childPath);
        child->attachTo(node);
    }

    H5Gclose(group);
    return node;
}

} // namespace

void write_node(const std::string& filename, std::shared_ptr<Node> root, const float& cgnsVersion) {
    hid_t fcpl = make_cgns_file_creation_plist();
    hid_t file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, fcpl, H5P_DEFAULT);
    H5Pclose(fcpl);
    hid_t gcpl = make_cgns_group_creation_plist();
    write_cgns_file_metadata(file);
    write_cgns_library_version(file, gcpl, resolved_cgns_version(root, cgnsVersion));
    if (is_cgns_tree_root(root)) {
        for (const auto& child : root->children()) {
            if (is_cgns_library_version_node(child)) {
                continue;
            }
            write_node_rec(file, gcpl, child, "", root->name());
        }
    } else {
        write_node_rec(file, gcpl, root, "");
    }
    H5Pclose(gcpl);
    H5Fclose(file);
}

std::shared_ptr<Node> read(const std::string& filename) {
    hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    auto root = read_node_rec(file, "/");
    H5Fclose(file);
    return root;
}

} // namespace io

#endif // ENABLE_HDF5_IO
