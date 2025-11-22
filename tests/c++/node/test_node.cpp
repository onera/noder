# include "test_node.hpp"

using namespace std::string_literals;

void test_init() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b", "type_t");
}

void test_name() {
    auto a = std::make_shared<Node>("a");
    std::string name = a->name();
    if ( name != std::string("a") ) throw py::value_error("did not get name 'a' as string");
    if ( name != "a" ) throw py::value_error("did not get name 'a' as chars");
}

void test_setName() {
    auto a = std::make_shared<Node>("a");
    a->setName("b");
    std::string name = a->name();
    if ( name != "b" ) throw py::value_error("did not get name 'b'");
}

void test_type() {
    auto a = std::make_shared<Node>("a", "type_t");
    std::string node_type = a->type();
    if ( node_type != "type_t" ) throw py::value_error("did not get type 'type_t'");
}

void test_setType() {
    auto a = std::make_shared<Node>("a");
    a->setType("NewType_t");
    std::string node_type = a->type();
    if ( node_type != "NewType_t" ) throw py::value_error("did not get type 'NewType_t'");
}

void test_binding_setNameAndTypeFromPython(Node& node) {
    node.setName("NewName");
    node.setType("NewType");
}

void test_noData() {
    auto a = std::make_shared<Node>();
    if (!a->noData()) throw py::value_error("expected no data");
}

void test_children_empty() {
    auto a = std::make_shared<Node>("a");
    auto children = a->children();
    if (children.size() != 0) throw std::runtime_error("failed C++ empty children children");
}

void test_parent_empty() {
    auto a = std::make_shared<Node>("a");
    auto parent = a->parent().lock();
    if ( parent ) throw py::value_error("did not get null parent");
}

void test_root_itself() {
    auto a = std::make_shared<Node>("a");
    auto b = a->root();
    if ( b.get() != a.get() ) throw py::value_error("single node root does not point to itself");
}

void test_level_0() {
    auto a = std::make_shared<Node>("a");
    size_t level = a->level();
    if ( level != 0 ) throw py::value_error("single node level is not 0");
}

void test_position_null() {
    auto a = std::make_shared<Node>("a");
    size_t pos = a->position();
    if ( pos != 0 ) throw py::value_error("expected 0 for single node sibling pos");
}

void test_getPath_itself() {
    auto a = std::make_shared<Node>("a");
    std::string path = a->path();
    if ( path != "a" ) throw py::value_error("expected single node path 'a'");
}

void test_attachTo() {

    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    b->attachTo(a);
    std::string expected_path_of_b = "a/b";
    if ( b->path() != expected_path_of_b ) throw py::value_error("expected path 'a/b'"+" but got "s + b->path());

}


void test_attachTo_multiple_levels() {
    size_t max_levels = 20;
    std::shared_ptr<Node> first_node = std::make_shared<Node>("0");
    std::vector<std::shared_ptr<Node>> nodes = {first_node};
    std::vector<std::string> paths = {"0"};
    for (size_t i = 1; i < max_levels; i++)
    {
        std::shared_ptr<Node> node = std::make_shared<Node>(std::to_string(i));
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
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    a->addChild(b);
    std::string expected_path_of_b = "a/b";
    if ( b->path() != expected_path_of_b ) throw py::value_error("expected path 'a/b'");
}

void test_addChildAsPointer() {
    auto a = std::make_shared<Node>("a");

    std::shared_ptr<Node> b = std::make_shared<Node>(Node("b"));
    a->addChild(b);

    std::string path_of_b = b->path();

    if ( path_of_b != "a/b" ) {
        throw py::value_error("expected path 'a/b'");
    }
}

void test_detach_0() {
    auto a = std::make_shared<Node>("a");
    a->detach();
    if ( a->parent().lock() ) throw py::value_error("expected no parent after detachment");
}

void test_detach_1() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");

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
    auto a = std::make_shared<Node>("a");

    std::vector<std::shared_ptr<Node>> children;
    children.resize(nb_of_children);
    for (size_t i = 0; i < nb_of_children; i++) {
        children[i] = std::make_shared<Node>(std::to_string(i));
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
    nodes[0] = std::make_shared<Node>("0");
    std::stringstream expected_path;
    expected_path << 0;
    for (size_t i = 1; i < max_levels; i++) {
        nodes[i] = std::make_shared<Node>(std::to_string(i));
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
    nodes.emplace_back(std::make_shared<Node>(std::to_string(0)));
    for (size_t i = 1; i < max_levels; i++) {
        nodes.emplace_back(std::make_shared<Node>(std::to_string(i)));
        nodes[i]->attachTo(nodes[i-1]);
    }
    
    size_t index_node_to_delete = 3;
    nodes[index_node_to_delete]->detach();
    nodes.erase(nodes.begin()+static_cast<ssize_t>(index_node_to_delete));
}


void test_getPath() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    auto c = std::make_shared<Node>("c");
    b->attachTo(a);
    c->attachTo(b);
    std::string expected_path_of_c = "a/b/c";
    if (c->path() != expected_path_of_c) {
        throw py::value_error("was expecting path 'a/b/c'");
    }
}

void test_root() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    auto c = std::make_shared<Node>("c");
    auto d = std::make_shared<Node>("d");
    auto e = std::make_shared<Node>("e");

    a->addChild(b);
    b->addChild(c);
    c->addChild(d);
    d->addChild(e);

    if (e->root().get() != a.get()) throw py::value_error("root expected top node");
}

void test_level() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    auto c = std::make_shared<Node>("c");
    auto d = std::make_shared<Node>("d");
    auto e = std::make_shared<Node>("e");

    a->addChild(b);
    b->addChild(c);
    c->addChild(d);
    d->addChild(e);

    if (e->level() != 4) throw py::value_error("expected level 4");
}

void test_printTree() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    auto c = std::make_shared<Node>("c");
    auto d = std::make_shared<Node>("d");
    auto e = std::make_shared<Node>("e");
    auto f = std::make_shared<Node>("f");
    auto g = std::make_shared<Node>("g");
    auto h = std::make_shared<Node>("h");
    auto i = std::make_shared<Node>("i");

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
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    auto c = std::make_shared<Node>("c");
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
    auto b = std::make_shared<Node>("b");
    auto c = std::make_shared<Node>("c");
    auto d = std::make_shared<Node>("d");

    b->attachTo(node);
    c->attachTo(b);
    d->attachTo(node);
}

void test_position() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    auto c = std::make_shared<Node>("c");
    auto d = std::make_shared<Node>("d");

    b->attachTo(a);
    c->attachTo(a);
    d->attachTo(a);

    if (a->position() != 0) throw py::value_error("expected pos 0 for node a");
    if (b->position() != 0) throw py::value_error("expected pos 0 for node b");
    if (c->position() != 1) throw py::value_error("expected pos 1 for node c");
    if (d->position() != 2) throw py::value_error("expected pos 2 for node d");
}
