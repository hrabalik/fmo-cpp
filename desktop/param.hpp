#pragma once
#ifndef WTF_COGS_UTILITY_PARAM
#define WTF_COGS_UTILITY_PARAM

#include <functional>
#include <initializer_list>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace wtf {

    class param_group;

    struct param_base {
        param_base(const std::string& name_);
        void addToLoader(param_group& pset);

        virtual ~param_base();
        virtual void read(const std::string& in) = 0;
        // virtual void write(std::ostream& out) const = 0;
        virtual bool is_valid() const = 0;

        // data
        const std::string name;
    };

    template <typename T>
    struct param_impl : public param_base {
        using validator_t = std::function<bool(const T&)>;
        static bool dummy_validator(const T&) { return true; }

        param_impl(const std::string& name, const T& initial = T(),
                   const validator_t& valid = dummy_validator);
        param_impl(const std::string& name, const T& initial, param_group& pset,
                   const validator_t& valid = dummy_validator);

        // void write(std::ostream& out) const override;
        bool is_valid() const override { return validator(value); }

        // data
        T value;
        validator_t validator;
    };

    struct invalid_param : public std::runtime_error {
        invalid_param(const std::string& name);
        virtual const char* what() const throw() override;
        const std::string param_name;
    };

    class param_group {
    public:
        void add(param_base* param_);
        bool all_valid() const;
        void validate_throw() const;
        param_base* get(const std::string& name);

        // data
        std::map<std::string, param_base*> params;
    };

    // param_impl implementation
    template <typename T>
    param_impl<T>::param_impl(const std::string& name, const T& initial, const validator_t& valid)
        : param_base(name), value(initial), validator(valid) {}

    template <typename T>
    param_impl<T>::param_impl(const std::string& name, const T& initial, param_group& pset,
                              const validator_t& valid)
        : param_impl(name, initial, valid) {
        addToLoader(pset);
    }

    template <typename T>
    struct param : param_impl<T> {
        using param_impl::param_impl;

        void read(const std::string& in) override {
            std::istringstream iss(in);
            iss >> value;
        }
    };

    template <>
    struct param<std::string> : param_impl<std::string> {
        using param_impl::param_impl;

        void read(const std::string& in) override {
            value = in;
        }
    };

    // reads command line arguments in the form "--name value", skips argv[0]
    void command_line_read_param(int argc, const char* const* argv, param_group& out);

} // namespace wtf

#endif
