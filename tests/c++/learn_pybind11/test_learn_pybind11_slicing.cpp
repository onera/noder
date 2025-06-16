# include "test_learn_pybind11_slicing.hpp"

# define all py::slice(py::none(), py::none(), py::none())

void test_slicingReferences() {
    /*
        This test shows the short-coming of slicing a py::array, which
        creates copies, unlike the equivalent slicing operation in Python.
        We should find a way to slice by referece using pybind.
    */


    int rawarray[3] = {1, 2, 3};
    py::array_t<int> arr = py::array_t<int>({3}, rawarray);

    auto arrdata = arr.mutable_unchecked<1>();

    // casting to new py::array
    py::array first = arr[py::make_tuple(0)];

    auto firstdata = first.mutable_unchecked<int,0>();
    firstdata() = 5;

    if (arrdata(0) == firstdata()) {
        throw py::value_error("unexpectedly did not make copy using py::array");
    }

    // using default type
    py::detail::item_accessor first_ref = arr[py::make_tuple(0)];
    first_ref = 7; 

    if ( arrdata(0) == 7 ) {
        throw py::value_error("unexpectedly did not make copy using accessor");
    }

}


void test_pointerAccess() {
    int raw_array[3] = {0, 1, 2};
    py::array_t<int> pyarray = py::array_t<int>({3}, raw_array);
    int* data = static_cast<int*>(pyarray.request().ptr);
    
    // modification of last item using new pointer to data
    int* last_item = &data[2];
    *last_item = 5;

    if (data[2] != 5) {
        throw py::value_error("expected in-place modification by ptr");
    }

    // modification of second item using reference to data
    int& second_item = data[1];
    second_item = 7;

    if (data[1] != 7) {
        throw py::value_error("expected in-place modification by ref");
    }



}