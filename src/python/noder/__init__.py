"""
Copyright (c) 2026 ONERA. All rights reserved.

noder: Node-Oriented Data Extraction & Representation
"""
import sys
import os
import glob

from ._version import version as __version__

__all__ = ["__version__"]




from .core import (
    registerDefaultFactory,
    factory,
    Node,
    Array)
registerDefaultFactory()

def zeros(*args,**kwargs):
    kwargs.setdefault("dtype","double")
    builder = getattr(factory,f"zeros_{kwargs['dtype']}")
    kwargs.pop("dtype")
    return builder(*args,**kwargs)

factory.zeros = zeros

# obsolete if using bundling:
# def find_hdf5_dll_dir():
#     for path in os.environ["PATH"].split(";"):
#         path = path.strip()
#         if os.path.isdir(path) and glob.glob(os.path.join(path, "hdf5*.dll")):
#             return path 

# if sys.platform == "win32":
#     os.add_dll_directory(os.path.dirname(__file__))

#     hdf5_lib_dir = find_hdf5_dll_dir()
#     if hdf5_lib_dir: os.add_dll_directory(hdf5_lib_dir)