#pragma once
#ifndef WTF_COGS_UTILITY_PARAM
#define WTF_COGS_UTILITY_PARAM

#include <functional>
#include <initializer_list>
#include <iostream>
#include <map>
#include <string>

namespace wtf {

    class param_group;

    struct param_base {
        param_base(const std::string& name_);
        void addToLoader(param_group& pset);

        virtual ~param_base();
        virtual void read(std::istream& in) = 0;
        virtual void write(std::ostream& out) const = 0;
        virtual bool is_valid() const = 0;

        // data
        const std::string name;
    };

    template <typename T>
    struct param : public param_base {
        using validator_t = std::function<bool(const T&)>;
        static bool dummy_validator(const T&) { return true; }

        param(const std::string& name, const T& initial = T(),
              const validator_t& valid = dummy_validator);
        param(const std::string& name, const T& initial, param_group& pset,
              const validator_t& valid = dummy_validator);

        void read(std::istream& in) override;
        void write(std::ostream& out) const override;
        bool is_valid() const override;

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

    // reads command line arguments in the form "-name=value", skips argv[0]
    void command_line_read_param(int argc, const char* const* argv, param_group& out);

    // simple generic parameter reader
    void stream_read_param(std::istream& in, char param_separator, char name_value_separator,
                           param_group& out);

    // simple generic parameter writer
    void stream_write_param(std::ostream& out, char param_separator, char name_value_separator,
                            const param_group& in);

    // param implementation
    template <typename T>
    param<T>::param(const std::string& name, const T& initial, const validator_t& valid)
        : param_base(name), value(initial), validator(valid) {}

    template <typename T>
    param<T>::param(const std::string& name, const T& initial, param_group& pset,
                    const validator_t& valid)
        : param(name, initial, valid) {
        addToLoader(pset);
    }

    template <typename T>
    void param<T>::write(std::ostream& out) const {
        out << value;
    }

    template <typename T>
    void param<T>::read(std::istream& in) {
        in >> value;
    }

    template <typename T>
    bool param<T>::is_valid() const {
        return validator(value);
    }

} // namespace wtf

#endif
