from ..core import Array as arrayInCpp
from . import data_types

import numpy as np

class Array(arrayInCpp):
    """
    Array class holding numerical data, with associated methods.
    
    ..note:: 
        This class inherits from C++ Array class, which wraps
        pybind11::array class, a C++ view of numpy.ndarray
    
    """

    def __init__(self, data=None):
        """
        Initialize the Array object.

        Parameters
        ----------
        
            data: :py:obj:`None` or :py:class:`numpy.ndarray`

        """
        if data is None:
            super().__init__()

        elif isinstance(data, np.ndarray):
            super().__init__(data)

        else:
            raise ValueError("Unsupported input type for Array. Expected None or numpy array.")

    def is_none(self):
        """
        Returns True if the array is None, False otherwise.
        """
        return arrayInCpp.isNone()

    def has_string(self):
        """
        Returns True if the array has a string, False otherwise.
        """
        return super().hasString()

    def is_scalar(self):
        """
        Returns True if the array is a scalar, False otherwise.
        """
        return super().isScalar()

    def is_contiguous(self):
        """
        Returns True if the array is contiguous in memory, False otherwise.
        """
        return super().isContiguous()

    def is_contiguous_in_style_c(self):
        """
        Returns True if the array is contiguous in memory following style C/C++.
        False otherwise. C-style (a.k.a C-order or row-major) states that the
        last dimension of a multi-dimensional array is contiguous.
        """
        return super().isContiguousInStyleC()

    def is_contiguous_in_style_fortran(self):
        """
        Returns True if the array is contiguous in memory following style Fortran.
        False otherwise. Fortran-style (a.k.a F-order or column-major) states
        that the last dimension of a multi-dimensional array is contiguous.
        """
        return super().isContiguousInStyleFortran()

    
    def extract_string(self):
        """
        Returns the string contained in the array. 
        Supports unicode strings (including emojis 🥳).
        """
        return super().extractString()


    def get_flat_index(indices):
        """
        Return the flat index of the array respecting its shape and order.
        
        For example, flat index in order 'C' for 3D matrix is calculated as:
        flat_index = i*Nj*Nk + j*Nk + k;

        and flat index in order 'F' for 3D matrix is calculated as:
        flat_index = i + j*Ni + k*Ni*Nj

        Parameters
        ----------

            indices : tuple, list, Array
                List of (i,j,k...) indices to transform into flat index

        Returns
        -------

            flat_index : int

        """
        return super().getFlatIndex(indices)