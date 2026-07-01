#include <pybind11/pybind11.h>

#include "cgns/base_pybind.hpp"
#include "cgns/tree_pybind.hpp"
#include "cgns/zone_pybind.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_cgns, m) {
    bindZone(m);
    bindBase(m);
    bindTree(m);
}
