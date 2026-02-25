# include "test_node_group.hpp"

void test_operator_plus_attachToFirstParent() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");

    b->attachTo(a);
    d->attachTo(c);

    auto group = b + c + d;
    (void)group;

    if (b->parent().lock().get() != a.get()) throw py::value_error("expected b parent to be a");
    if (c->parent().lock().get() != a.get()) throw py::value_error("expected c parent to be a");
    if (d->parent().lock().get() != a.get()) throw py::value_error("expected d parent to be a");

    const auto& aChildren = a->children();
    if (aChildren.size() != 3) throw py::value_error("expected a to have 3 children");
    if (aChildren[0].get() != b.get()) throw py::value_error("expected child 0 to be b");
    if (aChildren[1].get() != c.get()) throw py::value_error("expected child 1 to be c");
    if (aChildren[2].get() != d.get()) throw py::value_error("expected child 2 to be d");
}

void test_operator_div_addChild_chain() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");

    auto end = a / b / c;
    if (end.get() != c.get()) throw py::value_error("expected a/b/c to return c");

    if (b->parent().lock().get() != a.get()) throw py::value_error("expected b parent to be a");
    if (c->parent().lock().get() != b.get()) throw py::value_error("expected c parent to be b");
}

void test_operator_div_with_plus_group() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");
    auto e = newNode("e");

    auto end = a / b / (c + d + e);
    if (end.get() != e.get()) throw py::value_error("expected a/b/(c+d+e) to return e");

    if (b->parent().lock().get() != a.get()) throw py::value_error("expected b parent to be a");
    if (c->parent().lock().get() != b.get()) throw py::value_error("expected c parent to be b");
    if (d->parent().lock().get() != b.get()) throw py::value_error("expected d parent to be b");
    if (e->parent().lock().get() != b.get()) throw py::value_error("expected e parent to be b");

    const auto& bChildren = b->children();
    if (bChildren.size() != 3) throw py::value_error("expected b to have 3 children");
    if (bChildren[0].get() != c.get()) throw py::value_error("expected child 0 to be c");
    if (bChildren[1].get() != d.get()) throw py::value_error("expected child 1 to be d");
    if (bChildren[2].get() != e.get()) throw py::value_error("expected child 2 to be e");
}

void test_operator_nested_expression_tree() {
    auto a = newNode("a");
    auto b = newNode("b");
    auto c = newNode("c");
    auto d = newNode("d");
    auto e = newNode("e");
    auto f = newNode("f");
    auto g = newNode("g");
    auto h = newNode("h");

    auto end = a / (b + (c / (e + (f / g) + h)) + d);
    if (end.get() != d.get()) throw py::value_error("expected expression to return d");

    std::vector<std::string> expectedPaths = {
        "a",
        "a/b",
        "a/c",
        "a/c/e",
        "a/c/f",
        "a/c/f/g",
        "a/c/h",
        "a/d"
    };

    auto descendants = a->descendants();
    if (descendants.size() != expectedPaths.size()) {
        throw py::value_error("unexpected number of descendants");
    }

    for (size_t i = 0; i < descendants.size(); ++i) {
        if (descendants[i]->path() != expectedPaths[i]) {
            throw py::value_error(
                "unexpected path at index " + std::to_string(i) +
                ": got '" + descendants[i]->path() +
                "' expected '" + expectedPaths[i] + "'");
        }
    }
}

void test_nodes_accessor() {
    auto a = newNode("a");
    auto b = newNode("b");
    NodeGroup group = a + b;
    const auto& nodes = group.nodes();
    if (nodes.size() != 2) throw py::value_error("expected 2 grouped nodes");
    if (nodes[0].get() != a.get()) throw py::value_error("expected first node to be a");
    if (nodes[1].get() != b.get()) throw py::value_error("expected second node to be b");
}

void test_empty_group() {
    NodeGroup group;
    if (!group.empty()) throw py::value_error("default group should be empty");
}
