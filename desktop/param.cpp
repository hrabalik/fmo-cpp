#include "param.hpp"
#include <cctype>
#include <exception>
#include <iomanip>
#include <limits>
#include <sstream>

namespace wtf {

    // param_base

    param_base::param_base(const std::string& name_) : name(name_) {}
    param_base::~param_base() {}
    void param_base::addToLoader(param_group& pset) { pset.add(this); }

    // param_group

    void param_group::add(param_base* param_) {
        params.insert(std::make_pair(param_->name, param_));
    }

    bool param_group::all_valid() const {
        bool result = true;
        for (auto& pair : params) {
            auto& p = pair.second;
            if (!p->is_valid()) result = false;
        }
        return result;
    }

    void param_group::validate_throw() const {
        for (auto& pair : params) {
            auto& p = pair.second;
            if (!p->is_valid()) throw invalid_param(p->name);
        }
    }

    auto param_group::get(const std::string& name) -> param_base* {
        auto it = params.find(name);
        if (it == end(params)) return nullptr;
        return it->second;
    }

    // invalid_param

    namespace {
        std::string globalMsg;

        const std::string& store(const std::string& msg) {
            globalMsg = msg;
            return globalMsg;
        }
    }

    invalid_param::invalid_param(const std::string& name)
        : std::runtime_error(store("invalid parameter '" + name + "'")), param_name(name) {}

    auto invalid_param::what() const throw() -> const char* { return std::runtime_error::what(); }

    // readers and writers

    void command_line_read_param(int argc, const char* const* argv, param_group& out) {
        for (int i = 1; i < argc; i += 2) {
            if (argv[i][0] != '-' || argv[i][1] != '-') continue;
            std::string name;
            {
                std::istringstream argss(&argv[i][2]);
                argss >> name;
            }
            param_base* param_ptr = out.get(name);
            if (param_ptr == nullptr) continue;;
            {
                param_ptr->read(argv[i + 1]);
            }
        }
    }
}
