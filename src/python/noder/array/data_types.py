floating_types = ["float","double"]
positive_integral_types = ["uint8","uint16","uint32","uint64"]
signed_integral_types = ["int8", "int16", "int32", "int64"]
integral_types = positive_integral_types + signed_integral_types
floating_and_integral_types = integral_types + floating_types
scalar_types = floating_and_integral_types + ["bool"]
string_and_integral = ["str"] + scalar_types 

to_numpy_dtype = {
    "uint8": "uint8",
    "uint16": "uint16",
    "uint32": "uint32",
    "uint64": "uint64",
    "int8": "int8",
    "int16": "int16",
    "int32": "int32",
    "int64": "int64",
    "float": "float32",
    "double": "float64",
    "bool": "bool"
}

def raise_error_if_invalid_dtype(dtype, allowed_dtypes=scalar_types):

    if not isinstance(dtype, str):
        raise TypeError(f"dtype must be a string, not {type(dtype)}")
    
    elif dtype not in allowed_dtypes:
        raise ValueError(f"Invalid dtype: {dtype}, must be one of {allowed_dtypes}")


def raise_error_if_invalid_order(order):

    if not isinstance(order, str):
        raise TypeError(f"order must be a string, not {type(order)}")
    
    elif order not in ["C", "F"]:
        raise ValueError(f"Invalid order: {order}, must be 'C' or 'F'")