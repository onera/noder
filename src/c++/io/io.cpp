#ifdef ENABLE_HDF5_IO

#include "io/io.hpp"
#include "array/array.hpp"
#include <hdf5.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

namespace io {

namespace {

constexpr size_t kCGNSLongStringAttrLen = 33;
constexpr size_t kCGNSTypeAttrLen = 3;
constexpr unsigned kCGNSLinkCreationOrderFlags = H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED;

} // namespace

void check_status(herr_t status, const std::string& msg) {
    if (status < 0) throw std::runtime_error("HDF5 error: " + msg);
}

hid_t make_cgns_group_creation_plist() {
    hid_t gcpl = H5Pcreate(H5P_GROUP_CREATE);
    if (gcpl < 0) throw std::runtime_error("HDF5 error: cannot create group creation property list");
    check_status(H5Pset_link_creation_order(gcpl, kCGNSLinkCreationOrderFlags),
                 "set group link creation order");
    return gcpl;
}

hid_t make_cgns_file_creation_plist() {
    hid_t fcpl = H5Pcreate(H5P_FILE_CREATE);
    if (fcpl < 0) throw std::runtime_error("HDF5 error: cannot create file creation property list");
    check_status(H5Pset_link_creation_order(fcpl, kCGNSLinkCreationOrderFlags),
                 "set root group link creation order");
    return fcpl;
}


void write_numpy(const py::array& array, const std::string& filename, const std::string& dataset_name) {
    hid_t file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file_id < 0) throw std::runtime_error("Could not create HDF5 file");

    py::buffer_info info = array.request();
    std::vector<hsize_t> dims(info.shape.begin(), info.shape.end());

    if (dims.empty()) dims.push_back(1);

    hid_t dataspace_id = H5Screate_simple(dims.size(), dims.data(), NULL);

    hid_t datatype;
    auto dtype = array.dtype();
    char kind = dtype.kind();

    if (dtype.is(py::dtype::of<int8_t>())) datatype = H5T_NATIVE_INT8;
    else if (dtype.is(py::dtype::of<int16_t>())) datatype = H5T_NATIVE_INT16;
    else if (dtype.is(py::dtype::of<int32_t>())) datatype = H5T_NATIVE_INT32;
    else if (dtype.is(py::dtype::of<int64_t>())) datatype = H5T_NATIVE_INT64;
    else if (dtype.is(py::dtype::of<uint8_t>())) datatype = H5T_NATIVE_UINT8;
    else if (dtype.is(py::dtype::of<uint16_t>())) datatype = H5T_NATIVE_UINT16;
    else if (dtype.is(py::dtype::of<uint32_t>())) datatype = H5T_NATIVE_UINT32;
    else if (dtype.is(py::dtype::of<uint64_t>())) datatype = H5T_NATIVE_UINT64;
    else if (dtype.is(py::dtype::of<float>())) datatype = H5T_NATIVE_FLOAT;
    else if (dtype.is(py::dtype::of<double>())) datatype = H5T_NATIVE_DOUBLE;
    else if (kind == 'U' || kind == 'S') {
        Array noderArray(array);
        std::string data_str = noderArray.extractString();
        std::vector<int8_t> data_int(data_str.begin(), data_str.end());

        dims = { data_int.size() };
        dataspace_id = H5Screate_simple(1, dims.data(), NULL);
        datatype = H5T_NATIVE_INT8;

        hid_t dataset_id = H5Dcreate2(file_id, dataset_name.c_str(), datatype, dataspace_id,
                                      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data_int.data()) < 0)
            throw std::runtime_error("Failed to write string data to HDF5 dataset");

        H5Dclose(dataset_id);
        H5Sclose(dataspace_id);
        H5Fclose(file_id);
        return;
    }
    else throw std::runtime_error("Unsupported numpy data type");

    hid_t dataset_id = H5Dcreate2(file_id, dataset_name.c_str(), datatype, dataspace_id, 
                                  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    if (H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, info.ptr) < 0)
        throw std::runtime_error("Failed to write data to HDF5 dataset");

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
    H5Fclose(file_id);
}

py::array read_numpy(const std::string& filename, const std::string& dataset_name, const std::string& order) {
    hid_t file_id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0) throw std::runtime_error("Could not open HDF5 file");

    hid_t dataset_id = H5Dopen2(file_id, dataset_name.c_str(), H5P_DEFAULT);

    hid_t datatype = H5Dget_type(dataset_id);
    hid_t dataspace_id = H5Dget_space(dataset_id);

    int ndims = H5Sget_simple_extent_ndims(dataspace_id);
    std::vector<hsize_t> dims(ndims);
    H5Sget_simple_extent_dims(dataspace_id, dims.data(), NULL);

    py::dtype py_dtype;
    if (H5Tequal(datatype, H5T_NATIVE_INT8)) py_dtype = py::dtype::of<int8_t>();
    else if (H5Tequal(datatype, H5T_NATIVE_INT16)) py_dtype = py::dtype::of<int16_t>();
    else if (H5Tequal(datatype, H5T_NATIVE_INT32)) py_dtype = py::dtype::of<int32_t>();
    else if (H5Tequal(datatype, H5T_NATIVE_INT64)) py_dtype = py::dtype::of<int64_t>();
    else if (H5Tequal(datatype, H5T_NATIVE_UINT8)) py_dtype = py::dtype::of<uint8_t>();
    else if (H5Tequal(datatype, H5T_NATIVE_UINT16)) py_dtype = py::dtype::of<uint16_t>();
    else if (H5Tequal(datatype, H5T_NATIVE_UINT32)) py_dtype = py::dtype::of<uint32_t>();
    else if (H5Tequal(datatype, H5T_NATIVE_UINT64)) py_dtype = py::dtype::of<uint64_t>();
    else if (H5Tequal(datatype, H5T_NATIVE_FLOAT)) py_dtype = py::dtype::of<float>();
    else if (H5Tequal(datatype, H5T_NATIVE_DOUBLE)) py_dtype = py::dtype::of<double>();
    else throw std::runtime_error("Unsupported datatype in HDF5 file");

    py::array array;

    std::vector<ssize_t> strides(dims.size());

    ssize_t stride = py_dtype.itemsize();
    if (order == "F") {
        for (size_t i = 0; i < dims.size(); ++i) {
            strides[i] = stride;
            stride *= static_cast<ssize_t>(dims[i]);
        }
    } else {  // C order
        for (ssize_t i = dims.size() - 1; i >= 0; --i) {
            strides[i] = stride;
            stride *= static_cast<ssize_t>(dims[i]);
        }
    }
    array = py::array(py_dtype, dims, strides);
    


    py::buffer_info info = array.request();
    if (H5Dread(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, info.ptr) < 0)
        throw std::runtime_error("Failed to read data from HDF5 dataset");

    H5Tclose(datatype);
    H5Sclose(dataspace_id);
    H5Dclose(dataset_id);
    H5Fclose(file_id);

    return array;
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
    H5Aclose(attr); H5Sclose(space); H5Tclose(type);
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
    H5Aclose(attr); H5Sclose(space);
}

void add_int_attr(hid_t id, const std::string& key, int32_t value) {
    hid_t space = H5Screate(H5S_SCALAR);
    hid_t attr = H5Acreate2(id, key.c_str(), H5T_NATIVE_INT32, space, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Awrite(attr, H5T_NATIVE_INT32, &value), "H5Awrite int");
    H5Aclose(attr); H5Sclose(space);
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

    H5Tclose(type); H5Aclose(attr);
    return result;
}

int32_t read_int_attr(hid_t id, const std::string& key, int32_t fallback = 0) {
    if (!H5Aexists(id, key.c_str())) return fallback;
    int32_t value;
    hid_t attr = H5Aopen(id, key.c_str(), H5P_DEFAULT);
    check_status(H5Aread(attr, H5T_NATIVE_INT32, &value), "H5Aread int");
    H5Aclose(attr);
    return value;
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

    const std::string format_value = "IEEE_LITTLE_32";
    std::vector<int8_t> format_data(format_value.begin(), format_value.end());
    hsize_t format_dims[1] = {static_cast<hsize_t>(format_data.size())};
    hid_t format_space = H5Screate_simple(1, format_dims, nullptr);
    hid_t format_dset = H5Dcreate2(file, " format", H5T_NATIVE_INT8, format_space,
                                   H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Dwrite(format_dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, format_data.data()),
                 "write root format dataset");
    H5Dclose(format_dset);
    H5Sclose(format_space);

    const std::string version_value = "HDF5 Version 1.8.17";
    std::vector<int8_t> version_data(version_value.begin(), version_value.end());
    version_data.resize(kCGNSLongStringAttrLen, 0);
    hsize_t version_dims[1] = {static_cast<hsize_t>(version_data.size())};
    hid_t version_space = H5Screate_simple(1, version_dims, nullptr);
    hid_t version_dset = H5Dcreate2(file, " hdf5version", H5T_NATIVE_INT8, version_space,
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Dwrite(version_dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, version_data.data()),
                 "write root hdf5version dataset");
    H5Dclose(version_dset);
    H5Sclose(version_space);
}


bool is_link_group(hid_t group) {
    return read_string_attr(group, "type") == "LK";
}


void write_array(hid_t loc, const std::string& name, const py::array& arr, const std::string& cgns_type) {
    py::buffer_info info = arr.request();
    std::vector<hsize_t> dims(info.shape.begin(), info.shape.end());
    hid_t space = H5Screate_simple(static_cast<int>(dims.size()), dims.data(), nullptr);

    hid_t dtype = -1;

    if (cgns_type == "I4") dtype = H5T_NATIVE_INT32;
    else if (cgns_type == "I8") dtype = H5T_NATIVE_INT64;
    else if (cgns_type == "U1") dtype = H5T_NATIVE_UINT8;
    else if (cgns_type == "U2") dtype = H5T_NATIVE_UINT16;
    else if (cgns_type == "U4") dtype = H5T_NATIVE_UINT32;
    else if (cgns_type == "U8") dtype = H5T_NATIVE_UINT64;
    else if (cgns_type == "R4") dtype = H5T_NATIVE_FLOAT;
    else if (cgns_type == "R8") dtype = H5T_NATIVE_DOUBLE;
    else if (cgns_type == "X1") dtype = H5T_NATIVE_INT8;  // fallback for bool
    else if (cgns_type == "C1") {
        std::string str = Array(arr).extractString();
        std::vector<int8_t> buffer(str.begin(), str.end());
        hsize_t one_dim = buffer.size();
        hid_t str_space = H5Screate_simple(1, &one_dim, nullptr);
        dtype = H5T_NATIVE_INT8;
        hid_t dset = H5Dcreate2(loc, name.c_str(), dtype, str_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        check_status(H5Dwrite(dset, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()), "write string");
        H5Dclose(dset);
        H5Sclose(str_space);
        return;
    } else {
        throw std::runtime_error("Unsupported CGNS type: " + cgns_type);
    }

    hid_t dset = H5Dcreate2(loc, name.c_str(), dtype, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Dwrite(dset, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, info.ptr), "write numeric");
    H5Dclose(dset);
    H5Sclose(space);
}

void write_node_rec(hid_t file, hid_t gcpl, std::shared_ptr<Node> node, const std::string& path) {
    std::string group_path = path + "/" + node->name();
    hid_t group = H5Gcreate2(file, group_path.c_str(), H5P_DEFAULT, gcpl, H5P_DEFAULT);
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
        write_int8_string_dataset(file, group_path + "/ file", node->linkTargetFile());
        write_int8_string_dataset(file, group_path + "/ path", node->linkTargetPath());

        const std::string link_dataset_path = group_path + "/ link";
        if (node->linkTargetFile().empty() || node->linkTargetFile() == ".") {
            check_status(
                H5Lcreate_soft(node->linkTargetPath().c_str(), file, link_dataset_path.c_str(),
                               H5P_DEFAULT, H5P_DEFAULT),
                "create soft link at " + link_dataset_path);
        } else {
            check_status(
                H5Lcreate_external(node->linkTargetFile().c_str(), node->linkTargetPath().c_str(), file,
                                   link_dataset_path.c_str(), H5P_DEFAULT, H5P_DEFAULT),
                "create external link at " + link_dataset_path);
        }

        H5Gclose(group);
        return;
    }

    add_flags_attr(group);

    std::string node_cgns_data_type = "MT";
    if (!node->noData()) {
        auto array = std::dynamic_pointer_cast<Array>(node->dataPtr());
        if (!array) throw std::runtime_error("Expected Array");
        py::array pyarr = array->getPyArray();

        auto dtype = pyarr.dtype();
        char kind = dtype.kind();
        std::string cgns_type;

        if (dtype.is(py::dtype::of<int32_t>())) {
            cgns_type = "I4";
        } else if (dtype.is(py::dtype::of<int64_t>())) {
            cgns_type = "I8";
        } else if (dtype.is(py::dtype::of<uint8_t>())) {
            cgns_type = "U1";
        } else if (dtype.is(py::dtype::of<uint16_t>())) {
            cgns_type = "U2";
        } else if (dtype.is(py::dtype::of<uint32_t>())) {
            cgns_type = "U4";
        } else if (dtype.is(py::dtype::of<uint64_t>())) {
            cgns_type = "U8";
        } else if (dtype.is(py::dtype::of<float>())) {
            cgns_type = "R4";
        } else if (dtype.is(py::dtype::of<double>())) {
            cgns_type = "R8";
        } else if (dtype.is(py::dtype::of<bool>()) || dtype.is(py::dtype::of<int8_t>())) {
            cgns_type = "X1";
        } else if (kind == 'U' || kind == 'S') {
            cgns_type = "C1";
        } else {
            throw std::runtime_error("Unsupported array type at node "+node->path());
        }

        node_cgns_data_type = cgns_type;
        write_array(file, group_path + "/ data", pyarr, cgns_type);
    }
    add_cgns_type_attr(group, node_cgns_data_type);

    for (auto& child : node->children()) {
        write_node_rec(file, gcpl, child, group_path);
    }

    H5Gclose(group);
}

void write_cgns_library_version(hid_t file, hid_t gcpl, const float& cgnsVersion) {
    std::string group_path = "/CGNSLibraryVersion";
    hid_t group = H5Gcreate2(file, group_path.c_str(), H5P_DEFAULT, gcpl, H5P_DEFAULT);
    add_cgns_name_attr(group, "CGNSLibraryVersion");
    add_cgns_label_attr(group, "CGNSLibraryVersion_t");

    add_flags_attr(group);
    add_cgns_type_attr(group, "R4");
    write_array(file, group_path + "/ data",
                py::array_t<float>({1}, {sizeof(float)}, &cgnsVersion, py::cast(nullptr)),
                "R4");
    H5Gclose(group);
}

void write_node(const std::string& filename, std::shared_ptr<Node> root, const float& cgnsVersion) {
    hid_t fcpl = make_cgns_file_creation_plist();
    hid_t file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, fcpl, H5P_DEFAULT);
    H5Pclose(fcpl);
    hid_t gcpl = make_cgns_group_creation_plist();
    write_cgns_file_metadata(file);
    write_cgns_library_version(file, gcpl, cgnsVersion);
    write_node_rec(file, gcpl, root, "");
    H5Pclose(gcpl);
    H5Fclose(file);
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
        std::string target_file = read_int8_string_dataset(file, path + "/ file");
        std::string target_path = read_int8_string_dataset(file, path + "/ path");
        if (target_path.empty()) {
            H5Gclose(group);
            throw std::runtime_error("Link node has empty target path at " + path);
        }
        node->setLinkTarget(target_file, target_path);
        H5Gclose(group);
        return node;
    }

   
    std::string data_path = path + "/ data";
    if (H5Lexists(file, data_path.c_str(), H5P_DEFAULT)) {
        hid_t dset = H5Dopen2(file, data_path.c_str(), H5P_DEFAULT);
        hid_t space = H5Dget_space(dset);
        int ndims = H5Sget_simple_extent_ndims(space);
        std::vector<hsize_t> dims(ndims);
        H5Sget_simple_extent_dims(space, dims.data(), nullptr);
        hid_t dtype = H5Dget_type(dset);

        std::string cgns_type = read_string_attr(group, "type");
        if (cgns_type.empty()) {
            cgns_type = read_string_attr(dset, "type");
        }
        py::array arr;

        if (cgns_type == "C1") {
            std::vector<int8_t> buffer(dims[0]);
            H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data());
            std::string str(buffer.begin(), buffer.end());
            while (!str.empty() && str.back() == '\0') {
                str.pop_back();
            }
            node->setData(Array(str));
        } else if (cgns_type == "I1") {
            arr = py::array_t<int8_t>(dims);
            H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "I2") {
            arr = py::array_t<int16_t>(dims);
            H5Dread(dset, H5T_NATIVE_INT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "I4") {
            arr = py::array_t<int32_t>(dims);
            H5Dread(dset, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "I8") {
            arr = py::array_t<int64_t>(dims);
            H5Dread(dset, H5T_NATIVE_INT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "U1") {
            arr = py::array_t<uint8_t>(dims);
            H5Dread(dset, H5T_NATIVE_UINT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "U2") {
            arr = py::array_t<uint16_t>(dims);
            H5Dread(dset, H5T_NATIVE_UINT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "U4") {
            arr = py::array_t<uint32_t>(dims);
            H5Dread(dset, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "U8") {
            arr = py::array_t<uint64_t>(dims);
            H5Dread(dset, H5T_NATIVE_UINT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "R4") {
            arr = py::array_t<float>(dims);
            H5Dread(dset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "R8") {
            arr = py::array_t<double>(dims);
            H5Dread(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else if (cgns_type == "X1") {
            arr = py::array_t<int8_t>(dims);
            H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, arr.mutable_data());
            node->setData(Array(arr));
        } else {
            throw std::runtime_error("Unsupported type in read: " + cgns_type);
        }

        H5Tclose(dtype);
        H5Sclose(space);
        H5Dclose(dset);
    }

    hsize_t n_objs;
    H5Gget_num_objs(group, &n_objs);
    for (hsize_t i = 0; i < n_objs; ++i) {
        if (H5Gget_objtype_by_idx(group, i) != H5G_GROUP) {
            continue;
        }
        char name_buf[256];
        H5Gget_objname_by_idx(group, i, name_buf, sizeof(name_buf));
        std::string child_name(name_buf);
        if (child_name == " data" || child_name == " file" || child_name == " path" || child_name == " link") {
            continue;
        }
        std::string child_path = path + "/" + child_name;
        auto child = read_node_rec(file, child_path);
        child->attachTo(node);
    }

    H5Gclose(group);
    return node;
}

std::shared_ptr<Node> read(const std::string& filename) {
    hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    auto root = read_node_rec(file, "/");
    H5Fclose(file);
    return root;
}

} // namespace io

#endif // ENABLE_HDF5_IO
