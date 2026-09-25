#ifdef ENABLE_HDF5_IO

#include "apps/cgnsviz/terminal_view/terminal_model.hpp"
#include "io/hdf5/lazycgns/lazy_hdf5_reader.hpp"

#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

struct Options {
    std::string filename;
    char order = 'F';
    std::size_t pageSize = 5;
    std::size_t payloadLimit = 64;
    bool interactive = true;
};

std::size_t parseSize(const std::string& value, const char* option) {
    try {
        const std::size_t parsed = std::stoull(value);
        if (parsed == 0) {
            throw std::invalid_argument("must be greater than zero");
        }
        return parsed;
    } catch (const std::exception& error) {
        throw std::invalid_argument(
            std::string("invalid value for ") + option + ": " + error.what());
    }
}

Options parseOptions(int argc, char** argv) {
    if (argc < 2) {
        throw std::invalid_argument(
            "usage: cgnsviz FILE [--order C|F] [--page-size N] [--payload-limit N] [--non-interactive]");
    }

    Options options;
    options.filename = argv[1];
    for (int index = 2; index < argc; ++index) {
        const std::string option = argv[index];
        if (option == "--order") {
            if (index + 1 >= argc) {
                throw std::invalid_argument("--order requires C or F");
            }
            const std::string value = argv[++index];
            if (value.size() != 1 || (value[0] != 'C' && value[0] != 'c' && value[0] != 'F' && value[0] != 'f')) {
                throw std::invalid_argument("--order requires C or F");
            }
            options.order = value[0];
        } else if (option == "--page-size") {
            if (index + 1 >= argc) {
                throw std::invalid_argument("--page-size requires an integer");
            }
            options.pageSize = parseSize(argv[++index], "--page-size");
        } else if (option == "--payload-limit") {
            if (index + 1 >= argc) {
                throw std::invalid_argument("--payload-limit requires an integer");
            }
            options.payloadLimit = parseSize(argv[++index], "--payload-limit");
        } else if (option == "--non-interactive") {
            options.interactive = false;
        } else if (option == "--help" || option == "-h") {
            throw std::invalid_argument(
                "usage: cgnsviz FILE [--order C|F] [--page-size N] [--payload-limit N] [--non-interactive]");
        } else {
            throw std::invalid_argument("unknown option: " + option);
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
            std::cout << "usage: cgnsviz FILE [--order C|F] [--page-size N] [--payload-limit N] [--non-interactive]\n";
            return 0;
        }
        const Options options = parseOptions(argc, argv);
        auto reader = std::make_shared<io::hdf5::cgns::LazyHdf5Reader>(options.filename, options.order);
        cgnsviz::terminal::TerminalModel model(reader, options.pageSize, options.payloadLimit);
        if (!options.interactive) {
            return cgnsviz::terminal::runTerminal(model, std::cin, std::cout);
        }
        return cgnsviz::terminal::runInteractiveTerminal(model);
    } catch (const std::exception& error) {
        std::cerr << "cgnsviz: " << error.what() << "\n";
        return 1;
    }
}

#else

#include <iostream>

int main() {
    std::cerr << "cgnsviz was built without HDF5 support.\n";
    return 1;
}

#endif // ENABLE_HDF5_IO
