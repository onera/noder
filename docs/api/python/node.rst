Node and navigation bindings
============================

The object-oriented tree interface and its navigation helpers are exposed
from the compiled :mod:`noder.core` module.

.. autoclass:: noder.core.Node
   :members:
   :special-members: __init__, __add__, __truediv__
   :show-inheritance:

.. autoclass:: noder.core.Navigation
   :members:
   :show-inheritance:

.. autoclass:: noder.core._NodeGroup
   :members: nodes
   :show-inheritance:

.. autofunction:: noder.core.new_node

.. autofunction:: noder.core.registerDefaultFactory

Node utilities
--------------

.. autofunction:: noder.cgns.add

.. autofunction:: noder.cgns.merge

CGNS/Python conversion
----------------------

.. autofunction:: noder.core.pyCGNSToNode

.. autofunction:: noder.core.nodeToPyCGNS
