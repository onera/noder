#ifndef NODE_PREDICATE_EXPLORER_HPP
#define NODE_PREDICATE_EXPLORER_HPP

#include <memory>
#include <string>
#include <vector>

class Node;

/**
 * @brief Backend-neutral directional predicate queries over Node trees.
 *
 * An expression is a sequence of directional blocks. ``/`` searches
 * descendants and ``\\`` searches ancestors. Each block accepts boolean
 * predicates over node name (``n:``), type (``t:``), data (``d:``), and
 * relative traversal level (``l:``). The evaluator only depends on the
 * Node/Data abstractions, so the same implementation works for eager and
 * lazy trees.
 */
namespace predicate_explorer {

/** @brief Return every node matching an expression, in deterministic order. */
std::vector<std::shared_ptr<Node>> allByPredicate(
    Node& start,
    const std::string& expression);

/** @brief Return the first node matching an expression, or ``nullptr``. */
std::shared_ptr<Node> byPredicate(
    Node& start,
    const std::string& expression);

} // namespace predicate_explorer

#endif // NODE_PREDICATE_EXPLORER_HPP
