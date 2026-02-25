# ifndef NAVIGATION_HPP
# define NAVIGATION_HPP

# include <string>
# include <memory>
# include <regex>
# include <vector>

# include "utils/template_instantiator.hpp"


class Node;

/**
 * @brief Read-only traversal and query helper for Node trees.
 *
 * Navigation methods search descendants by name, type, data, glob or regex
 * matching, and combined predicates.
 */
class Navigation {

private:
    Node& _node;

public:

    /** @brief Bind navigator to an owner node. */
    explicit Navigation(Node& ownerNode);

    /** @name Queries by node name
     *  @{
     */
    std::shared_ptr<Node> childByName(const std::string& name);

    std::shared_ptr<Node> byName(const std::string& name, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByName(const std::string& name, const size_t& depth=100);

    std::shared_ptr<Node> byNameRegex(const std::string& name_pattern, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByNameRegex(const std::string& name_pattern, const size_t& depth=100);

    std::shared_ptr<Node> byNameGlob(const std::string& name_pattern, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByNameGlob(const std::string& name_pattern, const size_t& depth=100);
    /** @} */

    /** @name Queries by node type
     *  @{
     */
    std::shared_ptr<Node> childByType(const std::string& type);

    std::shared_ptr<Node> byType(const std::string& type, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByType(const std::string& type, const size_t& depth=100);

    std::shared_ptr<Node> byTypeRegex(const std::string& name_pattern, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByTypeRegex(const std::string& name_pattern, const size_t& depth=100);

    std::shared_ptr<Node> byTypeGlob(const std::string& name_pattern, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByTypeGlob(const std::string& name_pattern, const size_t& depth=100);
    /** @} */

    /** @name Queries by payload value
     *  @{
     */
    std::shared_ptr<Node> childByData(const std::string& data);

    std::shared_ptr<Node> childByData(const char* data);

    template <typename T>
    std::shared_ptr<Node> childByData(const T& data);
    
    std::shared_ptr<Node> byData(const std::string& data, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByData(const std::string& data, const size_t& depth=100);

    std::shared_ptr<Node> byDataGlob(const std::string& data_pattern, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByDataGlob(const std::string& data_pattern, const size_t& depth=100);

    std::shared_ptr<Node> byData(const char* data, const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByData(const char* data, const size_t& depth=100);

    template <typename T>
    std::shared_ptr<Node> byData(const T& data, const size_t& depth=100);

    template <typename T>
    std::vector<std::shared_ptr<Node>> allByData(const T& data, const size_t& depth=100);
    /** @} */

    /** @name Combined predicates (name + type + data)
     *  @{
     */
    std::shared_ptr<Node> byAnd(
        const std::string& name = std::string(""),
        const std::string& type = std::string(""),
        const std::string& data = std::string(""),
        const size_t& depth=100);

    std::shared_ptr<Node> byAnd(
        const char* name = "",
        const char* type = "",
        const char* data = "",
        const size_t& depth=100);


    template <typename T>
    std::shared_ptr<Node> byAnd(
        const std::string& name = std::string(""),
        const std::string& type = std::string(""),
        const T& data = std::string(""),
        const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByAnd(
        const std::string& name = std::string(""),
        const std::string& type = std::string(""),
        const std::string& data = std::string(""),
        const size_t& depth=100);

    std::vector<std::shared_ptr<Node>> allByAnd(
        const char* name = "",
        const char* type = "",
        const char* data = "",
        const size_t& depth=100);

    template <typename T>
    std::vector<std::shared_ptr<Node>> allByAnd(
        const std::string& name = std::string(""),
        const std::string& type = std::string(""),
        const T& data = std::string(""),
        const size_t& depth=100);
    /** @} */

};

# endif 
