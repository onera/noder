#ifdef ENABLE_HDF5_IO
#ifndef IO_NUMPY_HPP
#define IO_NUMPY_HPP

#include <string>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <hdf5.h>

#include "array/array_numpy_bridge.hpp"
#include "array/factory/matrices.hpp"

namespace py = pybind11;

namespace io {

/**
 * @brief Persist a NumPy array into an HDF5 dataset.
 * @param array Data to write.
 * @param filename Output file path.
 * @param dataset_name Dataset key.
 */
inline hid_t hdf5_dtype_from_array(const Array& array) {
    if (array.hasDataOfType<int8_t>()) return H5T_NATIVE_INT8;
    if (array.hasDataOfType<int16_t>()) return H5T_NATIVE_INT16;
    if (array.hasDataOfType<int32_t>()) return H5T_NATIVE_INT32;
    if (array.hasDataOfType<int64_t>()) return H5T_NATIVE_INT64;
    if (array.hasDataOfType<uint8_t>()) return H5T_NATIVE_UINT8;
    if (array.hasDataOfType<uint16_t>()) return H5T_NATIVE_UINT16;
    if (array.hasDataOfType<uint32_t>()) return H5T_NATIVE_UINT32;
    if (array.hasDataOfType<uint64_t>()) return H5T_NATIVE_UINT64;
    if (array.hasDataOfType<float>()) return H5T_NATIVE_FLOAT;
    if (array.hasDataOfType<double>()) return H5T_NATIVE_DOUBLE;
    if (array.hasDataOfType<bool>()) return H5T_NATIVE_UINT8;
    throw std::runtime_error("Unsupported array dtype for HDF5");
}

inline void write_numpy(const py::array& array, const std::string& filename, const std::string& dataset_name) {
    hid_t fileId = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (fileId < 0) {
        throw std::runtime_error("Could not create HDF5 file");
    }

    Array noderArray = arraybridge::arrayFromPyArray(array);

    if (noderArray.hasString()) {
        const std::string dataString = noderArray.extractString();
        std::vector<int8_t> buffer(dataString.begin(), dataString.end());
        hsize_t dims[1] = {static_cast<hsize_t>(buffer.size())};
        hid_t dataspaceId = H5Screate_simple(1, dims, nullptr);
        hid_t datasetId = H5Dcreate2(fileId, dataset_name.c_str(), H5T_NATIVE_INT8, dataspaceId,
                                     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (H5Dwrite(datasetId, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()) < 0) {
            throw std::runtime_error("Failed to write string data to HDF5 dataset");
        }
        H5Dclose(datasetId);
        H5Sclose(dataspaceId);
        H5Fclose(fileId);
        return;
    }

    const std::vector<size_t> shape = noderArray.shape();
    hid_t dataspaceId = shape.empty()
        ? H5Screate(H5S_SCALAR)
        : H5Screate_simple(static_cast<int>(shape.size()),
                           std::vector<hsize_t>(shape.begin(), shape.end()).data(),
                           nullptr);

    hid_t datatype = hdf5_dtype_from_array(noderArray);
    hid_t datasetId = H5Dcreate2(fileId, dataset_name.c_str(), datatype, dataspaceId,
                                 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    if (H5Dwrite(datasetId, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, noderArray.rawData()) < 0) {
        throw std::runtime_error("Failed to write data to HDF5 dataset");
    }

    H5Dclose(datasetId);
    H5Sclose(dataspaceId);
    H5Fclose(fileId);
}
/**
 * @brief Load a NumPy array from an HDF5 dataset.
 * @param filename Input file path.
 * @param dataset_name Dataset key.
 * @param order Memory order of returned array ("C" or "F").
 * @return Loaded NumPy array.
 */
inline py::array read_numpy(const std::string& filename, const std::string& dataset_name, const std::string& order = "F") {
    hid_t fileId = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fileId < 0) {
        throw std::runtime_error("Could not open HDF5 file");
    }

    hid_t datasetId = H5Dopen2(fileId, dataset_name.c_str(), H5P_DEFAULT);
    hid_t datatype = H5Dget_type(datasetId);
    hid_t dataspaceId = H5Dget_space(datasetId);

    int ndims = H5Sget_simple_extent_ndims(dataspaceId);
    std::vector<hsize_t> dims(ndims > 0 ? static_cast<size_t>(ndims) : 0);
    if (ndims > 0) {
        H5Sget_simple_extent_dims(dataspaceId, dims.data(), nullptr);
    }
    std::vector<size_t> shape(dims.begin(), dims.end());

    const char layout = order == "F" ? 'F' : 'C';
    if (H5Tequal(datatype, H5T_NATIVE_INT8)) {
        Array array = arrayfactory::empty<int8_t>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_INT16)) {
        Array array = arrayfactory::empty<int16_t>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_INT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_INT32)) {
        Array array = arrayfactory::empty<int32_t>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_INT64)) {
        Array array = arrayfactory::empty<int64_t>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_INT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_UINT8)) {
        Array array = arrayfactory::empty<uint8_t>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_UINT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_UINT16)) {
        Array array = arrayfactory::empty<uint16_t>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_UINT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_UINT32)) {
        Array array = arrayfactory::empty<uint32_t>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_UINT64)) {
        Array array = arrayfactory::empty<uint64_t>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_UINT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_FLOAT)) {
        Array array = arrayfactory::empty<float>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else if (H5Tequal(datatype, H5T_NATIVE_DOUBLE)) {
        Array array = arrayfactory::empty<double>(shape, layout);
        H5Dread(datasetId, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData());
        H5Tclose(datatype); H5Sclose(dataspaceId); H5Dclose(datasetId); H5Fclose(fileId);
        return arraybridge::toPyArray(array);
    } else {
        throw std::runtime_error("Unsupported datatype in HDF5 file");
    }
}

} // namespace io

#endif // IO_NUMPY_HPP
#endif // ENABLE_HDF5_IO
