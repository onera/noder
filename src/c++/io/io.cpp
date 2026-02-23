#ifdef ENABLE_HDF5_IO

#include "io/io.hpp"
#include "array/array.hpp"
#include <hdf5.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

namespace io {

void check_status(herr_t status, const std::string& msg) {
    if (status < 0) throw std::runtime_error("HDF5 error: " + msg);
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


void add_string_attr(hid_t id, const std::string& key, const std::string& value) {
    hid_t space = H5Screate(H5S_SCALAR);
    hid_t type = H5Tcopy(H5T_C_S1);
    H5Tset_size(type, H5T_VARIABLE);
    const char* str = value.c_str();
    hid_t attr = H5Acreate2(id, key.c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Awrite(attr, type, &str), "H5Awrite string");
    H5Aclose(attr); H5Sclose(space); H5Tclose(type);
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
    char* value;
    check_status(H5Aread(attr, type, &value), "H5Aread string");
    std::string result(value);
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


void write_array_with_attrs(hid_t loc, const std::string& name, const py::array& arr, const std::string& cgns_type) {
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
        add_string_attr(dset, "type", "C1");
        H5Dclose(dset);
        H5Sclose(str_space);
        return;
    } else {
        throw std::runtime_error("Unsupported CGNS type: " + cgns_type);
    }

    hid_t dset = H5Dcreate2(loc, name.c_str(), dtype, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    check_status(H5Dwrite(dset, dtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, info.ptr), "write numeric");
    add_string_attr(dset, "type", cgns_type);
    H5Dclose(dset);
    H5Sclose(space);
}

void write_node_rec(hid_t file, std::shared_ptr<Node> node, const std::string& path) {
    std::string group_path = path + "/" + node->name();
    hid_t group = H5Gcreate2(file, group_path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    add_string_attr(group, "name", node->name());
    add_string_attr(group, "label", node->type());

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
        

        write_array_with_attrs(file, group_path + "/ data", pyarr, cgns_type);
    }

    for (auto& child : node->children()) {
        write_node_rec(file, child, group_path);
    }

    H5Gclose(group);
}

void write_node(const std::string& filename, std::shared_ptr<Node> root) {
    hid_t file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    write_node_rec(file, root, "");
    H5Fclose(file);
}

std::shared_ptr<Node> read_node_rec(hid_t file, const std::string& path) {
    hid_t group = H5Gopen2(file, path.c_str(), H5P_DEFAULT);
    if (group < 0) {
        throw std::runtime_error("Failed to open group: " + path);
    }
    std::string name = read_string_attr(group, "name");
    std::string label = read_string_attr(group, "label");

    auto node = std::make_shared<Node>(name, label);

   
    std::string data_path = path + "/ data";
    if (H5Lexists(file, data_path.c_str(), H5P_DEFAULT)) {
        hid_t dset = H5Dopen2(file, data_path.c_str(), H5P_DEFAULT);
        hid_t space = H5Dget_space(dset);
        int ndims = H5Sget_simple_extent_ndims(space);
        std::vector<hsize_t> dims(ndims);
        H5Sget_simple_extent_dims(space, dims.data(), nullptr);
        hid_t dtype = H5Dget_type(dset);

        std::string cgns_type = read_string_attr(dset, "type");
        py::array arr;

        if (cgns_type == "C1") {
            std::vector<int8_t> buffer(dims[0]);
            H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data());
            std::string str(buffer.begin(), buffer.end());
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
        char name_buf[256];
        H5Gget_objname_by_idx(group, i, name_buf, sizeof(name_buf));
        std::string child_name(name_buf);
        if (child_name == " data") continue;
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
