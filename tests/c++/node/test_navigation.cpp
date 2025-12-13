# include "test_navigation.hpp"
# include <array/array.hpp>

using namespace std::string_literals;

void test_childByName() {
    auto a = std::make_shared<Node>("a"s);
    auto b = std::make_shared<Node>("b"s);
    b->attachTo(a);
    auto c = std::make_shared<Node>("c"s);
    c->attachTo(a);
    auto d = std::make_shared<Node>("d"s);
    d->attachTo(c);

    auto n = a->pick().childByName("c"s);
    if (n->name()!="c") throw std::runtime_error("failed test_childByName");

    auto unexpected = a->pick().childByName("d"s);
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

    auto n = a->pick().byName("d"s);
    if (n->name()!="d") throw py::value_error("expected name 'd'");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byName("e");
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

    auto n = a->pick().byNamePattern("ab\\B"); // equivalent to R"(ab\B)" 
    if (n->name()!="abcd") throw py::value_error("failed test_byNamePattern");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byNamePattern(R"(\Bab)");
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

    auto n = a->pick().childByType("c_t"s); // important, note the "*" in order to avoid copy
    if (n->type()!="c_t") throw std::runtime_error("failed test_childByType");

    auto unexpected = a->pick().childByType("d_t"s);
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

    auto n = a->pick().byType("d_t");
    if (n->type()!="d_t") throw py::value_error("expected type 'd_t'");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byType("e_t");
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

    auto n = a->pick().byTypePattern("ab\\B"); // equivalent to R"(ab\B)" 
    if (n->type()!="abcd_t") throw py::value_error("failed test_byTypePattern");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byTypePattern(R"(\Bab)");
    if (unexpected != nullptr) throw py::value_error("failed null test_byTypePattern");
}


void test_childByData() {
    auto a = std::make_shared<Node>("a"s);
    auto b = std::make_shared<Node>("b"s);
    Array bData("test"s);
    b->setData(bData);
    b->attachTo(a);

    auto c = std::make_shared<Node>("c"s);
    Array cData = arrayfactory::zeros<double>({1});
    c->setData(cData);
    c->attachTo(a);
    
    auto d = std::make_shared<Node>("d"s);
    Array dData = arrayfactory::zeros<int8_t>({3,3});
    d->setData(dData);
    d->attachTo(c);

    auto e = std::make_shared<Node>("e"s);
    Array eData("requested"s);
    e->setData(eData);
    e->attachTo(a);


    auto n = a->pick().childByData("requested"s);
    if (n->name()!="e") throw std::runtime_error("failed test_childByData");

    auto unexpected = a->pick().childByData("d"s);
    if (unexpected) throw std::runtime_error("failed null test_childByData");
}
