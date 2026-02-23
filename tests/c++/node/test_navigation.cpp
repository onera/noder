# include "test_navigation.hpp"
# include <array/array.hpp>

using namespace std::string_literals;

void test_childByName() {
    auto a = newNode("a"s);
    auto b = newNode("b"s);
    b->attachTo(a);
    auto c = newNode("c"s);
    c->attachTo(a);
    auto d = newNode("d"s);
    d->attachTo(c);

    auto n = a->pick().childByName("c"s);
    if (n->name()!="c") throw std::runtime_error("failed test_childByName");

    auto unexpected = a->pick().childByName("d"s);
    if (unexpected) throw std::runtime_error("failed null test_childByName");
}


void test_byName() {
    auto a = newNode("a");
    auto b = newNode("b");
    b->attachTo(a);
    auto c = newNode("c");
    c->attachTo(a);
    auto d = newNode("d");
    d->attachTo(c);

    auto n = a->pick().byName("d"s);
    if (n->name()!="d") throw py::value_error("expected name 'd'");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byName("e");
    if (unexpected != nullptr) throw py::value_error("expected nullptr");

}

void test_allByName() {
    auto a = newNode("a");
    auto b = newNode("target");
    b->attachTo(a);
    auto c = newNode("c");
    c->attachTo(a);
    auto d = newNode("target");
    d->attachTo(c);
    auto e = newNode("target");
    e->attachTo(d);

    auto matches = a->pick().allByName("target");
    if (matches.size() != 3) throw py::value_error("expected 3 matches");
    if (matches[0].get() != b.get()) throw py::value_error("expected first match b");
    if (matches[1].get() != d.get()) throw py::value_error("expected second match d");
    if (matches[2].get() != e.get()) throw py::value_error("expected third match e");

    auto none = a->pick().allByName("missing");
    if (!none.empty()) throw py::value_error("expected empty vector");
}


void test_byNameRegex() {
    auto a = newNode("a");
    auto b = newNode("b");
    b->attachTo(a);
    auto c = newNode("c");
    c->attachTo(a);
    auto d = newNode("abcd");
    d->attachTo(c);

    auto n = a->pick().byNameRegex("ab\\B"); // equivalent to R"(ab\B)" 
    if (n->name()!="abcd") throw py::value_error("failed test_byNameRegex");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byNameRegex(R"(\Bab)");
    if (unexpected != nullptr) throw py::value_error("failed null test_byNameRegex");
}

void test_allByNameRegex() {
    auto a = newNode("a");
    auto b = newNode("ab_one");
    b->attachTo(a);
    auto c = newNode("c");
    c->attachTo(a);
    auto d = newNode("ab_two");
    d->attachTo(c);

    auto matches = a->pick().allByNameRegex("^ab_.*");
    if (matches.size() != 2) throw py::value_error("expected 2 matches");
    if (matches[0].get() != b.get()) throw py::value_error("expected first match b");
    if (matches[1].get() != d.get()) throw py::value_error("expected second match d");

    auto none = a->pick().allByNameRegex("^zz_.*");
    if (!none.empty()) throw py::value_error("expected empty vector");
}

void test_byNameGlob() {
    auto a = newNode("a");
    auto b = newNode("b");
    b->attachTo(a);
    auto c = newNode("c");
    c->attachTo(a);
    auto d = newNode("abcd");
    d->attachTo(c);

    auto n = a->pick().byNameGlob("ab*");
    if (n->name()!="abcd") throw py::value_error("failed test_byNameGlob");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byNameGlob("zz*");
    if (unexpected != nullptr) throw py::value_error("failed null test_byNameGlob");
}

void test_allByNameGlob() {
    auto a = newNode("a");
    auto b = newNode("ab_one");
    b->attachTo(a);
    auto c = newNode("c");
    c->attachTo(a);
    auto d = newNode("ab_two");
    d->attachTo(c);

    auto matches = a->pick().allByNameGlob("ab_*");
    if (matches.size() != 2) throw py::value_error("expected 2 matches");
    if (matches[0].get() != b.get()) throw py::value_error("expected first match b");
    if (matches[1].get() != d.get()) throw py::value_error("expected second match d");

    auto none = a->pick().allByNameGlob("zz_*");
    if (!none.empty()) throw py::value_error("expected empty vector");
}


void test_childByType() {
    auto a = newNode("a"s,"a_t"s);
    auto b = newNode("b"s,"b_t"s);
    b->attachTo(a);
    auto c = newNode("c"s,"c_t"s);
    c->attachTo(a);
    auto d = newNode("d"s,"d_t"s);
    d->attachTo(c);

    auto n = a->pick().childByType("c_t"s); // important, note the "*" in order to avoid copy
    if (n->type()!="c_t") throw std::runtime_error("failed test_childByType");

    auto unexpected = a->pick().childByType("d_t"s);
    if (unexpected != nullptr) throw std::runtime_error("failed null test_childByType");
}


void test_byType() {
    auto a = newNode("a","a_t");
    auto b = newNode("b","b_t");
    b->attachTo(a);
    auto c = newNode("c","c_t");
    c->attachTo(a);
    auto d = newNode("d","d_t");
    d->attachTo(c);

    auto n = a->pick().byType("d_t");
    if (n->type()!="d_t") throw py::value_error("expected type 'd_t'");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byType("e_t");
    if (unexpected != nullptr) throw py::value_error("expected nullptr");

}

void test_allByType() {
    auto a = newNode("a","a_t");
    auto b = newNode("b","target_t");
    b->attachTo(a);
    auto c = newNode("c","c_t");
    c->attachTo(a);
    auto d = newNode("d","target_t");
    d->attachTo(c);

    auto matches = a->pick().allByType("target_t");
    if (matches.size() != 2) throw py::value_error("expected 2 matches");
    if (matches[0].get() != b.get()) throw py::value_error("expected first match b");
    if (matches[1].get() != d.get()) throw py::value_error("expected second match d");

    auto none = a->pick().allByType("missing_t");
    if (!none.empty()) throw py::value_error("expected empty vector");
}


void test_byTypeRegex() {
    auto a = newNode("a","a_t");
    auto b = newNode("b","b_t");
    b->attachTo(a);
    auto c = newNode("c","c_t");
    c->attachTo(a);
    auto d = newNode("d","abcd_t");
    d->attachTo(c);

    auto n = a->pick().byTypeRegex("ab\\B"); // equivalent to R"(ab\B)" 
    if (n->type()!="abcd_t") throw py::value_error("failed test_byTypeRegex");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byTypeRegex(R"(\Bab)");
    if (unexpected != nullptr) throw py::value_error("failed null test_byTypeRegex");
}

void test_allByTypeRegex() {
    auto a = newNode("a","a_t");
    auto b = newNode("b","ab_one_t");
    b->attachTo(a);
    auto c = newNode("c","c_t");
    c->attachTo(a);
    auto d = newNode("d","ab_two_t");
    d->attachTo(c);

    auto matches = a->pick().allByTypeRegex("^ab_.*_t$");
    if (matches.size() != 2) throw py::value_error("expected 2 matches");
    if (matches[0].get() != b.get()) throw py::value_error("expected first match b");
    if (matches[1].get() != d.get()) throw py::value_error("expected second match d");

    auto none = a->pick().allByTypeRegex("^zz_.*_t$");
    if (!none.empty()) throw py::value_error("expected empty vector");
}

void test_byTypeGlob() {
    auto a = newNode("a","a_t");
    auto b = newNode("b","b_t");
    b->attachTo(a);
    auto c = newNode("c","c_t");
    c->attachTo(a);
    auto d = newNode("d","abcd_t");
    d->attachTo(c);

    auto n = a->pick().byTypeGlob("ab*");
    if (n->type()!="abcd_t") throw py::value_error("failed test_byTypeGlob");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byTypeGlob("zz*");
    if (unexpected != nullptr) throw py::value_error("failed null test_byTypeGlob");
}

void test_allByTypeGlob() {
    auto a = newNode("a","a_t");
    auto b = newNode("b","ab_one_t");
    b->attachTo(a);
    auto c = newNode("c","c_t");
    c->attachTo(a);
    auto d = newNode("d","ab_two_t");
    d->attachTo(c);

    auto matches = a->pick().allByTypeGlob("ab_*_t");
    if (matches.size() != 2) throw py::value_error("expected 2 matches");
    if (matches[0].get() != b.get()) throw py::value_error("expected first match b");
    if (matches[1].get() != d.get()) throw py::value_error("expected second match d");

    auto none = a->pick().allByTypeGlob("zz_*_t");
    if (!none.empty()) throw py::value_error("expected empty vector");
}


void test_childByData() {
    auto a = newNode("a"s);
    auto b = newNode("b"s);
    Array bData("test"s);
    b->setData(bData);
    b->attachTo(a);

    auto c = newNode("c"s);
    Array cData = arrayfactory::zeros<double>({1});
    c->setData(cData);
    c->attachTo(a);
    
    auto d = newNode("d"s);
    Array dData = arrayfactory::zeros<int8_t>({3,3});
    d->setData(dData);
    d->attachTo(c);

    auto e = newNode("e"s);
    Array eData("requested"s);
    e->setData(eData);
    e->attachTo(a);


    auto n = a->pick().childByData("requested"s);
    if (n->name()!="e") throw std::runtime_error("failed test_childByData");

    auto unexpected = a->pick().childByData("d"s);
    if (unexpected) throw std::runtime_error("failed null test_childByData");
}


void test_childByDataUsingChar() {
    auto a = newNode("a"s);
    auto b = newNode("b"s);
    Array bData("test"s);
    b->setData(bData);
    b->attachTo(a);

    auto c = newNode("c"s);
    Array cData = arrayfactory::zeros<double>({1});
    c->setData(cData);
    c->attachTo(a);
    
    auto d = newNode("d"s);
    Array dData = arrayfactory::zeros<int8_t>({3,3});
    d->setData(dData);
    d->attachTo(c);

    auto e = newNode("e"s);
    Array eData("requested"s);
    e->setData(eData);
    e->attachTo(a);


    auto n = a->pick().childByData("requested");
    if (n->name()!="e") throw std::runtime_error("failed test_childByDataUsingChar");

    auto unexpected = a->pick().childByData("d");
    if (unexpected) throw std::runtime_error("failed null test_childByDataUsingChar");
}


void test_childByDataScalarDirect() {
    auto a = newNode("a"s);

    auto b = newNode("b"s);
    b->setData(7);
    b->attachTo(a);

    auto c = newNode("c"s);
    c->setData(3.5);
    c->attachTo(a);

    auto d = newNode("d"s);
    d->setData(true);
    d->attachTo(a);

    auto nInt = a->pick().childByData(7);
    if (!nInt || nInt->name() != "b") throw std::runtime_error("failed test_childByDataScalarDirect int");
    if (nInt.get() != b.get()) throw std::runtime_error("expected node b without copy");

    auto nFloat = a->pick().childByData(3.5);
    if (!nFloat || nFloat->name() != "c") throw std::runtime_error("failed test_childByDataScalarDirect float");
    if (nFloat.get() != c.get()) throw std::runtime_error("expected node c without copy");

    auto nBool = a->pick().childByData(true);
    if (!nBool || nBool->name() != "d") throw std::runtime_error("failed test_childByDataScalarDirect bool");
    if (nBool.get() != d.get()) throw std::runtime_error("expected node d without copy");

    auto unexpected = a->pick().childByData(9);
    if (unexpected) throw std::runtime_error("failed null test_childByDataScalarDirect");
}


void test_byData() {
    auto a = newNode("a");
    a->setData("a_d");

    auto b = newNode("b");
    b->setData("b_d");
    b->attachTo(a);
    
    auto c = newNode("c");
    c->setData("c_d");
    c->attachTo(a);
    
    auto d = newNode("d");
    d->setData("d_d");
    d->attachTo(c);

    auto n = a->pick().byData("d_d");
    if (n->data().extractString()!="d_d") throw py::value_error("expected data 'd_d'");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byData("e_d");
    if (unexpected != nullptr) throw py::value_error("expected nullptr");

}

void test_allByData() {
    auto a = newNode("a");
    a->setData("root");

    auto b = newNode("b");
    b->setData("target");
    b->attachTo(a);

    auto c = newNode("c");
    c->setData("mid");
    c->attachTo(a);

    auto d = newNode("d");
    d->setData("target");
    d->attachTo(c);

    auto matches = a->pick().allByData("target");
    if (matches.size() != 2) throw py::value_error("expected 2 matches");
    if (matches[0].get() != b.get()) throw py::value_error("expected first match b");
    if (matches[1].get() != d.get()) throw py::value_error("expected second match d");

    auto none = a->pick().allByData("missing");
    if (!none.empty()) throw py::value_error("expected empty vector");
}

void test_byDataGlob() {
    auto a = newNode("a");
    a->setData("a_d");

    auto b = newNode("b");
    b->setData("b_d");
    b->attachTo(a);

    auto c = newNode("c");
    c->setData("c_d");
    c->attachTo(a);

    auto d = newNode("d");
    d->setData("requested_value_42");
    d->attachTo(c);

    auto n = a->pick().byDataGlob("requested_*_4?");
    if (n->data().extractString()!="requested_value_42") throw py::value_error("expected data 'requested_value_42'");
    if (n.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byDataGlob("missing*");
    if (unexpected != nullptr) throw py::value_error("expected nullptr");
}

void test_allByDataGlob() {
    auto a = newNode("a");
    a->setData("root");

    auto b = newNode("b");
    b->setData("requested_value_01");
    b->attachTo(a);

    auto c = newNode("c");
    c->setData("mid");
    c->attachTo(a);

    auto d = newNode("d");
    d->setData("requested_value_42");
    d->attachTo(c);

    auto matches = a->pick().allByDataGlob("requested_*_??");
    if (matches.size() != 2) throw py::value_error("expected 2 matches");
    if (matches[0].get() != b.get()) throw py::value_error("expected first match b");
    if (matches[1].get() != d.get()) throw py::value_error("expected second match d");

    auto none = a->pick().allByDataGlob("missing*");
    if (!none.empty()) throw py::value_error("expected empty vector");
}

void test_byDataScalarDirect() {
    auto a = newNode("a");
    a->setData(123);

    auto b = newNode("b");
    b->setData(7);
    b->attachTo(a);

    auto c = newNode("c");
    c->setData(3.5);
    c->attachTo(a);

    auto d = newNode("d");
    d->setData(true);
    d->attachTo(c);

    auto nInt = a->pick().byData(7);
    if (!nInt || nInt->name() != "b") throw py::value_error("expected data 7");
    if (nInt.get() != b.get()) throw py::value_error("expected to recover node b without copy");

    auto nFloat = a->pick().byData(3.5);
    if (!nFloat || nFloat->name() != "c") throw py::value_error("expected data 3.5");
    if (nFloat.get() != c.get()) throw py::value_error("expected to recover node c without copy");

    auto nBool = a->pick().byData(true);
    if (!nBool || nBool->name() != "d") throw py::value_error("expected data true");
    if (nBool.get() != d.get()) throw py::value_error("expected to recover node d without copy");

    auto unexpected = a->pick().byData(999);
    if (unexpected != nullptr) throw py::value_error("expected nullptr");
}

void test_allByDataScalarDirect() {
    auto a = newNode("a");
    a->setData(123);

    auto b = newNode("b");
    b->setData(7);
    b->attachTo(a);

    auto c = newNode("c");
    c->setData(7);
    c->attachTo(a);

    auto d = newNode("d");
    d->setData(3.5);
    d->attachTo(c);

    auto e = newNode("e");
    e->setData(true);
    e->attachTo(c);

    auto ints = a->pick().allByData(7);
    if (ints.size() != 2) throw py::value_error("expected 2 int matches");
    if (ints[0].get() != b.get()) throw py::value_error("expected first match b");
    if (ints[1].get() != c.get()) throw py::value_error("expected second match c");

    auto floats = a->pick().allByData(3.5);
    if (floats.size() != 1 || floats[0].get() != d.get()) {
        throw py::value_error("expected one float match (d)");
    }

    auto bools = a->pick().allByData(true);
    if (bools.size() != 1 || bools[0].get() != e.get()) {
        throw py::value_error("expected one bool match (e)");
    }

    auto none = a->pick().allByData(999);
    if (!none.empty()) throw py::value_error("expected empty vector");
}

void test_byAnd() {
    auto a = newNode("a");
    auto b = newNode("b");
    b->attachTo(a);
    auto c = newNode("c","typeT","value");
    c->attachTo(b);
    auto d = newNode("d");
    d->attachTo(c);

    auto n = a->pick().byAnd("c","typeT","value");
    if (!n || n->name() != "c") throw py::value_error("expected extraction of node named c");
    if (n.get() != c.get()) throw py::value_error("expected to recover node c without copy");
}


void test_byAndScalar() {
    auto a = newNode("a");
    auto b = newNode("b");
    b->attachTo(a);
    auto c = newNode("c","typeT",3.14);
    c->attachTo(b);
    auto d = newNode("d");
    d->attachTo(c);

    auto n = a->pick().byAnd("c","typeT",3.14);
    if (!n || n->name() != "c") throw py::value_error("expected extraction of node named c");
    if (n.get() != c.get()) throw py::value_error("expected to recover node c without copy");
}

void test_allByAnd() {
    auto a = newNode("a");
    auto b = newNode("b");
    b->attachTo(a);
    auto c = newNode("c","typeT","value");
    c->attachTo(b);
    auto d = newNode("d","typeT","value");
    d->attachTo(c);

    auto matches = a->pick().allByAnd("", "typeT", "value");
    if (matches.size() != 2) throw py::value_error("expected 2 matches");

    if (matches[0].get() != c.get()) throw py::value_error("expected to recover node c without copy");
    if (matches[1].get() != d.get()) throw py::value_error("expected to recover node d without copy");
}
