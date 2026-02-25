# include "test_node.hpp"

#ifdef ENABLE_HDF5_IO
#include "io/io.hpp"
#endif

using namespace std::string_literals;

namespace {

const ParameterValue* findEntryByName(const ParameterValue::DictEntries& entries, const std::string& key) {
    for (const auto& [entryKey, entryValue] : entries) {
        if (entryKey == key) {
            return &entryValue;
        }
    }
    return nullptr;
}

} // namespace

void test_init() {
    auto a = newNode("a");
    auto b = newNode("b", "type_t");
}

void test_name() {
    auto a = newNode("a");
    std::string name = a->name();
    if ( name != std::string("a") ) throw py::value_error("did not get name 'a' as string");
    if ( name != "a" ) throw py::value_error("did not get name 'a' as chars");
}

void test_setName() {
    auto a = newNode("a");
    a->setName("b");
    std::string name = a->name();
    if ( name != "b" ) throw py::value_error("did not get name 'b'");
}

void test_type() {
    auto a = newNode("a", "type_t");
    std::string node_type = a->type();
    if ( node_type != "type_t" ) throw py::value_error("did not get type 'type_t'");
}

void test_setType() {
    auto a = newNode("a");
    a->setType("NewType_t");
    std::string node_type = a->type();
    if ( node_type != "NewType_t" ) throw py::value_error("did not get type 'NewType_t'");
}

void test_binding_setNameAndTypeFromPython(Node& node) {
    node.setName("NewName");
    node.setType("NewType");
}

void test_noData() {
    auto a = newNode();
    if (!a->noData()) throw py::value_error("expected no data");
}

void test_children_empty() {
    auto a = newNode("a");
    auto children = a->children();
    if (children.size() != 0) throw std::runtime_error("failed C++ empty children children");
}

void test_parent_empty() {
    auto a = newNode("a");
    auto parent = a->parent().lock();
    if ( parent ) throw py::value_error("did not get null parent");
}

void test_root_itself() {
    auto a = newNode("a");
    auto b = a->root();
    if ( b.get() != a.get() ) throw py::value_error("single node root does not point to itself");
}

void test_level_0() {
    auto a = newNode("a");
    size_t level = a->level();
    if ( level != 0 ) throw py::value_error("single node level is not 0");
}

void test_position_null() {
    auto a = newNode("a");
    size_t pos = a->position();
    if ( pos != 0 ) throw py::value_error("expected 0 for single node sibling pos");
}

void test_getPath_itself() {
    auto a = newNode("a");
    std::string path = a->path();
    if ( path != "a" ) throw py::value_error("expected single node path 'a'");
}

void test_attachTo() {

    auto a = newNode("a");
    auto b = newNode("b");
    b->attachTo(a);
    std::string expected_path_of_b = "a/b";
    if ( b->path() != expected_path_of_b ) throw py::value_error("expected path 'a/b'"+" but got "s + b->path());

}


void test_attachTo_multiple_levels() {
    size_t max_levels = 20;
    std::shared_ptr<Node> first_node = newNode("0");
    std::vector<std::shared_ptr<Node>> nodes = {first_node};
    std::vector<std::string> paths = {"0"};
    for (size_t i = 1; i < max_levels; i++)
    {
        std::shared_ptr<Node> node = newNode(std::to_string(i));
        std::shared_ptr<Node> previous_node = nodes[nodes.size()-1];
        node->attachTo(previous_node);
        nodes.push_back(node);
        paths.push_back(paths[paths.size()-1]+"/"+node->name());
        
        std::string path = node->path();
        if ( path != paths[paths.size()-1] ) throw py::value_error("path "+path+" did not match expected: "+paths[paths.size()-1]);

        auto children_of_parent = nodes[nodes.size()-2]->children();
        if ( children_of_parent.size() != 1 ) throw py::value_error("expected only 1 child for node "+node->name());
        if ( node != children_of_parent[0] ) throw py::value_error("expected node to be the child");
        
        auto parent = node->parent().lock();
        if ( parent.get() != nodes[nodes.size()-2].get() ) throw py::value_error("expected parent is before-last item of nodes vector");
    }
    nodes.clear();
}

void test_addChild() {
    auto a = newNode("a");
    auto b = newNode("b");
    a->addChild(b);
    std::string expected_path_of_b = "a/b";
    if ( b->path() != expected_path_of_b ) throw py::value_error("expected path 'a/b'");
}

void test_example_addChild() {
    // docs:start add_child_cpp_example
    # include <node/node.hpp>
    # include <node/node_factory.hpp>
    
    auto a = newNode("a");
    auto b = newNode("b");

    a->addChild(b); // add b to a

    const std::string expected_path_of_b = "a/b";
    if (b->path() != expected_path_of_b) {
        throw py::value_error("expected path 'a/b'");
    }
    // docs:end add_child_cpp_example
}

void test_addChildAsPointer() {
    auto a = newNode("a");

    std::shared_ptr<Node> b = newNode("b");
    a->addChild(b);

    std::string path_of_b = b->path();

    if ( path_of_b != "a/b" ) {
        throw py::value_error("expected path 'a/b'");
    }
}

void test_detach_0() {
    auto a = newNode("a");
    a->detach();
    if ( a->parent().lock() ) throw py::value_error("expected no parent after detachment");
}

void test_detach_1() {
    auto a = newNode("a");
    auto b = newNode("b");

    b->attachTo(a);
    b->detach();
    if ( b->parent().lock() ) throw py::value_error("expected no parent after detachment");

    auto children_of_a = a->children();
    if ( children_of_a.size() != 0 ) throw py::value_error("expected a to have no children");
    auto parent_of_b = b->parent().lock();
    if ( parent_of_b ) throw py::value_error("expected b to have no parent");
    std::string path_of_b = b->path();
    if ( path_of_b != "b" ) throw py::value_error("expected path of b to be exactly 'b'");
}

void test_detach_2() {
    size_t nb_of_children = 5;
    auto a = newNode("a");

    std::vector<std::shared_ptr<Node>> children;
    children.resize(nb_of_children);
    for (size_t i = 0; i < nb_of_children; i++) {
        children[i] = newNode(std::to_string(i));
        children[i]->attachTo(a);
    }
    auto children_of_a = a->children();
    auto another_view_of_children_of_a = a->children();
    if ( &children_of_a == &another_view_of_children_of_a ) {
        throw py::value_error("expected copy after calling children");
    }

    for (size_t i = 0; i < children_of_a.size(); i++) {
        if ( children_of_a[i].get() != another_view_of_children_of_a[i].get() ) {
            throw py::value_error("multiple children views don't points towards same object");
        }    
    }    


    size_t index_child_to_detach = 2;
    auto child_to_detach = children_of_a[index_child_to_detach];
    child_to_detach->detach();
    if ( a->children().size() != nb_of_children - 1 ) {
        throw py::value_error("children of a was not updated correctly after detachment");
    }
    
    size_t i = 0;
    for (auto node: a->children()) {
        if ( i < index_child_to_detach ) {
            if ( node->name() != std::to_string(i) ) throw py::value_error("wrong name, expected "+std::to_string(i));
        } else {
            if ( node->name() != std::to_string(i+1) ) throw py::value_error("wrong name, expected "+std::to_string(i+1));
        }
        i += 1;
    }
}

void test_detach_3() {
    size_t max_levels = 20;
    std::vector<std::shared_ptr<Node>> nodes;
    nodes.resize(max_levels);
    nodes[0] = newNode("0");
    std::stringstream expected_path;
    expected_path << 0;
    for (size_t i = 1; i < max_levels; i++) {
        nodes[i] = newNode(std::to_string(i));
        nodes[i]->attachTo(nodes[i-1]);
        auto children_of_parent = nodes[i-1]->children();

        if ( children_of_parent.size() != 1 ) throw py::value_error("expected last parent to have 1 child");
        if ( nodes[i].get() != children_of_parent[0].get()) throw py::value_error("expected ptr to itself");
        auto parent = nodes[i]->parent().lock();
        if ( parent.get() != nodes[i-1].get()) throw py::value_error("expected parent to be previous node");
        expected_path << "/" << i;
        if ( nodes[i]->path() != expected_path.str() ) throw py::value_error("did not match the expected path");
    }

    size_t index_child_to_detach = 5;
    if ( index_child_to_detach >= max_levels ) throw py::value_error("requested detach index must be lower than max_levels");
    auto child_to_detach = nodes[index_child_to_detach];
    child_to_detach->detach();
    std::string last_child_path = nodes[max_levels-1]->path();
    expected_path.str(std::string());
    expected_path.clear();
    expected_path << index_child_to_detach;
    for (size_t i = index_child_to_detach+1; i < max_levels; i++) {
        expected_path << "/" << i;
    }
    if ( last_child_path != expected_path.str() ) {
        std::string msg = "got "+last_child_path+" instead of: "+expected_path.str();
        throw py::value_error(msg);
    }
}

void test_delete_multiple_descendants() {
    size_t max_levels = 20;
    std::vector<std::shared_ptr<Node>> nodes;
    nodes.reserve(max_levels);
    nodes.emplace_back(newNode(std::to_string(0)));
    for (size_t i = 1; i < max_levels; i++) {
        nodes.emplace_back(newNode(std::to_string(i)));
        nodes[i]->attachTo(nodes[i-1]);
    }
    
    size_t index_node_to_delete = 3;
    nodes[index_node_to_delete]->detach();
    nodes.erase(nodes.begin()+static_cast<ssize_t>(index_node_to_delete));
}


void test_getPath() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    b->attachTo(a);
    c->attachTo(b);
    std::string expected_path_of_c = "a/b/c";
    if (c->path() != expected_path_of_c) {
        throw py::value_error("was expecting path 'a/b/c'");
    }
}

void test_root() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");
    auto e = newNode("e");

    a->addChild(b);
    b->addChild(c);
    c->addChild(d);
    d->addChild(e);

    if (e->root().get() != a.get()) throw py::value_error("root expected top node");
}

void test_level() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");
    auto e = newNode("e");

    a->addChild(b);
    b->addChild(c);
    c->addChild(d);
    d->addChild(e);

    if (e->level() != 4) throw py::value_error("expected level 4");
}

void test_printTree() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");
    auto e = newNode("e");
    auto f = newNode("f");
    auto g = newNode("g");
    auto h = newNode("h");
    auto i = newNode("i");

    a->addChild(b);
    b->addChild(c);
    c->addChild(d);
    d->addChild(e);

    f->addChild(g);
    g->addChild(h);
    f->addChild(i);

    f->attachTo(b);

    std::cout << g;
}

void test_children() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    b->attachTo(a);
    c->attachTo(a);

    if (a->children().size() != 2) {
        throw py::value_error("expected 2 children");
    }
    
    std::vector<std::string> expected_children_paths = {"a/b", "a/c"};
    
    size_t i = 0;
    for (auto child: a->children()) {
        if (child->path() != expected_children_paths[i]) {
            throw py::value_error("expected path "+expected_children_paths[i]);
        }
        i += 1;
    }
}


void test_binding_addChildrenFromPython(std::shared_ptr<Node> node) {
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");

    b->attachTo(node);
    c->attachTo(b);
    d->attachTo(node);
}

void test_position() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");

    b->attachTo(a);
    c->attachTo(a);
    d->attachTo(a);

    if (a->position() != 0) throw py::value_error("expected pos 0 for node a");
    if (b->position() != 0) throw py::value_error("expected pos 0 for node b");
    if (c->position() != 1) throw py::value_error("expected pos 1 for node c");
    if (d->position() != 2) throw py::value_error("expected pos 2 for node d");
}

void test_hasChildren() {
    auto a = newNode("a");
    if (a->hasChildren()) throw py::value_error("expected no children");

    auto b = newNode("b");
    a->addChild(b);

    if (!a->hasChildren()) throw py::value_error("expected children");
    if (b->hasChildren()) throw py::value_error("expected no children on leaf");
}

void test_siblings() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");

    a->addChildren({b, c, d});

    auto allSiblings = c->siblings();
    if (allSiblings.size() != 3) throw py::value_error("expected 3 siblings including self");

    auto siblingsWithoutSelf = c->siblings(false);
    if (siblingsWithoutSelf.size() != 2) throw py::value_error("expected 2 siblings excluding self");
    if (siblingsWithoutSelf[0]->name() != "b") throw py::value_error("expected sibling b");
    if (siblingsWithoutSelf[1]->name() != "d") throw py::value_error("expected sibling d");
}

void test_hasSiblings() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");

    if (a->hasSiblings()) throw py::value_error("root should not have siblings");

    a->addChild(b);
    if (b->hasSiblings()) throw py::value_error("single child should not have siblings");

    a->addChild(c);
    if (!b->hasSiblings()) throw py::value_error("expected siblings after second child");
}

void test_getChildrenNames() {
    auto a = newNode("a");
    a->addChildren({newNode("b"), newNode("c"), newNode("d")});

    auto names = a->getChildrenNames();
    if (names.size() != 3) throw py::value_error("expected 3 child names");
    if (names[0] != "b") throw py::value_error("expected child name b");
    if (names[1] != "c") throw py::value_error("expected child name c");
    if (names[2] != "d") throw py::value_error("expected child name d");
}

void test_addChildren() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");

    a->addChildren({b, c, d});

    if (a->children().size() != 3) throw py::value_error("expected three children");
    if (b->path() != "a/b") throw py::value_error("expected path a/b");
    if (c->path() != "a/c") throw py::value_error("expected path a/c");
    if (d->path() != "a/d") throw py::value_error("expected path a/d");
}

void test_overrideSiblingByName_attachTo() {
    auto parent = newNode("parent");
    auto oldChild = newNode("child");
    auto incoming = newNode("child");
    oldChild->attachTo(parent);

    incoming->attachTo(parent, -1, true);
    if (parent->children().size() != 1) throw py::value_error("override=true should keep one child");
    if (parent->children()[0].get() != incoming.get()) throw py::value_error("incoming child should replace old child");
    if (oldChild->parent().lock()) throw py::value_error("replaced child should be detached");
    if (oldChild->path() != "child") throw py::value_error("replaced child path should be reset");

    auto parent2 = newNode("parent2");
    auto first = newNode("child");
    auto second = newNode("child");
    first->attachTo(parent2);
    second->attachTo(parent2, -1, false);

    if (parent2->children().size() != 2) throw py::value_error("override=false should keep both children");
    if (second->name() != "child.0") throw py::value_error("incoming child should be renamed to child.0");
    if (parent2->children()[0]->name() != "child") throw py::value_error("expected original child name");
    if (parent2->children()[1]->name() != "child.0") throw py::value_error("expected renamed incoming child");
}

void test_overrideSiblingByName_addChild() {
    auto parent = newNode("parent");
    parent->addChild(newNode("x"));
    auto conflict = newNode("x");
    parent->addChild(conflict, false);
    if (conflict->name() != "x.0") throw py::value_error("addChild override=false should rename x to x.0");

    auto replaceParent = newNode("replaceParent");
    auto oldNode = newNode("x");
    auto newNodeSameName = newNode("x");
    replaceParent->addChild(oldNode);
    replaceParent->addChild(newNodeSameName, true);
    if (replaceParent->children().size() != 1) throw py::value_error("addChild override=true should replace");
    if (replaceParent->children()[0].get() != newNodeSameName.get()) throw py::value_error("expected replacement child");
}

void test_overrideSiblingByName_addChildren() {
    auto noOverride = newNode("parent");
    noOverride->addChildren({newNode("n"), newNode("n"), newNode("n")}, false);
    auto names = noOverride->getChildrenNames();
    if (names.size() != 3) throw py::value_error("override=false should keep all children");
    if (names[0] != "n") throw py::value_error("expected n");
    if (names[1] != "n.0") throw py::value_error("expected n.0");
    if (names[2] != "n.1") throw py::value_error("expected n.1");

    auto override = newNode("parent2");
    override->addChildren({newNode("n"), newNode("n"), newNode("n")}, true);
    if (override->children().size() != 1) throw py::value_error("override=true should keep last sibling only");
    if (override->children()[0]->name() != "n") throw py::value_error("expected final child to keep base name n");
}

void test_swap() {
    auto parentLeft = newNode("left");
    auto parentRight = newNode("right");
    auto b = newNode("b");
    auto c = newNode("c");

    parentLeft->addChild(b);
    parentRight->addChild(c);

    b->swap(c);

    if (b->parent().lock().get() != parentRight.get()) throw py::value_error("b should move to right");
    if (c->parent().lock().get() != parentLeft.get()) throw py::value_error("c should move to left");
    if (b->path() != "right/b") throw py::value_error("unexpected path for b after swap");
    if (c->path() != "left/c") throw py::value_error("unexpected path for c after swap");
}

void test_copy() {
    auto a = newNode("a");
    a->setData(3.14);
    auto b = newNode("b");
    b->setData(7);
    a->addChild(b);

    auto shallow = a->copy(false);
    if (shallow->name() != "a") throw py::value_error("shallow copy name mismatch");
    if (shallow->children().size() != 1) throw py::value_error("shallow copy should include children");
    if (shallow->dataPtr().get() != a->dataPtr().get()) throw py::value_error("shallow copy should share data");
    if (shallow->children()[0]->dataPtr().get() != b->dataPtr().get()) throw py::value_error("shallow child should share data");

    auto deep = a->copy(true);
    if (deep->dataPtr().get() == a->dataPtr().get()) throw py::value_error("deep copy should clone root data");
    if (deep->children()[0]->dataPtr().get() == b->dataPtr().get()) throw py::value_error("deep copy should clone child data");
}

void test_getAtPath() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");

    a->addChild(b);
    b->addChild(c);
    c->addChild(d);

    auto fromRoot = d->getAtPath("a/b/c");
    if (!fromRoot || fromRoot.get() != c.get()) throw py::value_error("expected to find c from absolute path");

    auto fromRelative = b->getAtPath("c/d", true);
    if (!fromRelative || fromRelative.get() != d.get()) throw py::value_error("expected to find d from relative path");

    auto missing = a->getAtPath("a/not_found");
    if (missing) throw py::value_error("expected null for missing path");
}

void test_getLinks() {
    auto root = newNode("root");
    auto target = newNode("target");
    auto localLink = newNode("local_link");
    auto externalLink = newNode("external_link");

    root->addChildren({target, localLink, externalLink});

    localLink->setLinkTarget(".", "/root/target");
    externalLink->setLinkTarget("external.cgns", "/ExternalRoot/Node");

    auto links = root->getLinks();
    if (links.size() != 2) {
        throw py::value_error("expected exactly two links");
    }

    const auto& [cwd0, file0, path0, target0, flag0] = links[0];
    if (cwd0 != ".") throw py::value_error("expected first link current-directory marker '.'");
    if (file0 != ".") throw py::value_error("expected first link target file '.'");
    if (path0 != "/root/local_link") throw py::value_error("unexpected first link node path");
    if (target0 != "/root/target") throw py::value_error("unexpected first link target path");
    if (flag0 != 5) throw py::value_error("unexpected first link flag");

    const auto& [cwd1, file1, path1, target1, flag1] = links[1];
    if (cwd1 != ".") throw py::value_error("expected second link current-directory marker '.'");
    if (file1 != "external.cgns") throw py::value_error("unexpected second link target file");
    if (path1 != "/root/external_link") throw py::value_error("unexpected second link node path");
    if (target1 != "/ExternalRoot/Node") throw py::value_error("unexpected second link target path");
    if (flag1 != 5) throw py::value_error("unexpected second link flag");
}

void test_setParametersAndGetParameters() {
    auto root = newNode("root");

    ParameterValue::DictEntries nested;
    nested.emplace_back("x", ParameterValue::makeData(std::make_shared<Array>(int32_t(1))));

    ParameterValue::DictEntries item0;
    item0.emplace_back("a", ParameterValue::makeData(std::make_shared<Array>(int32_t(7))));

    ParameterValue::DictEntries item1;
    item1.emplace_back("b", ParameterValue::makeData(std::make_shared<Array>(int32_t(9))));

    ParameterValue::ListEntries listItems;
    listItems.push_back(ParameterValue::makeDict(std::move(item0)));
    listItems.push_back(ParameterValue::makeDict(std::move(item1)));

    ParameterValue::DictEntries parameters;
    parameters.emplace_back("scalar", ParameterValue::makeData(std::make_shared<Array>(double(3.5))));
    parameters.emplace_back("none_like", ParameterValue::makeNull());
    parameters.emplace_back("nested", ParameterValue::makeDict(std::move(nested)));
    parameters.emplace_back("items", ParameterValue::makeList(std::move(listItems)));

    auto container = root->setParameters("Params", parameters);
    if (!container) {
        throw py::value_error("setParameters should return the created container");
    }
    if (container->name() != "Params") {
        throw py::value_error("setParameters returned unexpected container name");
    }
    if (container->type() != "UserDefinedData_t") {
        throw py::value_error("setParameters did not set default container type");
    }

    ParameterValue output = root->getParameters("Params");
    if (output.kind != ParameterValue::Kind::Dict) {
        throw py::value_error("getParameters should return dict for regular parameter container");
    }

    const ParameterValue* scalar = findEntryByName(output.dictEntries, "scalar");
    if (!scalar || scalar->kind != ParameterValue::Kind::Data || !scalar->data || !(*scalar->data == 3.5)) {
        throw py::value_error("scalar parameter mismatch");
    }

    const ParameterValue* noneLike = findEntryByName(output.dictEntries, "none_like");
    if (!noneLike || noneLike->kind != ParameterValue::Kind::Null) {
        throw py::value_error("none-like parameter should round-trip as null");
    }

    const ParameterValue* nestedOut = findEntryByName(output.dictEntries, "nested");
    if (!nestedOut || nestedOut->kind != ParameterValue::Kind::Dict) {
        throw py::value_error("nested parameter should round-trip as dict");
    }
    const ParameterValue* nestedX = findEntryByName(nestedOut->dictEntries, "x");
    if (!nestedX || nestedX->kind != ParameterValue::Kind::Data || !nestedX->data || !(*nestedX->data == 1)) {
        throw py::value_error("nested leaf parameter mismatch");
    }

    const ParameterValue* itemsOut = findEntryByName(output.dictEntries, "items");
    if (!itemsOut || itemsOut->kind != ParameterValue::Kind::List || itemsOut->listEntries.size() != 2) {
        throw py::value_error("list[dict] parameter mismatch");
    }
}

void test_getParametersMixedListAndDictFallback() {
    auto root = newNode("root");

    ParameterValue::DictEntries base;
    base.emplace_back("scalar", ParameterValue::makeData(std::make_shared<Array>(int32_t(2))));
    auto container = root->setParameters("Params", base);

    ParameterValue::DictEntries listItem;
    listItem.emplace_back("entry", ParameterValue::makeData(std::make_shared<Array>(int32_t(4))));
    container->setParameters("_list_.0", listItem);

    ParameterValue output = root->getParameters("Params");
    if (output.kind != ParameterValue::Kind::List) {
        throw py::value_error("mixed list+dict fallback should return a list");
    }
    if (output.listEntries.size() != 2) {
        throw py::value_error("mixed list+dict fallback should return [list..., dict]");
    }
    if (output.listEntries[0].kind != ParameterValue::Kind::Dict) {
        throw py::value_error("first mixed fallback item should be dict from _list_ entry");
    }
    if (output.listEntries[1].kind != ParameterValue::Kind::Dict) {
        throw py::value_error("last mixed fallback item should be dict of regular entries");
    }
    const ParameterValue* scalar = findEntryByName(output.listEntries[1].dictEntries, "scalar");
    if (!scalar || scalar->kind != ParameterValue::Kind::Data || !scalar->data || !(*scalar->data == 2)) {
        throw py::value_error("mixed fallback dict did not preserve scalar entry");
    }
}

#ifdef ENABLE_HDF5_IO
void test_reloadNodeData(const std::string& filename) {
    auto root = newNode("root");
    auto valueNode = newNode("value");
    valueNode->setData(42);
    valueNode->attachTo(root);

    root->write(filename);

    valueNode->setData(-7);
    valueNode->reloadNodeData(filename);

    if (!(valueNode->data() == 42)) {
        throw py::value_error("reloadNodeData did not restore persisted scalar value");
    }
}

void test_saveThisNodeOnly(const std::string& filename) {
    auto root = newNode("root");
    auto mutableNode = newNode("mutable");
    auto stableNode = newNode("stable");

    mutableNode->setData(1);
    stableNode->setData(2);
    root->addChildren({mutableNode, stableNode});
    root->write(filename);

    mutableNode->setData(99);
    mutableNode->saveThisNodeOnly(filename);

    auto readRoot = io::read(filename);
    auto persistedMutable = readRoot->getAtPath("root/mutable");
    auto persistedStable = readRoot->getAtPath("root/stable");

    if (!persistedMutable) {
        throw py::value_error("saveThisNodeOnly: could not read back node root/mutable");
    }
    if (!persistedStable) {
        throw py::value_error("saveThisNodeOnly: could not read back node root/stable");
    }
    if (!(persistedMutable->data() == 99)) {
        throw py::value_error("saveThisNodeOnly did not persist modified node data");
    }
    if (!(persistedStable->data() == 2)) {
        throw py::value_error("saveThisNodeOnly unexpectedly modified sibling node data");
    }
}
#endif

void test_merge() {
    auto left = newNode("Root");
    auto leftA = newNode("A");
    auto leftAX = newNode("X");
    leftA->addChild(leftAX);
    left->addChild(leftA);

    auto right = newNode("Root");
    auto rightA = newNode("A");
    auto rightAY = newNode("Y");
    auto rightB = newNode("B");
    rightA->addChild(rightAY);
    right->addChildren({rightA, rightB});

    left->merge(right);

    auto names = left->getChildrenNames();
    if (names.size() != 2) throw py::value_error("expected children A and B after merge");
    if (names[0] != "A") throw py::value_error("expected first child A after merge");
    if (names[1] != "B") throw py::value_error("expected second child B after merge");

    auto mergedA = left->children()[0];
    auto mergedANames = mergedA->getChildrenNames();
    if (mergedANames.size() != 2) throw py::value_error("expected A to contain X and Y after merge");
    if (mergedANames[0] != "X") throw py::value_error("expected first merged child X");
    if (mergedANames[1] != "Y") throw py::value_error("expected second merged child Y");
}

void test_Node_example() {
    // docs:start init_cpp_example
    auto node = newNode("zone", "Zone_t");
    if (node->name() != "zone") throw py::value_error("expected name zone");
    if (node->type() != "Zone_t") throw py::value_error("expected type Zone_t");
    // docs:end init_cpp_example
}

void test_pick_example() {
    // docs:start pick_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    root->addChild(child);
    if (root->pick().byName("child").get() != child.get()) throw py::value_error("pick.byName failed");
    // docs:end pick_cpp_example
}

void test_name_example() {
    // docs:start name_cpp_example
    auto node = newNode("zone");
    if (node->name() != "zone") throw py::value_error("expected name zone");
    // docs:end name_cpp_example
}

void test_setName_example() {
    // docs:start setName_cpp_example
    auto node = newNode("zone");
    node->setName("zone_updated");
    if (node->name() != "zone_updated") throw py::value_error("setName failed");
    // docs:end setName_cpp_example
}

void test_type_example() {
    // docs:start type_cpp_example
    auto node = newNode("zone", "Zone_t");
    if (node->type() != "Zone_t") throw py::value_error("expected type Zone_t");
    // docs:end type_cpp_example
}

void test_setType_example() {
    // docs:start setType_cpp_example
    auto node = newNode("zone");
    node->setType("UserDefinedData_t");
    if (node->type() != "UserDefinedData_t") throw py::value_error("setType failed");
    // docs:end setType_cpp_example
}

void test_data_example() {
    // docs:start data_cpp_example
    auto node = newNode("values");
    node->setData(42);
    if (!(node->data() == 42)) throw py::value_error("data accessor failed");
    // docs:end data_cpp_example
}

void test_setData_example() {
    // docs:start setData_cpp_example
    auto node = newNode("values");
    node->setData(3.14);
    if (!(node->data() == 3.14)) throw py::value_error("setData failed");
    // docs:end setData_cpp_example
}

void test_children_example() {
    // docs:start children_cpp_example
    auto root = newNode("root");
    root->addChildren({newNode("a"), newNode("b")});
    if (root->children().size() != 2) throw py::value_error("children size mismatch");
    // docs:end children_cpp_example
}

void test_hasChildren_example() {
    // docs:start hasChildren_cpp_example
    auto root = newNode("root");
    if (root->hasChildren()) throw py::value_error("expected no children");
    root->addChild(newNode("child"));
    if (!root->hasChildren()) throw py::value_error("expected children");
    // docs:end hasChildren_cpp_example
}

void test_parent_example() {
    // docs:start parent_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    root->addChild(child);
    if (child->parent().lock().get() != root.get()) throw py::value_error("parent mismatch");
    // docs:end parent_cpp_example
}

void test_root_example() {
    // docs:start root_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    auto leaf = newNode("leaf");
    root->addChild(child);
    child->addChild(leaf);
    if (leaf->root().get() != root.get()) throw py::value_error("root mismatch");
    // docs:end root_cpp_example
}

void test_level_example() {
    // docs:start level_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    auto leaf = newNode("leaf");
    root->addChild(child);
    child->addChild(leaf);
    if (leaf->level() != 2) throw py::value_error("level mismatch");
    // docs:end level_cpp_example
}

void test_position_example() {
    // docs:start position_cpp_example
    auto root = newNode("root");
    auto a = newNode("a");
    auto b = newNode("b");
    root->addChildren({a, b});
    if (a->position() != 0) throw py::value_error("expected first child at position 0");
    if (b->position() != 1) throw py::value_error("expected second child at position 1");
    // docs:end position_cpp_example
}

void test_detach_example() {
    // docs:start detach_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    root->addChild(child);
    child->detach();
    if (child->parent().lock()) throw py::value_error("child should be detached");
    if (!root->children().empty()) throw py::value_error("root should have no children");
    // docs:end detach_cpp_example
}

void test_attachTo_example() {
    // docs:start attachTo_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    child->attachTo(root);
    if (child->path() != "root/child") throw py::value_error("attachTo path mismatch");
    // docs:end attachTo_cpp_example
}

void test_addChild_example() {
    // docs:start addChild_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    root->addChild(child);
    if (child->path() != "root/child") throw py::value_error("addChild path mismatch");
    // docs:end addChild_cpp_example
}

void test_addChildren_example() {
    // docs:start addChildren_cpp_example
    auto root = newNode("root");
    root->addChildren({newNode("a"), newNode("b")});
    if (root->getChildrenNames() != std::vector<std::string>({"a", "b"})) {
        throw py::value_error("addChildren names mismatch");
    }
    // docs:end addChildren_cpp_example
}

void test_siblings_example() {
    // docs:start siblings_cpp_example
    auto root = newNode("root");
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    root->addChildren({a, b, c});
    auto siblings = b->siblings(false);
    if (siblings.size() != 2) throw py::value_error("siblings(false) mismatch");
    // docs:end siblings_cpp_example
}

void test_hasSiblings_example() {
    // docs:start hasSiblings_cpp_example
    auto root = newNode("root");
    auto a = newNode("a");
    auto b = newNode("b");
    root->addChildren({a, b});
    if (!a->hasSiblings()) throw py::value_error("expected siblings");
    // docs:end hasSiblings_cpp_example
}

void test_getChildrenNames_example() {
    // docs:start getChildrenNames_cpp_example
    auto root = newNode("root");
    root->addChildren({newNode("a"), newNode("b")});
    if (root->getChildrenNames() != std::vector<std::string>({"a", "b"})) {
        throw py::value_error("getChildrenNames mismatch");
    }
    // docs:end getChildrenNames_cpp_example
}

void test_swap_example() {
    // docs:start swap_cpp_example
    auto left = newNode("left");
    auto right = newNode("right");
    auto b = newNode("b");
    auto c = newNode("c");
    left->addChild(b);
    right->addChild(c);
    b->swap(c);
    if (b->path() != "right/b") throw py::value_error("swap failed for b");
    if (c->path() != "left/c") throw py::value_error("swap failed for c");
    // docs:end swap_cpp_example
}

void test_copy_example() {
    // docs:start copy_cpp_example
    auto root = newNode("root");
    root->setData(1.0);
    auto shallow = root->copy();
    auto deep = root->copy(true);
    if (shallow->dataPtr().get() != root->dataPtr().get()) throw py::value_error("shallow copy should share data");
    if (deep->dataPtr().get() == root->dataPtr().get()) throw py::value_error("deep copy should clone data");
    // docs:end copy_cpp_example
}

void test_getAtPath_example() {
    // docs:start getAtPath_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    root->addChild(child);
    if (root->getAtPath("root/child").get() != child.get()) throw py::value_error("getAtPath failed");
    // docs:end getAtPath_cpp_example
}

void test_path_example() {
    // docs:start path_cpp_example
    auto root = newNode("root");
    auto child = newNode("child");
    root->addChild(child);
    if (child->path() != "root/child") throw py::value_error("path mismatch");
    // docs:end path_cpp_example
}

void test_descendants_example() {
    // docs:start descendants_cpp_example
    auto root = newNode("root");
    root->addChildren({newNode("a"), newNode("b")});
    if (root->descendants().size() != 3) throw py::value_error("descendants size mismatch");
    // docs:end descendants_cpp_example
}

void test_merge_example() {
    // docs:start merge_cpp_example
    auto left = newNode("Root");
    left->addChild(newNode("A"));
    auto right = newNode("Root");
    right->addChild(newNode("B"));
    left->merge(right);
    if (left->getChildrenNames() != std::vector<std::string>({"A", "B"})) {
        throw py::value_error("merge mismatch");
    }
    // docs:end merge_cpp_example
}

void test_hasLinkTarget_example() {
    // docs:start hasLinkTarget_cpp_example
    auto link = newNode("link");
    if (link->hasLinkTarget()) throw py::value_error("link should not have target initially");
    link->setLinkTarget(".", "/root/target");
    if (!link->hasLinkTarget()) throw py::value_error("link target should be set");
    // docs:end hasLinkTarget_cpp_example
}

void test_linkTargetFile_example() {
    // docs:start linkTargetFile_cpp_example
    auto link = newNode("link");
    link->setLinkTarget("external.cgns", "/CGNSTree/Base");
    if (link->linkTargetFile() != "external.cgns") throw py::value_error("linkTargetFile mismatch");
    // docs:end linkTargetFile_cpp_example
}

void test_linkTargetPath_example() {
    // docs:start linkTargetPath_cpp_example
    auto link = newNode("link");
    link->setLinkTarget("external.cgns", "/CGNSTree/Base");
    if (link->linkTargetPath() != "/CGNSTree/Base") throw py::value_error("linkTargetPath mismatch");
    // docs:end linkTargetPath_cpp_example
}

void test_setLinkTarget_example() {
    // docs:start setLinkTarget_cpp_example
    auto link = newNode("link");
    link->setLinkTarget(".", "/root/target");
    if (!link->hasLinkTarget()) throw py::value_error("setLinkTarget failed");
    // docs:end setLinkTarget_cpp_example
}

void test_clearLinkTarget_example() {
    // docs:start clearLinkTarget_cpp_example
    auto link = newNode("link");
    link->setLinkTarget(".", "/root/target");
    link->clearLinkTarget();
    if (link->hasLinkTarget()) throw py::value_error("clearLinkTarget failed");
    // docs:end clearLinkTarget_cpp_example
}

void test_getLinks_example() {
    // docs:start getLinks_cpp_example
    auto root = newNode("root");
    auto link = newNode("link");
    root->addChild(link);
    link->setLinkTarget(".", "/root/target");
    if (root->getLinks().size() != 1) throw py::value_error("getLinks size mismatch");
    // docs:end getLinks_cpp_example
}

void test_setParameters_example() {
    // docs:start setParameters_cpp_example
    auto root = newNode("root");
    ParameterValue::DictEntries entries;
    entries.emplace_back("alpha", ParameterValue::makeData(std::make_shared<Array>(double(1.5))));
    root->setParameters("Parameters", entries);
    auto out = root->getParameters("Parameters");
    if (out.kind != ParameterValue::Kind::Dict) throw py::value_error("setParameters/getParameters dict mismatch");
    // docs:end setParameters_cpp_example
}

void test_getParameters_example() {
    // docs:start getParameters_cpp_example
    auto root = newNode("root");
    ParameterValue::DictEntries entries;
    entries.emplace_back("count", ParameterValue::makeData(std::make_shared<Array>(int32_t(3))));
    root->setParameters("Parameters", entries);
    auto out = root->getParameters("Parameters");
    const auto* count = findEntryByName(out.dictEntries, "count");
    if (!count || !count->data || !(*count->data == 3)) throw py::value_error("getParameters value mismatch");
    // docs:end getParameters_cpp_example
}

void test_printTree_example() {
    // docs:start printTree_cpp_example
    auto root = newNode("root");
    root->addChildren({newNode("a"), newNode("b")});
    auto tree = root->printTree();
    if (tree.find("root") == std::string::npos) throw py::value_error("printTree output mismatch");
    // docs:end printTree_cpp_example
}

#ifdef ENABLE_HDF5_IO
void test_reloadNodeData_example() {
    // docs:start reloadNodeData_cpp_example
    const std::string filename = "reload_node_data_example.cgns";
    auto root = newNode("root");
    auto valueNode = newNode("value");
    root->addChild(valueNode);
    valueNode->setData(42);
    root->write(filename);
    valueNode->setData(-1);
    valueNode->reloadNodeData(filename);
    if (!(valueNode->data() == 42)) throw py::value_error("reloadNodeData example failed");
    // docs:end reloadNodeData_cpp_example
}

void test_saveThisNodeOnly_example() {
    // docs:start saveThisNodeOnly_cpp_example
    const std::string filename = "save_this_node_only_example.cgns";
    auto root = newNode("root");
    auto mutableNode = newNode("mutable");
    mutableNode->setData(1);
    root->addChild(mutableNode);
    root->write(filename);
    mutableNode->setData(99);
    mutableNode->saveThisNodeOnly(filename);
    auto readRoot = io::read(filename);
    if (!(readRoot->getAtPath("root/mutable")->data() == 99)) throw py::value_error("saveThisNodeOnly example failed");
    // docs:end saveThisNodeOnly_cpp_example
}

void test_write_example() {
    // docs:start write_cpp_example
    const std::string filename = "write_example.cgns";
    auto root = newNode("root");
    root->addChild(newNode("child"));
    root->write(filename);
    auto readRoot = io::read(filename);
    if (!readRoot->getAtPath("root/child")) throw py::value_error("write example failed");
    // docs:end write_cpp_example
}
#endif

void test_node_method_examples() {
    test_Node_example();
    test_pick_example();
    test_name_example();
    test_setName_example();
    test_type_example();
    test_setType_example();
    test_data_example();
    test_setData_example();
    test_children_example();
    test_hasChildren_example();
    test_parent_example();
    test_root_example();
    test_level_example();
    test_position_example();
    test_detach_example();
    test_attachTo_example();
    test_addChild_example();
    test_addChildren_example();
    test_siblings_example();
    test_hasSiblings_example();
    test_getChildrenNames_example();
    test_swap_example();
    test_copy_example();
    test_getAtPath_example();
    test_path_example();
    test_descendants_example();
    test_merge_example();
    test_hasLinkTarget_example();
    test_linkTargetFile_example();
    test_linkTargetPath_example();
    test_setLinkTarget_example();
    test_clearLinkTarget_example();
    test_getLinks_example();
    test_setParameters_example();
    test_getParameters_example();
    test_printTree_example();
#ifdef ENABLE_HDF5_IO
    test_reloadNodeData_example();
    test_saveThisNodeOnly_example();
    test_write_example();
#endif
}
