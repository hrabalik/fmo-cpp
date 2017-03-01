#include "param.hpp"
#include <cctype>
#include <exception>
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

    namespace {
        void param_read(std::istream& in, char param_separator, char name_value_separator,
                        param_group& out) {
            // read name
            std::string name;
            int c = in.get();

            while (in.good() && c != param_separator && c != name_value_separator) {
                if (std::isspace(c)) continue; // ignore whitespace
                name.push_back(static_cast<char>(c));
                c = in.get();
            }

            // check that loop ended with the name-value separator
            if (!in.good()) {
                return;
            } else if (c == param_separator) {
                in.putback(static_cast<char>(c));
                return;
            }

            // find param by name, read value
            param_base* param_ptr = out.get(name);
            if (param_ptr == nullptr) return;
            param_ptr->read(in);
        }
    }

    void command_line_read_param(int argc, const char* const* argv, param_group& out) {
        for (int i = 1; i < argc; ++i) {
            if (argv[i][0] != '-' || argv[i][1] != '-') continue;
            std::istringstream argss(&argv[i][2]);
            param_read(argss, '\0', '=', out);
        }
    }

    void stream_read_param(std::istream& in, char param_separator, char name_value_separator,
                           param_group& out) {
        while (in.good()) {
            param_read(in, param_separator, name_value_separator, out);
            in.ignore(std::numeric_limits<std::streamsize>::max(), param_separator);
        }
    }

    void stream_write_param(std::ostream& out, char param_separator, char name_value_separator,
                            const param_group& in) {
        for (auto& pair : in.params) {
            auto& param_name = pair.first;
            auto* param_ptr = pair.second;
            out << param_name << name_value_separator;
            param_ptr->write(out);
            out << param_separator;
        }
    }
}
