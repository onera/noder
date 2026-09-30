#include "test_predicate_explorer.hpp"

#include "array/array.hpp"

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void expectMatches(
    const std::vector<std::shared_ptr<Node>>& actual,
    const std::initializer_list<std::shared_ptr<Node>>& expected,
    const std::string& context) {

    if (actual.size() != expected.size()) {
        throw py::value_error(
            context + ": expected " + std::to_string(expected.size()) +
            " matches, got " + std::to_string(actual.size()));
    }

    std::size_t index = 0;
    for (const auto& expectedNode : expected) {
        if (actual[index].get() != expectedNode.get()) {
            throw py::value_error(
                context + ": match order or identity differs at index " +
                std::to_string(index));
        }
        ++index;
    }
}

struct PredicateTree {
    std::shared_ptr<Node> root = newNode("root", "Root_t");
    std::shared_ptr<Node> branch = newNode("branch", "Branch_t");
    std::shared_ptr<Node> left = newNode("left", "DataArray_t");
    std::shared_ptr<Node> right = newNode("right", "DataArray_t");
    std::shared_ptr<Node> blade = newNode("blade", "FamilyName_t");
    std::shared_ptr<Node> deep = newNode("deep", "DataArray_t");

    PredicateTree() {
        branch->attachTo(root);
        left->setData(7);
        left->attachTo(branch);
        right->setData(8);
        right->attachTo(branch);
        blade->setData("BLADE");
        blade->attachTo(branch);
        deep->setData(12);
        deep->attachTo(left);
    }
};

} // namespace

void test_predicate_explorer_traversal() {
    PredicateTree tree;

    expectMatches(
        tree.root->pick().allByPredicate("/n:* & l:1"),
        {tree.branch},
        "inward traversal and exact level");

    expectMatches(
        tree.root->pick().allByPredicate("/l:2 & t:DataArray_t"),
        {tree.left, tree.right},
        "inward traversal and creation order");

    if (tree.root->pick().byPredicate("/n:deep").get() != tree.deep.get()) {
        throw py::value_error("byPredicate did not return the first matching node");
    }

    expectMatches(
        tree.root->pick().allByPredicate("/n:left|n:right \\t:Branch_t"),
        {tree.branch},
        "outward traversal deduplicates shared ancestors");

    expectMatches(
        tree.root->pick().allByPredicate("/n:branch /n:righ?"),
        {tree.right},
        "directional steps compose left to right");

    if (!tree.root->pick().allByPredicate("/n:missing").empty()) {
        throw py::value_error("empty predicate result was not preserved");
    }
}

void test_predicate_explorer_data_and_levels() {
    PredicateTree tree;

    expectMatches(
        tree.root->pick().allByPredicate("/d:>6"),
        {tree.left, tree.deep, tree.right},
        "numeric scalar comparison");

    expectMatches(
        tree.root->pick().allByPredicate("/l:1 & t:DataArray_t"),
        {},
        "type and level filter");

    expectMatches(
        tree.root->pick().allByPredicate("/l:2 & n:left"),
        {tree.left},
        "relative level two");

    expectMatches(
        tree.deep->pick().allByPredicate("\\l:<=2 & n:*") ,
        {tree.left, tree.branch},
        "ancestor level comparison");

    expectMatches(
        tree.root->pick().allByPredicate("/t:FamilyName_t & d:BLA*"),
        {tree.blade},
        "C1/string data glob");

    std::vector<int32_t> values(10, 1);
    auto largeArray = newNode("large", "DataArray_t");
    largeArray->setData(Array(
        ArrayTypeId::Int32,
        sizeof(int32_t),
        values.data(),
        {values.size()},
        {sizeof(int32_t)}));
    largeArray->attachTo(tree.root);
    if (!tree.root->pick().allByPredicate("/n:large & d:>0").empty()) {
        throw py::value_error("numeric predicate matched a non-scalar array");
    }
}

void test_predicate_explorer_boolean_and_quoting() {
    PredicateTree tree;
    auto literal = newNode("left&right", "A|B");
    literal->setData("A|B");
    literal->attachTo(tree.root);

    expectMatches(
        tree.root->pick().allByPredicate("/(n:left | n:blade) & t:*") ,
        {tree.left, tree.blade},
        "boolean precedence and parentheses");

    expectMatches(
        tree.root->pick().allByPredicate("/n:\"left&right\"") ,
        {literal},
        "quoted operator in name glob");

    expectMatches(
        tree.root->pick().allByPredicate("/t:\"A|B\" & d:\"A|B\"") ,
        {literal},
        "quoted operators in type and data globs");

    if (tree.root->pick().byPredicate("//n:left").get() != tree.left.get()) {
        throw py::value_error("double-direction shorthand did not match");
    }
}

void test_predicate_explorer_parser_errors() {
    PredicateTree tree;
    const std::vector<std::string> invalidExpressions = {
        "",
        "/",
        "/x:value",
        "/n:",
        "/n:left &",
        "/(n:left",
        "/l:abc",
        "/d:>abc"
    };

    for (const auto& expression : invalidExpressions) {
        bool rejected = false;
        try {
            tree.root->pick().allByPredicate(expression);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        if (!rejected) {
            throw py::value_error(
                "malformed predicate was accepted: '" + expression + "'");
        }
    }
}
