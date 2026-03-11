"""
CGNS-focused Python entry points.
"""

from . import core as _core
from ._cgns import Zone

__all__ = ["Zone"]
