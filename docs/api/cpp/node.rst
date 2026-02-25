Node (C++)
==========

.. _cpp-node-class:

``Node`` Class
--------------

Primary hierarchical data structure implemented in ``include/node/node.hpp``.

Python counterpart: :py:class:`noder.core.Node`

.. _cpp-node-ctor:

Constructor
~~~~~~~~~~~

Signature: ``Node(const std::string& name = "", const std::string& type = "DataArray_t")``

Python counterpart: :py:class:`noder.core.Node`

.. _cpp-node-pick:

``pick``
~~~~~~~~

Signature: ``Navigation& pick()``

Python counterpart: :py:meth:`noder.core.Node.pick`

.. _cpp-node-name:

``name``
~~~~~~~~

Signature: ``const std::string& name() const``

Python counterpart: :py:meth:`noder.core.Node.name`

.. _cpp-node-setname:

``setName``
~~~~~~~~~~~

Signature: ``void setName(const std::string& name)``

Python counterpart: :py:meth:`noder.core.Node.set_name`

.. _cpp-node-type:

``type``
~~~~~~~~

Signature: ``std::string type() const``

Python counterpart: :py:meth:`noder.core.Node.type`

.. _cpp-node-settype:

``setType``
~~~~~~~~~~~

Signature: ``void setType(const std::string& type)``

Python counterpart: :py:meth:`noder.core.Node.set_type`

.. _cpp-node-data:

``data``
~~~~~~~~

Signature: ``const Data& data() const``

Python counterpart: :py:meth:`noder.core.Node.data`

.. _cpp-node-setdata:

``setData``
~~~~~~~~~~~

Signatures include:

- ``void setData(std::shared_ptr<Data> d)``
- ``void setData(const Data& d)``
- scalar/string overloads in ``Node`` templates

Python counterpart: :py:meth:`noder.core.Node.set_data`

.. _cpp-node-children:

``children``
~~~~~~~~~~~~

Signature: ``const std::vector<std::shared_ptr<Node>>& children() const``

Python counterpart: :py:meth:`noder.core.Node.children`

.. _cpp-node-haschildren:

``hasChildren``
~~~~~~~~~~~~~~~

Signature: ``bool hasChildren() const``

Python counterpart: :py:meth:`noder.core.Node.has_children`

.. _cpp-node-parent:

``parent``
~~~~~~~~~~

Signature: ``std::weak_ptr<Node> parent() const``

Python counterpart: :py:meth:`noder.core.Node.parent`

.. _cpp-node-root:

``root``
~~~~~~~~

Signature: ``std::shared_ptr<const Node> root() const``

Python counterpart: :py:meth:`noder.core.Node.root`

.. _cpp-node-level:

``level``
~~~~~~~~~

Signature: ``size_t level() const``

Python counterpart: :py:meth:`noder.core.Node.level`

.. _cpp-node-position:

``position``
~~~~~~~~~~~~

Signature: ``size_t position() const``

Python counterpart: :py:meth:`noder.core.Node.position`

.. _cpp-node-detach:

``detach``
~~~~~~~~~~

Signature: ``void detach()``

Python counterpart: :py:meth:`noder.core.Node.detach`

.. _cpp-node-attachto:

``attachTo``
~~~~~~~~~~~~

Signature: ``void attachTo(std::shared_ptr<Node> node, const int16_t& position = -1, bool overrideSiblingByName = true)``

Python counterpart: :py:meth:`noder.core.Node.attach_to`

.. _cpp-node-addchild:

``addChild``
~~~~~~~~~~~~

Signature: ``void addChild(std::shared_ptr<Node> node, bool overrideSiblingByName = true, const int16_t& position = -1)``

Python counterpart: :py:meth:`noder.core.Node.add_child`

.. _cpp-node-addchildren:

``addChildren``
~~~~~~~~~~~~~~~

Signature: ``void addChildren(const std::vector<std::shared_ptr<Node>>& nodes, bool overrideSiblingByName = true)``

Python counterpart: :py:meth:`noder.core.Node.add_children`

.. _cpp-node-siblings:

``siblings``
~~~~~~~~~~~~

Signature: ``std::vector<std::shared_ptr<Node>> siblings(bool includeMyself=true) const``

Python counterpart: :py:meth:`noder.core.Node.siblings`

.. _cpp-node-hassiblings:

``hasSiblings``
~~~~~~~~~~~~~~~

Signature: ``bool hasSiblings() const``

Python counterpart: :py:meth:`noder.core.Node.has_siblings`

.. _cpp-node-getchildrennames:

``getChildrenNames``
~~~~~~~~~~~~~~~~~~~~

Signature: ``std::vector<std::string> getChildrenNames() const``

Python counterpart: :py:meth:`noder.core.Node.get_children_names`

.. _cpp-node-swap:

``swap``
~~~~~~~~

Signature: ``void swap(std::shared_ptr<Node> node)``

Python counterpart: :py:meth:`noder.core.Node.swap`

.. _cpp-node-copy:

``copy``
~~~~~~~~

Signature: ``std::shared_ptr<Node> copy(bool deep=false) const``

Python counterpart: :py:meth:`noder.core.Node.copy`

.. _cpp-node-getatpath:

``getAtPath``
~~~~~~~~~~~~~

Signature: ``std::shared_ptr<Node> getAtPath(const std::string& path, bool pathIsRelative=false) const``

Python counterpart: :py:meth:`noder.core.Node.get_at_path`

.. _cpp-node-path:

``path``
~~~~~~~~

Signature: ``std::string path() const``

Python counterpart: :py:meth:`noder.core.Node.path`

.. _cpp-node-descendants:

``descendants``
~~~~~~~~~~~~~~~

Signature: ``std::vector<std::shared_ptr<Node>> descendants()``

Python counterpart: :py:meth:`noder.core.Node.descendants`

.. _cpp-node-merge:

``merge``
~~~~~~~~~

Signature: ``void merge(std::shared_ptr<Node> node)``

Python counterpart: :py:meth:`noder.core.Node.merge`

.. _cpp-node-haslinktarget:

``hasLinkTarget``
~~~~~~~~~~~~~~~~~

Signature: ``bool hasLinkTarget() const``

Python counterpart: :py:meth:`noder.core.Node.has_link_target`

.. _cpp-node-linktargetfile:

``linkTargetFile``
~~~~~~~~~~~~~~~~~~

Signature: ``const std::string& linkTargetFile() const``

Python counterpart: :py:meth:`noder.core.Node.link_target_file`

.. _cpp-node-linktargetpath:

``linkTargetPath``
~~~~~~~~~~~~~~~~~~

Signature: ``const std::string& linkTargetPath() const``

Python counterpart: :py:meth:`noder.core.Node.link_target_path`

.. _cpp-node-setlinktarget:

``setLinkTarget``
~~~~~~~~~~~~~~~~~

Signature: ``void setLinkTarget(const std::string& targetFile, const std::string& targetPath)``

Python counterpart: :py:meth:`noder.core.Node.set_link_target`

.. _cpp-node-clearlinktarget:

``clearLinkTarget``
~~~~~~~~~~~~~~~~~~~

Signature: ``void clearLinkTarget()``

Python counterpart: :py:meth:`noder.core.Node.clear_link_target`

.. _cpp-node-getlinks:

``getLinks``
~~~~~~~~~~~~

Signature: ``std::vector<std::tuple<std::string, std::string, std::string, std::string, int>> getLinks() const``

Python counterpart: :py:meth:`noder.core.Node.get_links`

.. _cpp-node-reloadnodedata:

``reloadNodeData``
~~~~~~~~~~~~~~~~~~

Signature: ``void reloadNodeData(const std::string& filename)``

Python counterpart: :py:meth:`noder.core.Node.reload_node_data`

.. _cpp-node-savethisnodeonly:

``saveThisNodeOnly``
~~~~~~~~~~~~~~~~~~~~

Signature: ``void saveThisNodeOnly(const std::string& filename, const std::string& backend = "hdf5")``

Python counterpart: :py:meth:`noder.core.Node.save_this_node_only`

.. _cpp-node-setparameters:

``setParameters``
~~~~~~~~~~~~~~~~~

Signature:

``std::shared_ptr<Node> setParameters(const std::string& containerName, const ParameterValue::DictEntries& parameters, const std::string& containerType = "UserDefinedData_t", const std::string& parameterType = "DataArray_t")``

Python counterpart: :py:meth:`noder.core.Node.set_parameters`

.. _cpp-node-getparameters:

``getParameters``
~~~~~~~~~~~~~~~~~

Signature: ``ParameterValue getParameters(const std::string& containerName) const``

Python counterpart: :py:meth:`noder.core.Node.get_parameters`

.. _cpp-node-printtree:

``printTree``
~~~~~~~~~~~~~

Signature:

``std::string printTree(int max_depth=9999, std::string highlighted_path="", int depth=0, bool last_pos=false, std::string markers="") const``

Python counterpart: :py:meth:`noder.core.Node.print_tree`

.. _cpp-node-write:

``write`` (HDF5 builds)
~~~~~~~~~~~~~~~~~~~~~~~

Signature: ``void write(const std::string& filename)``

Python counterpart: :py:meth:`noder.core.Node.write`

