Data (Python)
=============

.. currentmodule:: noder.core

Abstract payload interface returned by :py:meth:`noder.core.Node.data`.

C++ counterpart: :doc:`../cpp/data`

Class
-----

.. autoclass:: Data
   :member-order: bysource

Methods
---------------

``hasString``
~~~~~~~~~~~~~

.. automethod:: Data.hasString

.. literalinclude:: ../../../tests/python/node/test_data.py
   :language: python
   :start-after: # docs:start data_has_string_example
   :end-before: # docs:end data_has_string_example
   :dedent: 4

``isNone``
~~~~~~~~~~

.. automethod:: Data.isNone

.. literalinclude:: ../../../tests/python/node/test_data.py
   :language: python
   :start-after: # docs:start data_is_none_example
   :end-before: # docs:end data_is_none_example
   :dedent: 4

``isScalar``
~~~~~~~~~~~~

.. automethod:: Data.isScalar

.. literalinclude:: ../../../tests/python/node/test_data.py
   :language: python
   :start-after: # docs:start data_is_scalar_example
   :end-before: # docs:end data_is_scalar_example
   :dedent: 4

``extractString``
~~~~~~~~~~~~~~~~~

.. automethod:: Data.extractString

.. literalinclude:: ../../../tests/python/node/test_data.py
   :language: python
   :start-after: # docs:start data_extract_string_example
   :end-before: # docs:end data_extract_string_example
   :dedent: 4
