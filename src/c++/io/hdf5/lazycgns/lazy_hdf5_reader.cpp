#ifdef ENABLE_HDF5_IO

#include "io/hdf5/lazycgns/lazy_hdf5_reader.hpp"

#include "array/array.hpp"
#include "array/factory/matrices.hpp"
#include "cgns/base.hpp"
#include "cgns/tree.hpp"
#include "cgns/zone.hpp"
#include "io/hdf5/cgns/hdf5_compat.hpp"
#include "io/hdf5/cgns/cgns_name_limits.hpp"

#include <hdf5.h>

#include <algorithm>
#include <cctype>
#include <complex>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace io::hdf5::cgns {

namespace {

constexpr size_t kMaximumSize = std::numeric_limits<size_t>::max();

class hdf5_handle {
public:
    using close_function = herr_t (*)(hid_t);

    hdf5_handle(hid_t id, close_function close) : _id(id), _close(close) {}
    hdf5_handle(const hdf5_handle&) = delete;
    hdf5_handle& operator=(const hdf5_handle&) = delete;

    hdf5_handle(hdf5_handle&& other) noexcept : _id(other._id), _close(other._close) {
        other._id = -1;
        other._close = nullptr;
    }

    hdf5_handle& operator=(hdf5_handle&& other) noexcept {
        if (this != &other) {
            reset();
            _id = other._id;
            _close = other._close;
            other._id = -1;
            other._close = nullptr;
        }
        return *this;
    }

    ~hdf5_handle() {
        reset();
    }

    hid_t get() const {
        return _id;
    }

public:
    void reset() noexcept {
        if (_id >= 0 && _close != nullptr) {
            _close(_id);
        }
        _id = -1;
        _close = nullptr;
    }

private:
    hid_t _id;
    close_function _close;
};

void check_status(herr_t status, const std::string& message) {
    if (status < 0) {
        throw std::runtime_error("HDF5 error: " + message);
    }
}

std::string hdf5ChildPath(const std::string& parent, const std::string& child) {
    if (parent.empty() || parent == "/") {
        return "/" + child;
    }
    return parent + "/" + child;
}

std::string readStringAttribute(hid_t id, const std::string& key) {
    const htri_t exists = H5Aexists(id, key.c_str());
    if (exists < 0) {
        throw std::runtime_error("cannot inspect attribute '" + key + "'");
    }
    if (exists == 0) {
        return {};
    }

    hdf5_handle attribute(H5Aopen(id, key.c_str(), H5P_DEFAULT), H5Aclose);
    if (attribute.get() < 0) {
        throw std::runtime_error("cannot open attribute '" + key + "'");
    }

    hdf5_handle type(H5Aget_type(attribute.get()), H5Tclose);
    if (type.get() < 0) {
        throw std::runtime_error("cannot inspect attribute type '" + key + "'");
    }

    if (H5Tis_variable_str(type.get()) > 0) {
        char* value = nullptr;
        check_status(H5Aread(attribute.get(), type.get(), &value), "read attribute '" + key + "'");
        std::string result = value == nullptr ? std::string() : std::string(value);
        if (value != nullptr) {
            H5free_memory(value);
        }
        return result;
    }

    const size_t size = static_cast<size_t>(H5Tget_size(type.get()));
    std::vector<char> value(size + 1, '\0');
    check_status(H5Aread(attribute.get(), type.get(), value.data()), "read attribute '" + key + "'");
    return std::string(value.data());
}

std::string readInt8StringDataset(hid_t file, const std::string& datasetPath) {
    const htri_t exists = H5Lexists(file, datasetPath.c_str(), H5P_DEFAULT);
    if (exists < 0) {
        throw std::runtime_error("cannot inspect link dataset '" + datasetPath + "'");
    }
    if (exists == 0) {
        return {};
    }

    hdf5_handle dataset(H5Dopen2(file, datasetPath.c_str(), H5P_DEFAULT), H5Dclose);
    if (dataset.get() < 0) {
        throw std::runtime_error("cannot open link dataset '" + datasetPath + "'");
    }
    hdf5_handle space(H5Dget_space(dataset.get()), H5Sclose);
    if (space.get() < 0) {
        throw std::runtime_error("cannot open link dataspace '" + datasetPath + "'");
    }

    const hssize_t pointCount = H5Sget_simple_extent_npoints(space.get());
    if (pointCount < 0) {
        throw std::runtime_error("cannot determine link dataset size '" + datasetPath + "'");
    }
    std::vector<int8_t> buffer(static_cast<size_t>(pointCount));
    check_status(
        H5Dread(dataset.get(), H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
        "read link dataset '" + datasetPath + "'");

    std::string result(buffer.begin(), buffer.end());
    while (!result.empty() && result.back() == '\0') {
        result.pop_back();
    }
    return result;
}

hid_t hdfTypeFromCgnsType(const std::string& cgnsType) {
    if (cgnsType == "I1") return H5T_NATIVE_INT8;
    if (cgnsType == "B1") return H5T_NATIVE_UINT8;
    if (cgnsType == "I2") return H5T_NATIVE_INT16;
    if (cgnsType == "I4") return H5T_NATIVE_INT32;
    if (cgnsType == "I8") return H5T_NATIVE_INT64;
    if (cgnsType == "U1") return H5T_NATIVE_UINT8;
    if (cgnsType == "U2") return H5T_NATIVE_UINT16;
    if (cgnsType == "U4") return H5T_NATIVE_UINT32;
    if (cgnsType == "U8") return H5T_NATIVE_UINT64;
    if (cgnsType == "R4") return H5T_NATIVE_FLOAT;
    if (cgnsType == "R8") return H5T_NATIVE_DOUBLE;
    throw std::runtime_error("unsupported CGNS data type '" + cgnsType + "'");
}

char normalizeOrder(char order) {
    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(order)));
    if (normalized != 'C' && normalized != 'F') {
        throw std::invalid_argument("LazyHdf5Reader: order must be 'C' or 'F'");
    }
    return normalized;
}

std::string normalizeCgnsTypeToken(std::string type) {
    const auto first = type.find_first_not_of(" \t\r\n\0", 0, 5);
    if (first == std::string::npos) {
        return {};
    }
    const auto last = type.find_last_not_of(" \t\r\n\0", std::string::npos, 5);
    type = type.substr(first, last - first + 1);
    std::transform(type.begin(), type.end(), type.begin(), [](const unsigned char character) {
        return static_cast<char>(std::toupper(character));
    });
    return type;
}

size_t flatSize(const std::vector<size_t>& shape) {
    size_t result = 1;
    for (const size_t dimension : shape) {
        if (dimension != 0 && result > std::numeric_limits<size_t>::max() / dimension) {
            throw std::overflow_error("HDF5 array shape product overflow");
        }
        result *= dimension;
    }
    return result;
}

std::vector<size_t> indicesFromFortranFlat(
    size_t flatIndex,
    const std::vector<size_t>& shape) {

    std::vector<size_t> indices(shape.size(), 0);
    for (size_t dimension = 0; dimension < shape.size(); ++dimension) {
        const size_t extent = shape[dimension];
        indices[dimension] = extent == 0 ? 0 : flatIndex % extent;
        flatIndex = extent == 0 ? 0 : flatIndex / extent;
    }
    return indices;
}

size_t cFlatFromIndices(
    const std::vector<size_t>& indices,
    const std::vector<size_t>& shape) {

    size_t index = 0;
    size_t stride = 1;
    for (size_t dimension = indices.size(); dimension-- > 0;) {
        index += indices[dimension] * stride;
        stride *= shape[dimension];
    }
    return index;
}

template <typename T>
Array readNumericArrayTyped(
    hid_t dataset,
    const std::vector<size_t>& shape,
    const std::string& cgnsType,
    char order,
    hid_t memoryType = -1) {

    const size_t totalSize = flatSize(shape);
    const hid_t hdfType = memoryType < 0 ? hdfTypeFromCgnsType(cgnsType) : memoryType;

    if (order == 'F') {
        Array array = arrayfactory::empty<T>(shape, 'F');
        check_status(
            H5Dread(dataset, hdfType, H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()),
            "read " + cgnsType + " payload");
        return array;
    }

    std::vector<T> buffer(totalSize);
    check_status(
        H5Dread(dataset, hdfType, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
        "read " + cgnsType + " payload");

    Array array = arrayfactory::empty<T>(shape, 'C');
    T* destination = static_cast<T*>(array.rawData());
    if (shape.size() <= 1) {
        std::memcpy(destination, buffer.data(), totalSize * sizeof(T));
        return array;
    }

    for (size_t index = 0; index < totalSize; ++index) {
        const std::vector<size_t> indices = indicesFromFortranFlat(index, shape);
        destination[cFlatFromIndices(indices, shape)] = buffer[index];
    }
    return array;
}

Array readBooleanArray(
    hid_t dataset,
    const std::vector<size_t>& shape,
    char order,
    const std::string& cgnsType) {
    Array bytes = readNumericArrayTyped<uint8_t>(
        dataset, shape, cgnsType, order, H5T_NATIVE_UINT8);
    Array booleans = arrayfactory::empty<bool>(shape, order);
    const auto* source = static_cast<const uint8_t*>(bytes.rawData());
    auto* destination = static_cast<bool*>(booleans.rawData());
    for (size_t index = 0; index < bytes.size(); ++index) {
        destination[index] = source[index] != 0;
    }
    return booleans;
}

template <typename T, typename Component>
Array readComplexArrayTyped(
    hid_t dataset,
    const std::vector<size_t>& shape,
    char order) {

    hdf5_handle sourceType(H5Dget_type(dataset), H5Tclose);
    if (sourceType.get() < 0 || H5Tget_class(sourceType.get()) != H5T_COMPOUND ||
        H5Tget_nmembers(sourceType.get()) != 2) {
        throw std::runtime_error("complex CGNS payload must use a two-member HDF5 compound type");
    }

    hdf5_handle memoryType(H5Tcreate(H5T_COMPOUND, sizeof(T)), H5Tclose);
    if (memoryType.get() < 0) {
        throw std::runtime_error("cannot create complex HDF5 memory type");
    }
    for (unsigned member = 0; member < 2; ++member) {
        char* memberName = H5Tget_member_name(sourceType.get(), member);
        if (memberName == nullptr) {
            throw std::runtime_error("cannot inspect complex HDF5 member name");
        }
        const size_t offset = member * sizeof(Component);
        const hid_t componentType = std::is_same_v<Component, float>
            ? H5T_NATIVE_FLOAT
            : H5T_NATIVE_DOUBLE;
        const herr_t status = H5Tinsert(
            memoryType.get(), memberName, offset, componentType);
        H5free_memory(memberName);
        check_status(status, "create complex HDF5 memory member");
    }

    const size_t totalSize = flatSize(shape);
    if (order == 'F') {
        Array array = arrayfactory::empty<T>(shape, 'F');
        check_status(
            H5Dread(dataset, memoryType.get(), H5S_ALL, H5S_ALL, H5P_DEFAULT, array.rawData()),
            "read complex CGNS payload");
        return array;
    }

    std::vector<T> buffer(totalSize);
    check_status(
        H5Dread(dataset, memoryType.get(), H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
        "read complex CGNS payload");
    Array array = arrayfactory::empty<T>(shape, 'C');
    T* destination = static_cast<T*>(array.rawData());
    if (shape.size() <= 1) {
        std::memcpy(destination, buffer.data(), totalSize * sizeof(T));
        return array;
    }
    for (size_t index = 0; index < totalSize; ++index) {
        const std::vector<size_t> indices = indicesFromFortranFlat(index, shape);
        destination[cFlatFromIndices(indices, shape)] = buffer[index];
    }
    return array;
}

Array readArrayFromDataset(
    hid_t dataset,
    const std::vector<size_t>& shape,
    const std::string& cgnsType,
    char order,
    bool strictTypes) {

    if (cgnsType == "C1") {
        hdf5_handle space(H5Dget_space(dataset), H5Sclose);
        if (space.get() < 0) {
            throw std::runtime_error("cannot open string dataspace");
        }
        const hssize_t pointCount = H5Sget_simple_extent_npoints(space.get());
        if (pointCount < 0) {
            throw std::runtime_error("cannot determine string dataset size");
        }
        std::vector<int8_t> buffer(static_cast<size_t>(pointCount));
        check_status(
            H5Dread(dataset, H5T_NATIVE_INT8, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data()),
            "read C1 payload");
        std::string value(buffer.begin(), buffer.end());
        while (!value.empty() && value.back() == '\0') {
            value.pop_back();
        }
        return Array(value);
    }

    if (cgnsType == "I1") return strictTypes
        ? readNumericArrayTyped<int32_t>(dataset, shape, cgnsType, order, H5T_NATIVE_INT32)
        : readNumericArrayTyped<int8_t>(dataset, shape, cgnsType, order);
    if (cgnsType == "I2") return strictTypes
        ? readNumericArrayTyped<int32_t>(dataset, shape, cgnsType, order, H5T_NATIVE_INT32)
        : readNumericArrayTyped<int16_t>(dataset, shape, cgnsType, order);
    if (cgnsType == "I4") return readNumericArrayTyped<int32_t>(dataset, shape, cgnsType, order);
    if (cgnsType == "I8") return readNumericArrayTyped<int64_t>(dataset, shape, cgnsType, order);
    if (cgnsType == "U1") return strictTypes
        ? readNumericArrayTyped<uint32_t>(dataset, shape, cgnsType, order, H5T_NATIVE_UINT32)
        : readNumericArrayTyped<uint8_t>(dataset, shape, cgnsType, order);
    if (cgnsType == "U2") return strictTypes
        ? readNumericArrayTyped<uint32_t>(dataset, shape, cgnsType, order, H5T_NATIVE_UINT32)
        : readNumericArrayTyped<uint16_t>(dataset, shape, cgnsType, order);
    if (cgnsType == "U4") return readNumericArrayTyped<uint32_t>(dataset, shape, cgnsType, order);
    if (cgnsType == "U8") return readNumericArrayTyped<uint64_t>(dataset, shape, cgnsType, order);
    if (cgnsType == "R4") return readNumericArrayTyped<float>(dataset, shape, cgnsType, order);
    if (cgnsType == "R8") return readNumericArrayTyped<double>(dataset, shape, cgnsType, order);
    if (cgnsType == "B1") return strictTypes
        ? readNumericArrayTyped<int32_t>(dataset, shape, cgnsType, order, H5T_NATIVE_INT32)
        : readBooleanArray(dataset, shape, order, cgnsType);
    if (cgnsType == "X4") return readComplexArrayTyped<std::complex<float>, float>(dataset, shape, order);
    if (cgnsType == "X8") return readComplexArrayTyped<std::complex<double>, double>(dataset, shape, order);

    throw std::runtime_error("unsupported payload type '" + cgnsType + "'");
}

bool isReservedDatasetName(const std::string& name) {
    return name == " data" || name == " file" || name == " path" || name == " link";
}

std::shared_ptr<Node> makeNodeForLabel(
    const std::string& name,
    const std::string& label) {

    if (label == "Zone_t") {
        return std::make_shared<Zone>(name);
    }
    if (label == "CGNSBase_t") {
        return std::make_shared<Base>(name);
    }
    if (label == "CGNSTree_t" || label == "Root Node of HDF5 File") {
        auto tree = std::make_shared<Tree>();
        tree->setName(name);
        tree->setType(label);
        return tree;
    }
    return std::make_shared<Node>(name, label.empty() ? "DataArray_t" : label);
}

} // namespace

class LazyHdf5Reader::State final
    : public NodeExpansion,
      public std::enable_shared_from_this<LazyHdf5Reader::State> {
private:
    struct Record {
        std::string hdf5Path;
        mutable std::string contentPath;
        mutable std::string dataType = "MT";
        std::string sourceTypeAttribute;
        mutable bool hasData = false;
        bool dataLoaded = false;
        bool isLink = false;
        mutable bool linkResolved = false;
        hsize_t nextObjectIndex = 0;
        hsize_t objectCount = 0;
        bool objectCountKnown = false;
        mutable size_t childCount = 0;
        mutable bool childCountKnown = false;
        H5_index_t linkIndexType = H5_INDEX_CRT_ORDER;
        bool linkIndexTypeKnown = false;
        ChildrenLoadState childrenState = ChildrenLoadState::Unloaded;
    };

public:
    State(std::string filename, char order, const bool safeMode, const bool strictTypes)
        : _filename(std::move(filename)),
          _order(normalizeOrder(order)),
          _safeMode(safeMode),
          _strictTypes(strictTypes),
          _file(H5Fopen(_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose) {

        if (_file.get() < 0) {
            throw std::runtime_error("LazyHdf5Reader: failed to open HDF5 file '" + _filename + "'");
        }
    }

    std::shared_ptr<Node> makeRoot() {
        try {
            return makeNode("/");
        } catch (const std::exception& error) {
            if (!_safeMode) {
                throw;
            }
            const std::shared_ptr<Node> root = makeNodeForLabel(
                "HDF5 MotherNode", "Root Node of HDF5 File");
            root->setExpansion(shared_from_this());
            Record record;
            record.hdf5Path = "/";
            record.contentPath = "/";
            record.childrenState = ChildrenLoadState::Unloaded;
            _records.emplace(root.get(), std::move(record));
            addCorrupted(*root, "/", error.what());
            return root;
        }
    }

    std::shared_ptr<Node> nodeAtPath(const std::string& path) {
        std::vector<std::string> pathElements;
        std::stringstream pathStream(path);
        std::string element;
        while (std::getline(pathStream, element, '/')) {
            if (!element.empty()) {
                pathElements.push_back(element);
            }
        }
        if (pathElements.empty()) {
            return nullptr;
        }

        // Build a temporary hierarchy containing only ancestors on this
        // path. Do not use _root: its direct children should remain unloaded.
        std::vector<const Node*> temporaryNodes;
        auto discardTemporaryRecords = [&]() {
            for (const Node* node : temporaryNodes) {
                _records.erase(node);
            }
            temporaryNodes.clear();
        };

        try {
            std::shared_ptr<Node> current = makeRoot();
            temporaryNodes.push_back(current.get());

            auto discardCorruptedPath = [&](const std::string& childPath,
                                             const std::string& detail,
                                             const bool isFinalComponent) {
                addCorrupted(*current, childPath, detail);
                const auto& loadedChildren = current->loadedChildren();
                std::shared_ptr<Node> marker =
                    isFinalComponent && !loadedChildren.empty() ? loadedChildren.back() : nullptr;
                if (marker) {
                    marker->detach();
                }
                discardTemporaryRecords();
                return marker;
            };

            size_t firstElement = 0;
            if (pathElements.front() == current->name()) {
                firstElement = 1;
            }

            for (size_t index = firstElement; index < pathElements.size(); ++index) {
                const std::string& childName = pathElements[index];
                if (isReservedDatasetName(childName)) {
                    discardTemporaryRecords();
                    return nullptr;
                }

                const Record& parentRecord = recordFor(*current);
                const std::string childPath = hdf5ChildPath(parentRecord.hdf5Path, childName);
                const htri_t exists = H5Lexists(_file.get(), childPath.c_str(), H5P_DEFAULT);
                if (exists == 0) {
                    discardTemporaryRecords();
                    return nullptr;
                }
                if (exists < 0) {
                    if (!_safeMode) {
                        throw std::runtime_error("cannot inspect path component '" + childPath + "'");
                    }
                    return discardCorruptedPath(
                        childPath,
                        "cannot inspect HDF5 path component",
                        index + 1 == pathElements.size());
                }

                hdf5_handle object(H5Oopen(_file.get(), childPath.c_str(), H5P_DEFAULT), H5Oclose);
                if (object.get() < 0) {
                    if (!_safeMode) {
                        throw std::runtime_error("failed to inspect path component '" + childPath + "'");
                    }
                    return discardCorruptedPath(
                        childPath,
                        "failed to inspect HDF5 path component",
                        index + 1 == pathElements.size());
                }
                if (H5Iget_type(object.get()) != H5I_GROUP) {
                    discardTemporaryRecords();
                    return nullptr;
                }

                std::shared_ptr<Node> child;
                try {
                    child = makeNode(childPath);
                } catch (const std::exception& error) {
                    if (!_safeMode) {
                        throw;
                    }
                    return discardCorruptedPath(
                        childPath,
                        error.what(),
                        index + 1 == pathElements.size());
                }
                temporaryNodes.push_back(child.get());
                child->attachTo(current, -1, false);
                current = std::move(child);
            }

            current->detach();
            temporaryNodes.pop_back(); // Keep the selected node's lazy record.
            discardTemporaryRecords();
            return current;
        } catch (...) {
            discardTemporaryRecords();
            throw;
        }
    }

    const std::string& filename() const {
        return _filename;
    }

    char order() const {
        return _order;
    }

    void close() {
        _file.reset();
    }

    bool isOpen() const {
        return _file.get() >= 0;
    }

    bool hasWarnings() const {
        return !_warnings.empty();
    }

    bool safeMode() const {
        return _safeMode;
    }

    bool strictTypes() const {
        return _strictTypes;
    }

    std::string cgnsTypeAttribute(const std::shared_ptr<Node>& node) const {
        if (!node) {
            throw std::invalid_argument("LazyHdf5Reader::cgnsTypeAttribute: node cannot be null");
        }
        const Record& record = recordFor(*node);
        if (!isOpen()) {
            return record.sourceTypeAttribute;
        }
        hdf5_handle group(H5Gopen2(_file.get(), record.hdf5Path.c_str(), H5P_DEFAULT), H5Gclose);
        if (group.get() < 0) {
            throw std::runtime_error("cannot inspect CGNS type attribute at '" + record.hdf5Path + "'");
        }
        return readStringAttribute(group.get(), "type");
    }

    std::vector<std::shared_ptr<Node>> findNodesByCgnsType(
        const std::shared_ptr<Node>& root,
        const std::string& cgnsType) const {
        if (!root) {
            throw std::invalid_argument("LazyHdf5Reader::findNodesByCgnsType: root cannot be null");
        }
        const std::string target = normalizeCgnsTypeToken(cgnsType);
        if (target.empty()) {
            throw std::invalid_argument("CGNS type to inspect cannot be empty");
        }

        std::vector<std::shared_ptr<Node>> matches;
        std::vector<std::shared_ptr<Node>> pending{root};
        while (!pending.empty()) {
            std::shared_ptr<Node> node = std::move(pending.back());
            pending.pop_back();

            try {
                (void)recordFor(*node);
            } catch (const std::runtime_error&) {
                continue;
            }

            if (normalizeCgnsTypeToken(cgnsTypeAttribute(node)) == target) {
                matches.push_back(node);
            }
            node->ensureChildrenLoaded();
            const auto& children = node->loadedChildren();
            pending.insert(pending.end(), children.rbegin(), children.rend());
        }
        return matches;
    }

    ChildrenLoadState childrenLoadState(const Node& node) const override {
        return recordFor(node).childrenState;
    }

    size_t childCount(const Node& node) const override {
        const Record& record = recordFor(node);
        if (!isOpen()) {
            return node.loadedChildren().size();
        }
        try {
            if (record.isLink) {
                resolveLinkContent(node, record);
                if (linkTargetsAncestor(node, record)) {
                    record.childCount = 0;
                    record.childCountKnown = true;
                    return 0;
                }
            }
            if (record.childCountKnown) {
                return record.childCount;
            }

            hdf5_handle group(H5Gopen2(_file.get(), record.contentPath.c_str(), H5P_DEFAULT), H5Gclose);
            if (group.get() < 0) {
                throw std::runtime_error("LazyHdf5Reader: failed to open group while counting children");
            }
            const hsize_t objectCount = hdf5_detail::group_link_count(
                group.get(), "enumerate children");
            size_t count = 0;
            for (hsize_t index = 0; index < objectCount; ++index) {
                const std::string name = hdf5_detail::link_name_by_index(
                    group.get(), index, "cannot read child name while counting children");
                if (isReservedDatasetName(name)) {
                    continue;
                }
                if (hdf5_detail::object_type_by_index(
                        group.get(), index, "cannot inspect child while counting children") ==
                    H5O_TYPE_GROUP) {
                    ++count;
                }
            }
            record.childCount = count;
            record.childCountKnown = true;
            return count;
        } catch (const std::exception& error) {
            if (!_safeMode) {
                throw;
            }
            addCorrupted(const_cast<Node&>(node), record.hdf5Path, error.what());
            record.childCount = node.loadedChildren().size();
            record.childCountKnown = true;
            return record.childCount;
        }
    }

    void ensureChildrenLoaded(Node& node, const size_t minimumChildren) override {
        Record& record = recordFor(node);
        if (record.childrenState == ChildrenLoadState::Complete) {
            return;
        }
        if (!isOpen()) {
            throw std::runtime_error("LazyHdf5Reader: file is closed");
        }

        const size_t requestedChildren =
            minimumChildren == 0 ? kMaximumSize : minimumChildren;

        try {
            resolveLinkContent(node, record);
            if (record.isLink && linkTargetsAncestor(node, record)) {
                record.childCount = 0;
                record.childCountKnown = true;
                record.childrenState = ChildrenLoadState::Complete;
                return;
            }

            hdf5_handle group(H5Gopen2(_file.get(), record.contentPath.c_str(), H5P_DEFAULT), H5Gclose);
            if (group.get() < 0) {
                throw std::runtime_error("failed to open group");
            }

            if (!record.objectCountKnown) {
                record.objectCount = hdf5_detail::group_link_count(
                    group.get(), "enumerate children");
                record.objectCountKnown = true;
            }

            const std::shared_ptr<Node> parent = node.selfPtr();
            if (!parent) {
                throw std::runtime_error("lazy nodes must be managed by shared_ptr");
            }

            while (node.loadedChildren().size() < requestedChildren &&
                   record.nextObjectIndex < record.objectCount) {

                const hsize_t index = record.nextObjectIndex++;
                std::string childPath = hdf5ChildPath(
                    record.contentPath, "#" + std::to_string(index));
                try {
                    ssize_t nameLength = H5Lget_name_by_idx(
                        group.get(),
                        ".",
                        record.linkIndexType,
                        H5_ITER_INC,
                        index,
                        nullptr,
                        0,
                        H5P_DEFAULT);

                    if (nameLength < 0 && !record.linkIndexTypeKnown &&
                        record.linkIndexType == H5_INDEX_CRT_ORDER) {
                        H5Eclear2(H5E_DEFAULT);
                        record.linkIndexType = H5_INDEX_NAME;
                        nameLength = H5Lget_name_by_idx(
                            group.get(),
                            ".",
                            record.linkIndexType,
                            H5_ITER_INC,
                            index,
                            nullptr,
                            0,
                            H5P_DEFAULT);
                    }
                    record.linkIndexTypeKnown = true;
                    if (nameLength < 0) {
                        throw std::runtime_error(
                            "cannot determine child name at index " + std::to_string(index));
                    }
                    std::string childName(static_cast<size_t>(nameLength) + 1, '\0');
                    const ssize_t copiedNameLength = H5Lget_name_by_idx(
                        group.get(),
                        ".",
                        record.linkIndexType,
                        H5_ITER_INC,
                        index,
                        childName.data(),
                        childName.size(),
                        H5P_DEFAULT);
                    if (copiedNameLength < 0) {
                        throw std::runtime_error(
                            "cannot read child name at index " + std::to_string(index));
                    }
                    childName.resize(static_cast<size_t>(copiedNameLength));
                    if (isReservedDatasetName(childName)) {
                        continue;
                    }

                    childPath = hdf5ChildPath(record.contentPath, childName);
                    hdf5_handle object(H5Oopen(group.get(), childName.c_str(), H5P_DEFAULT), H5Oclose);
                    if (object.get() < 0) {
                        throw std::runtime_error("failed to inspect child object '" + childPath + "'");
                    }
                    if (H5Iget_type(object.get()) != H5I_GROUP) {
                        continue;
                    }
                    const std::shared_ptr<Node> child = makeNode(childPath);
                    child->attachTo(parent, -1, false);
                } catch (const std::exception& error) {
                    if (!_safeMode) {
                        throw;
                    }
                    addCorrupted(*parent, childPath, error.what());
                }
            }

            record.childrenState = record.nextObjectIndex >= record.objectCount
                ? ChildrenLoadState::Complete
                : ChildrenLoadState::Partial;
        } catch (const std::exception& error) {
            if (!_safeMode) {
                throw contextualError(node, record, "load children", error.what());
            }
            addCorrupted(node, record.hdf5Path, error.what());
            record.childrenState = ChildrenLoadState::Complete;
        }
    }

    void ensureDataLoaded(Node& node) override {
        Record& record = recordFor(node);
        std::string dataPath = hdf5ChildPath(record.contentPath, " data");
        try {
            resolveLinkContent(node, record);
            dataPath = hdf5ChildPath(record.contentPath, " data");
            if (record.dataLoaded || !record.hasData) {
                record.dataLoaded = true;
                return;
            }
            if (!isOpen()) {
                throw std::runtime_error("LazyHdf5Reader: file is closed");
            }

            hdf5_handle dataset(H5Dopen2(_file.get(), dataPath.c_str(), H5P_DEFAULT), H5Dclose);
            if (dataset.get() < 0) {
                throw std::runtime_error("failed to open dataset '" + dataPath + "'");
            }
            hdf5_handle space(H5Dget_space(dataset.get()), H5Sclose);
            if (space.get() < 0) {
                throw std::runtime_error("failed to open dataspace '" + dataPath + "'");
            }

            const int rank = H5Sget_simple_extent_ndims(space.get());
            if (rank < 0) {
                throw std::runtime_error("cannot determine dataset rank");
            }
            std::vector<hsize_t> dimensions(static_cast<size_t>(rank));
            if (rank > 0) {
                check_status(
                    H5Sget_simple_extent_dims(space.get(), dimensions.data(), nullptr),
                    "read dataset dimensions");
            }
            std::vector<size_t> shape(dimensions.begin(), dimensions.end());
            if (record.dataType != "C1") {
                std::reverse(shape.begin(), shape.end());
            }
            if (_safeMode && std::any_of(
                    shape.begin(), shape.end(), [](const size_t dimension) {
                        return dimension == 0;
                    })) {
                throw std::runtime_error("malformed zero-size HDF5 dataset");
            }

            node.setData(readArrayFromDataset(
                dataset.get(), shape, record.dataType, _order, _strictTypes));
            record.dataLoaded = true;
        } catch (const std::exception& error) {
            const std::runtime_error contextual = contextualError(
                node,
                record,
                "load data dataset '" + dataPath + "'",
                error.what());
            if (!_safeMode) {
                throw contextual;
            }
            record.dataLoaded = true;
            const auto parent = node.parent().lock();
            addCorrupted(parent ? *parent : node, record.hdf5Path, contextual.what());
        }
    }

    bool hasData(const Node& node) const override {
        const Record& record = recordFor(node);
        if (!resolveLinkContentForQuery(node, record)) {
            return false;
        }
        return record.hasData;
    }

    std::optional<bool> dataIsScalar(const Node& node) const override {
        const Record& record = recordFor(node);
        if (!resolveLinkContentForQuery(node, record)) {
            return false;
        }
        if (!record.hasData || record.dataType == "C1") {
            return false;
        }

        const std::string dataPath = hdf5ChildPath(record.contentPath, " data");
        try {
            hdf5_handle dataset(H5Dopen2(_file.get(), dataPath.c_str(), H5P_DEFAULT), H5Dclose);
            if (dataset.get() < 0) {
                throw std::runtime_error("failed to open dataset '" + dataPath + "'");
            }
            hdf5_handle space(H5Dget_space(dataset.get()), H5Sclose);
            if (space.get() < 0) {
                throw std::runtime_error("failed to open dataspace '" + dataPath + "'");
            }

            const int rank = H5Sget_simple_extent_ndims(space.get());
            if (rank < 0) {
                throw std::runtime_error("cannot determine dataset rank");
            }
            if (rank == 0) {
                return true;
            }
            std::vector<hsize_t> dimensions(static_cast<size_t>(rank));
            check_status(
                H5Sget_simple_extent_dims(space.get(), dimensions.data(), nullptr),
                "read dataset dimensions");
            hsize_t elementCount = 1;
            for (const hsize_t dimension : dimensions) {
                elementCount *= dimension;
            }
            if (_safeMode && elementCount == 0) {
                throw std::runtime_error("malformed zero-size HDF5 dataset");
            }
            return elementCount == 1;
        } catch (const std::exception& error) {
            const std::runtime_error contextual = contextualError(
                node,
                record,
                "inspect scalar payload at dataset '" + dataPath + "'",
                error.what());
            if (!_safeMode) {
                throw contextual;
            }
            Node& mutableNode = const_cast<Node&>(node);
            const auto parent = mutableNode.parent().lock();
            addCorrupted(parent ? *parent : mutableNode, record.hdf5Path, contextual.what());
            return false;
        }
    }

    std::optional<bool> dataIsString(const Node& node) const override {
        const Record& record = recordFor(node);
        if (!resolveLinkContentForQuery(node, record)) {
            return false;
        }
        if (!record.hasData) {
            return false;
        }
        return record.dataType == "C1";
    }

    bool dataIsLoaded(const Node& node) const override {
        const Record& record = recordFor(node);
        if (!resolveLinkContentForQuery(node, record)) {
            return true;
        }
        return record.dataLoaded || !record.hasData;
    }

    void dataAssigned(Node& node) override {
        auto iterator = _records.find(&node);
        if (iterator != _records.end()) {
            Record& record = iterator->second;
            record.dataLoaded = true;
            const auto array = std::dynamic_pointer_cast<Array>(node.dataPtr());
            if (!array || array->isNone()) {
                record.hasData = false;
                record.dataType = "MT";
            } else {
                record.hasData = true;
                if (array->hasString()) record.dataType = "C1";
                else if (array->hasDataOfType<int8_t>()) record.dataType = "I1";
                else if (array->hasDataOfType<int16_t>()) record.dataType = "I2";
                else if (array->hasDataOfType<int32_t>()) record.dataType = "I4";
                else if (array->hasDataOfType<int64_t>()) record.dataType = "I8";
                else if (array->hasDataOfType<uint8_t>()) record.dataType = "U1";
                else if (array->hasDataOfType<uint16_t>()) record.dataType = "U2";
                else if (array->hasDataOfType<uint32_t>()) record.dataType = "U4";
                else if (array->hasDataOfType<uint64_t>()) record.dataType = "U8";
                else if (array->hasDataOfType<float>()) record.dataType = "R4";
                else if (array->hasDataOfType<double>()) record.dataType = "R8";
                else if (array->hasDataOfType<bool>()) record.dataType = "B1";
                else if (array->hasDataOfType<std::complex<float>>()) record.dataType = "X4";
                else if (array->hasDataOfType<std::complex<double>>()) record.dataType = "X8";
            }
        }
    }

    void dataUnloaded(Node& node) override {
        auto iterator = _records.find(&node);
        if (iterator != _records.end() && iterator->second.hasData) {
            iterator->second.dataLoaded = false;
        }
    }

    void beginExternalWrite() override {
        _file.reset();
    }

    void endExternalWrite() override {
        if (isOpen()) {
            return;
        }
        _file = hdf5_handle(H5Fopen(_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        if (_file.get() < 0) {
            throw std::runtime_error(
                "LazyHdf5Reader: failed to reopen HDF5 file '" + _filename + "' after external write");
        }
    }

private:
    void resolveLinkContent(const Node& node, const Record& record) const {
        if (!record.isLink || record.linkResolved) {
            return;
        }
        if (!isOpen()) {
            throw std::runtime_error(
                "LazyHdf5Reader: file is closed while resolving link at node '" + node.path() + "'");
        }

        std::string contentPath = hdf5ChildPath(record.hdf5Path, " link");
        std::unordered_set<std::string> visitedPaths;
        for (size_t depth = 0; depth < 64; ++depth) {
            if (!visitedPaths.insert(contentPath).second) {
                throw std::runtime_error("link target contains a cycle");
            }

            hdf5_handle group(H5Gopen2(_file.get(), contentPath.c_str(), H5P_DEFAULT), H5Gclose);
            if (group.get() < 0) {
                throw std::runtime_error(
                    "cannot open link target at HDF5 path '" + contentPath + "'");
            }

            const std::string targetType = readStringAttribute(group.get(), "type");
            if (targetType != "LK") {
                record.contentPath = contentPath;
                record.dataType = targetType.empty() ? "MT" : targetType;
                const std::string dataPath = hdf5ChildPath(contentPath, " data");
                const htri_t dataExists = H5Lexists(_file.get(), dataPath.c_str(), H5P_DEFAULT);
                if (dataExists < 0) {
                    throw std::runtime_error("cannot inspect link target payload");
                }
                record.hasData = dataExists > 0 && record.dataType != "MT";
                record.linkResolved = true;
                return;
            }

            contentPath = hdf5ChildPath(contentPath, " link");
        }

        throw std::runtime_error("link target nesting exceeds the supported depth");
    }

    bool resolveLinkContentForQuery(const Node& node, const Record& record) const {
        try {
            resolveLinkContent(node, record);
            return true;
        } catch (const std::exception& error) {
            if (!_safeMode) {
                throw;
            }
            Node& mutableNode = const_cast<Node&>(node);
            const auto parent = mutableNode.parent().lock();
            const std::runtime_error contextual = contextualError(
                node, record, "resolve link target", error.what());
            addCorrupted(parent ? *parent : mutableNode, record.hdf5Path, contextual.what());
            return false;
        }
    }

    std::string hdf5ObjectIdentity(const std::string& path) const {
        hdf5_handle group(H5Gopen2(_file.get(), path.c_str(), H5P_DEFAULT), H5Gclose);
        if (group.get() < 0) {
            throw std::runtime_error("cannot inspect HDF5 object identity at '" + path + "'");
        }
#if H5_VERSION_GE(1, 12, 0)
        H5O_info2_t objectInfo{};
        check_status(
            H5Oget_info3(group.get(), &objectInfo, H5O_INFO_BASIC),
            "inspect HDF5 object identity");
        hdf5_handle file(H5Iget_file_id(group.get()), H5Fclose);
        if (file.get() < 0) {
            throw std::runtime_error("cannot inspect HDF5 object file identity");
        }
        const ssize_t fileNameLength = H5Fget_name(file.get(), nullptr, 0);
        if (fileNameLength < 0) {
            throw std::runtime_error("cannot inspect HDF5 object file name");
        }
        std::string fileName(static_cast<size_t>(fileNameLength) + 1, '\0');
        if (H5Fget_name(file.get(), fileName.data(), fileName.size()) < 0) {
            throw std::runtime_error("cannot read HDF5 object file name");
        }
        fileName.resize(static_cast<size_t>(fileNameLength));
        char* tokenText = nullptr;
        check_status(
            H5Otoken_to_str(file.get(), &objectInfo.token, &tokenText),
            "serialize HDF5 object identity");
        const std::string identity = fileName + ":" + (tokenText == nullptr ? "" : tokenText);
        if (tokenText != nullptr) {
            H5free_memory(tokenText);
        }
        return identity;
#else
        H5O_info_t objectInfo{};
        check_status(H5Oget_info(group.get(), &objectInfo), "inspect HDF5 object identity");
        return std::to_string(objectInfo.fileno) + ":" +
            std::to_string(static_cast<unsigned long long>(objectInfo.addr));
#endif
    }

    bool linkTargetsAncestor(const Node& node, const Record& record) const {
        if (!record.isLink) {
            return false;
        }
        const std::string targetIdentity = hdf5ObjectIdentity(record.contentPath);
        std::shared_ptr<Node> ancestor = node.parent().lock();
        while (ancestor) {
            const auto iterator = _records.find(ancestor.get());
            if (iterator != _records.end()) {
                const Record& ancestorRecord = iterator->second;
                resolveLinkContent(*ancestor, ancestorRecord);
                if (hdf5ObjectIdentity(ancestorRecord.contentPath) == targetIdentity) {
                    return true;
                }
            }
            ancestor = ancestor->parent().lock();
        }
        return false;
    }

    void addCorrupted(Node& parent, const std::string& hdf5Path, const std::string& detail) const {
        if (!_safeMode || !_corruptedPaths.insert(hdf5Path).second) {
            return;
        }

        const std::string message =
            "file='" + _filename + "' hdf5_path='" + hdf5Path + "': " + detail;
        _warnings.push_back(message);
        std::cerr << "Warning: malformed HDF5/CGNS node " << message << "\n";

        auto marker = std::make_shared<Node>("Corrupted", "Corrupted_t");
        marker->setData(message);
        const auto parentPointer = parent.selfPtr();
        if (!parentPointer) {
            return;
        }
        marker->attachTo(parentPointer, -1, false);
    }

    Record& recordFor(Node& node) {
        const auto iterator = _records.find(&node);
        if (iterator == _records.end()) {
            throw std::runtime_error("lazy node is not owned by this reader");
        }
        return iterator->second;
    }

    const Record& recordFor(const Node& node) const {
        const auto iterator = _records.find(&node);
        if (iterator == _records.end()) {
            throw std::runtime_error("lazy node is not owned by this reader");
        }
        return iterator->second;
    }

    [[nodiscard]] std::runtime_error contextualError(
        const Node& node,
        const Record& record,
        const std::string& operation,
        const std::string& detail) const {

        return std::runtime_error(
            "Lazy CGNS/HDF5 error while attempting to " + operation +
            " at node path '" + node.path() +
            "' (HDF5 path '" + record.hdf5Path + "'): " + detail);
    }

    std::shared_ptr<Node> makeNode(const std::string& path) {
        if (!isOpen()) {
            throw std::runtime_error("LazyHdf5Reader: file is closed");
        }
        hdf5_handle group(H5Gopen2(_file.get(), path.c_str(), H5P_DEFAULT), H5Gclose);
        if (group.get() < 0) {
            throw std::runtime_error("failed to open node group");
        }

        std::string name = readStringAttribute(group.get(), "name");
        const std::string label = readStringAttribute(group.get(), "label");
        const std::string dataType = readStringAttribute(group.get(), "type");
        if (_safeMode) {
            if (name.empty()) {
                throw std::runtime_error("missing required CGNS 'name' attribute");
            }
            if (label.empty()) {
                throw std::runtime_error("missing required CGNS 'label' attribute");
            }
            if (dataType.empty()) {
                throw std::runtime_error("missing required CGNS 'type' attribute");
            }
        }

        if (name.empty()) {
            if (path == "/") {
                name = "HDF5 MotherNode";
            } else {
                const size_t separator = path.find_last_of('/');
                name = separator == std::string::npos ? path : path.substr(separator + 1);
            }
        }
        if (name.size() > kCGNSNameHardLimitBytes) {
            throw std::runtime_error(
                "CGNS node name at HDF5 path '" + path + "' is " + std::to_string(name.size()) +
                " bytes; the maximum supported length is " + std::to_string(kCGNSNameHardLimitBytes) + " bytes");
        }

        const std::shared_ptr<Node> node = makeNodeForLabel(name, label);
        node->setExpansion(shared_from_this());

        Record record;
        record.hdf5Path = path;
        record.contentPath = path;
        record.dataType = dataType.empty() ? "MT" : dataType;
        record.sourceTypeAttribute = dataType;
        record.isLink = record.dataType == "LK";
        if (record.isLink) {
            const std::string targetFile = readInt8StringDataset(_file.get(), path + "/ file");
            const std::string targetPath = readInt8StringDataset(_file.get(), path + "/ path");
            if (targetPath.empty()) {
                throw std::runtime_error("link node has empty target path");
            }
            node->setLinkTarget(targetFile, targetPath);
            record.childrenState = ChildrenLoadState::Unloaded;
        } else {
            const std::string dataPath = hdf5ChildPath(path, " data");
            const htri_t dataExists = H5Lexists(_file.get(), dataPath.c_str(), H5P_DEFAULT);
            if (dataExists < 0) {
                throw std::runtime_error("cannot inspect data dataset");
            }
            record.hasData = dataExists > 0 && record.dataType != "MT";
        }

        _records.emplace(node.get(), std::move(record));
        return node;
    }

    std::string _filename;
    char _order;
    bool _safeMode;
    bool _strictTypes;
    hdf5_handle _file;
    std::unordered_map<const Node*, Record> _records;
    mutable std::unordered_set<std::string> _corruptedPaths;
    mutable std::vector<std::string> _warnings;
};

LazyHdf5Reader::LazyHdf5Reader(
    const std::string& filename,
    const char order,
    const bool safeMode,
    const bool strictTypes)
    : _state(std::make_shared<State>(filename, order, safeMode, strictTypes)),
      _root(_state->makeRoot()) {}

LazyHdf5Reader::LazyHdf5Reader(
    const std::string& filename,
    const bool safeMode)
    : LazyHdf5Reader(filename, 'F', safeMode, true) {}

LazyHdf5Reader::~LazyHdf5Reader() = default;

LazyHdf5Reader::LazyHdf5Reader(LazyHdf5Reader&& other) noexcept
    : _state(std::move(other._state)),
      _root(std::move(other._root)) {}

LazyHdf5Reader& LazyHdf5Reader::operator=(LazyHdf5Reader&& other) noexcept {
    if (this != &other) {
        _state = std::move(other._state);
        _root = std::move(other._root);
    }
    return *this;
}

std::shared_ptr<Node> LazyHdf5Reader::root() const {
    return _root;
}

std::shared_ptr<Node> LazyHdf5Reader::nodeAtPath(const std::string& path) const {
    return _state->nodeAtPath(path);
}

std::string LazyHdf5Reader::cgnsTypeAttribute(const std::shared_ptr<Node>& node) const {
    return _state->cgnsTypeAttribute(node);
}

std::vector<std::shared_ptr<Node>> LazyHdf5Reader::findNodesByCgnsType(
    const std::string& cgnsType) const {
    return _state->findNodesByCgnsType(_root, cgnsType);
}

const std::string& LazyHdf5Reader::filename() const {
    return _state->filename();
}

char LazyHdf5Reader::order() const {
    return _state->order();
}

void LazyHdf5Reader::close() const {
    _state->close();
}

bool LazyHdf5Reader::isOpen() const {
    return _state->isOpen();
}

bool LazyHdf5Reader::hasWarnings() const {
    return _state->hasWarnings();
}

bool LazyHdf5Reader::safeMode() const {
    return _state->safeMode();
}

bool LazyHdf5Reader::strictTypes() const {
    return _state->strictTypes();
}

void LazyHdf5Reader::ensureChildrenLoaded(
    const std::shared_ptr<Node>& node,
    const size_t minimumChildren) const {

    if (!node) {
        throw std::invalid_argument("LazyHdf5Reader::ensureChildrenLoaded: node cannot be null");
    }
    node->ensureChildrenLoaded(minimumChildren);
}

void LazyHdf5Reader::ensureDataLoaded(const std::shared_ptr<Node>& node) const {
    if (!node) {
        throw std::invalid_argument("LazyHdf5Reader::ensureDataLoaded: node cannot be null");
    }
    node->ensureDataLoaded();
}

} // namespace io::hdf5::cgns

#endif // ENABLE_HDF5_IO
