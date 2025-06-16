"""
Copyright (c) 2024 Luis Bernardos. All rights reserved.

noder: Node Utility for Data Operations
"""
import sys
import os
import glob

from ._version import version as __version__

__all__ = ["__version__"]


# def find_hdf5_dll_dir():
#     for path in os.environ["PATH"].split(";"):
#         path = path.strip()
#         if os.path.isdir(path) and glob.glob(os.path.join(path, "hdf5*.dll")):
#             return path 

# if sys.platform == "win32":
#     os.add_dll_directory(os.path.dirname(__file__))

#     hdf5_lib_dir = find_hdf5_dll_dir()
#     if hdf5_lib_dir: os.add_dll_directory(hdf5_lib_dir)


from .core import registerDefaultFactory, factory
registerDefaultFactory()

def zeros(*args,**kwargs):
    kwargs.setdefault("dtype","double")
    builder = getattr(factory,f"zeros_{kwargs['dtype']}")
    kwargs.pop("dtype")
    return builder(*args,**kwargs)

factory.zeros = zeros