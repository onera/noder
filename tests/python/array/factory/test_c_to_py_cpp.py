import pytest
from noder.tests.array import factory as test_in_cpp
import noder.array.data_types as dtypes

@pytest.mark.parametrize("typestr", dtypes.scalar_types)
@pytest.mark.parametrize("copy", [True, False])
@pytest.mark.parametrize("type_of_array_in_cpp", ["carray", "stdarray", "vector"])
@pytest.mark.parametrize("rank",[1,2,3])
def test_cpp_toArray(type_of_array_in_cpp, typestr, copy, rank):
    test_function = getattr(test_in_cpp,f"from_{type_of_array_in_cpp}_toArray{rank}D_{typestr}")
    try:
        test_function(copy)
    except AttributeError as e:
        if type_of_array_in_cpp == "vector" and copy == False and rank >1:
            pass # correctly raises error since multiple c++ vectors are not memory contiguous

        elif type_of_array_in_cpp == "vector" and copy == False and rank==1 and typestr == "bool":
            expected_error_msg = "Cannot share memory for std::vector<bool>, use int8_t or copy=true."
            error_msg = str(e)
            if error_msg != error_msg:
                raise Exception("expected "+expected_error_msg+" but got "+error_msg)
            pass
        else:
            raise AttributeError from e
