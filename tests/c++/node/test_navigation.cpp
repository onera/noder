#include "test_navigation.hpp"

using namespace std::string_literals;

void test_childByName() {
    auto a = std::make_shared<Node>("a"s);
    auto b = std::make_shared<Node>("b"s);
    b->attachTo(a);
    auto c = std::make_shared<Node>("c"s);
    c->attachTo(a);
    auto d = std::make_shared<Node>("d"s);
    d->attachTo(c);

    auto n = a->nav().childByName("c"s);
    if (n->name()!="c") throw std::runtime_error("failed test_childByName");

    auto unexpected = a->nav().childByName("d"s);
    if (unexpected) throw std::runtime_error("failed null test_childByName");
}


void test_byName() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    b->attachTo(a);
    auto c = std::make_shared<Node>("c");
    c->attachTo(a);
    auto d = std::make_shared<Node>("d");
    d->attachTo(c);

    auto n = a->nav().byName("d"s);
    if (n->name()!="d") throw py::value_error("expected name 'd'");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->nav().byName("e");
    if (unexpected != nullptr) throw py::value_error("expected nullptr");

}


void test_byNamePattern() {
    auto a = std::make_shared<Node>("a");
    auto b = std::make_shared<Node>("b");
    b->attachTo(a);
    auto c = std::make_shared<Node>("c");
    c->attachTo(a);
    auto d = std::make_shared<Node>("abcd");
    d->attachTo(c);

    auto n = a->nav().byNamePattern("ab\\B"); // equivalent to R"(ab\B)" 
    if (n->name()!="abcd") throw py::value_error("failed test_byNamePattern");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->nav().byNamePattern(R"(\Bab)");
    if (unexpected != nullptr) throw py::value_error("failed null test_byNamePattern");
}


void test_childByType() {
    auto a = std::make_shared<Node>("a"s,"a_t"s);
    auto b = std::make_shared<Node>("b"s,"b_t"s);
    b->attachTo(a);
    auto c = std::make_shared<Node>("c"s,"c_t"s);
    c->attachTo(a);
    auto d = std::make_shared<Node>("d"s,"d_t"s);
    d->attachTo(c);

    auto n = a->nav().childByType("c_t"s); // important, note the "*" in order to avoid copy
    if (n->type()!="c_t") throw std::runtime_error("failed test_childByType");

    auto unexpected = a->nav().childByType("d_t"s);
    if (unexpected != nullptr) throw std::runtime_error("failed null test_childByType");
}


void test_byType() {
    auto a = std::make_shared<Node>("a","a_t");
    auto b = std::make_shared<Node>("b","b_t");
    b->attachTo(a);
    auto c = std::make_shared<Node>("c","c_t");
    c->attachTo(a);
    auto d = std::make_shared<Node>("d","d_t");
    d->attachTo(c);

    auto n = a->nav().byType("d_t");
    if (n->type()!="d_t") throw py::value_error("expected type 'd_t'");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->nav().byType("e_t");
    if (unexpected != nullptr) throw py::value_error("expected nullptr");

}


void test_byTypePattern() {
    auto a = std::make_shared<Node>("a","a_t");
    auto b = std::make_shared<Node>("b","b_t");
    b->attachTo(a);
    auto c = std::make_shared<Node>("c","c_t");
    c->attachTo(a);
    auto d = std::make_shared<Node>("d","abcd_t");
    d->attachTo(c);

    auto n = a->nav().byTypePattern("ab\\B"); // equivalent to R"(ab\B)" 
    if (n->type()!="abcd_t") throw py::value_error("failed test_byTypePattern");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->nav().byTypePattern(R"(\Bab)");
    if (unexpected != nullptr) throw py::value_error("failed null test_byTypePattern");
}