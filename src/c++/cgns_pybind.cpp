#include <pybind11/pybind11.h>

#include "cgns/zone_pybind.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_cgns, m) {
    bindZone(m);
}
