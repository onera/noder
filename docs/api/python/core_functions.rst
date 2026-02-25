Core Functions (Python)
=======================

.. currentmodule:: noder.core

This page documents top-level Python entry points exposed by ``noder.core``.

``registerDefaultFactory``
--------------------------

.. autofunction:: registerDefaultFactory

Purpose
~~~~~~~
Initializes default payload factory behavior used by ``Node`` construction.

Arguments
~~~~~~~~~
None.

Output
~~~~~~
``None``

Example
~~~~~~~

.. literalinclude:: ../../../tests/python/node/test_node_factory.py
   :language: python
   :pyobject: test_new_node_no_args

``new_node``
------------

.. autofunction:: new_node

Purpose
~~~~~~~
Constructs a node and optionally attaches it to a parent.

Arguments
~~~~~~~~~
``name`` (str), ``type`` (str), ``data`` (Any), ``parent`` (Node or None).

Output
~~~~~~
``Node``

Example
~~~~~~~

.. literalinclude:: ../../../tests/python/node/test_node_factory.py
   :language: python
   :pyobject: test_new_node_parent

``nodeToPyCGNS``
----------------

.. autofunction:: nodeToPyCGNS

Purpose
~~~~~~~
Converts a ``Node`` tree into a CGNS-like Python nested list.

Arguments
~~~~~~~~~
``node`` (Node).

Output
~~~~~~
``list``

Example
~~~~~~~

.. literalinclude:: ../../../tests/python/io/test_node_pycgns.py
   :language: python
   :pyobject: test_nodeToPyCGNS_tree

``pyCGNSToNode``
----------------

.. autofunction:: pyCGNSToNode

Purpose
~~~~~~~
Converts a CGNS-like Python nested list into a ``Node`` tree.

Arguments
~~~~~~~~~
``py_list`` (list).

Output
~~~~~~
``Node``

Example
~~~~~~~

.. literalinclude:: ../../../tests/python/io/test_node_pycgns.py
   :language: python
   :pyobject: test_pyCGNSToNode_tree
