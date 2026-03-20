#ifndef IO_YAML_NATIVE_NODER_YAML_IO_HPP
#define IO_YAML_NATIVE_NODER_YAML_IO_HPP

#include "node/node.hpp"

#include <memory>
#include <string>

namespace io::yaml {

void write_node(const std::string& filename, std::shared_ptr<Node> node);
std::shared_ptr<Node> read(const std::string& filename);

} // namespace io::yaml

#endif // IO_YAML_NATIVE_NODER_YAML_IO_HPP
