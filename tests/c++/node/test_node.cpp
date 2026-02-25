# include "test_node.hpp"

#ifdef ENABLE_HDF5_IO
#include "io/io.hpp"
#endif

using namespace std::string_literals;

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
