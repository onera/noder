#ifdef ENABLE_HDF5_IO
#ifndef TEST_IO_H
#define TEST_IO_H

# include <io/io.hpp>
# include <array/factory/vectors.hpp>
# include <node/node_factory.hpp>
# include <hdf5.h>
# include <sstream>
# include <vector>

using namespace std::string_literals;
using namespace io;
using namespace arrayfactory;

namespace test_io {

void test_write_nodes( std::string filename = "test.cgns") {
     auto a = newNode("a", "DataArray_t");
     Array arrA = uniformFromStep<int32_t>(0, 10);
     a->setData(arrA);

     auto b = newNode("b", "DataArray_t");
     Array arrB = uniformFromCount<float>(-1, 1, 5);
     b->setData(arrB);
     b->attachTo(a);

     auto c = newNode("c", "DataArray_t");
     Array arrC = "toto"s;
     c->setData(arrC);
     c->attachTo(a);

     auto d = newNode("d", "DataArray_t");
     d->attachTo(b);

     write_node(filename, a);
}

std::shared_ptr<Node> test_read( std::string tmp_filename = "test_read.cgns") {
     test_write_nodes(tmp_filename);
     auto node = read(tmp_filename);
     return node;
}

void test_write_link_nodes(std::string filename = "test_links.cgns") {
     auto root = newNode("root", "DataArray_t");
     auto target = newNode("target", "DataArray_t");
     target->setData(uniformFromStep<int32_t>(0, 4));
     target->attachTo(root);

     auto linkNode = newNode("target_link");
     linkNode->setLinkTarget(".", "/root/target");
     linkNode->attachTo(root);

     write_node(filename, root);
}

std::shared_ptr<Node> test_read_links(std::string tmp_filename = "test_read_links.cgns") {
     test_write_link_nodes(tmp_filename);
     return read(tmp_filename);
}

std::vector<std::string> list_root_links(const std::string& filename) {
     hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
     if (file < 0) {
         throw std::runtime_error("Could not open HDF5 file: " + filename);
     }

     std::vector<std::string> names;
     hsize_t index = 0;
     const auto callback = [](
         hid_t,
         const char* name,
         const H5L_info_t*,
         void* opData) -> herr_t {
             auto* rootNames = static_cast<std::vector<std::string>*>(opData);
             rootNames->emplace_back(name);
             return 0;
         };

     herr_t status = H5Literate(file, H5_INDEX_NAME, H5_ITER_NATIVE, &index, callback, &names);
     H5Fclose(file);

     if (status < 0) {
         throw std::runtime_error("Could not enumerate HDF5 root links in: " + filename);
     }

     return names;
 }

std::string runtime_hdf5_version() {
     unsigned major = 0;
     unsigned minor = 0;
     unsigned release = 0;
     if (H5get_libversion(&major, &minor, &release) < 0) {
         throw std::runtime_error("Could not get HDF5 library version");
     }

     std::ostringstream stream;
     stream << "HDF5 Version " << major << "." << minor << "." << release;
     return stream.str();
}

std::string read_root_hdf5_version(const std::string& filename) {
     hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
     if (file < 0) {
         throw std::runtime_error("Could not open HDF5 file: " + filename);
     }

     hid_t dset = H5Dopen2(file, " hdf5version", H5P_DEFAULT);
     if (dset < 0) {
         H5Fclose(file);
         throw std::runtime_error("Could not open root hdf5version dataset in: " + filename);
     }

     hid_t space = H5Dget_space(dset);
     hsize_t dims[1] = {0};
     H5Sget_simple_extent_dims(space, dims, nullptr);
     std::vector<int8_t> buffer(static_cast<size_t>(dims[0]));
     herr_t status = H5Dread(dset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data());

     H5Sclose(space);
     H5Dclose(dset);
     H5Fclose(file);

     if (status < 0) {
         throw std::runtime_error("Could not read root hdf5version dataset in: " + filename);
     }

     std::string value(buffer.begin(), buffer.end());
     while (!value.empty() && value.back() == '\0') {
         value.pop_back();
     }
     return value;
}

}

#endif // TEST_IO_H
#endif // ENABLE_HDF5_IO
