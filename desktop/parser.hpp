#ifndef FMO_DESKTOP_PARSER_HPP
#define FMO_DESKTOP_PARSER_HPP

#include <functional>
#include <string>
#include <unordered_map>
#include <forward_list>

/// Allows to read parameters from command line.
struct Parser {
    using FlagFunc = std::function<void()>;
    using IntFunc = std::function<void(int)>;
    using StringFunc = std::function<void(const std::string&)>;
    using TokenIter = std::vector<std::string>::const_iterator;

    void add(const std::string& key, const char* doc, FlagFunc callback);
    void add(const std::string& key, const char* doc, IntFunc callback);
    void add(const std::string& key, const char* doc, StringFunc callback);

    void parse(const std::string& filename);
    void parse(int argc, char** argv);
    void parse(const std::vector<std::string>& tokens);

    void printHelp();

private:
    struct Param {
        virtual void parse(TokenIter& i, TokenIter ie) = 0;

        // data
        const char* doc;
    };

    struct FlagParam final : public Param {
        virtual void parse(TokenIter& i, TokenIter ie) override;

        // data
        FlagFunc callback;
    };

    struct IntParam final : public Param {
        virtual void parse(TokenIter& i, TokenIter ie) override;

        // data
        IntFunc callback;
    };

    struct StringParam final : public Param {
        virtual void parse(TokenIter& i, TokenIter ie) override;

        // data
        StringFunc callback;
    };

    std::forward_list<FlagParam> mFlags;
    std::forward_list<IntParam> mInts;
    std::forward_list<StringParam> mStrings;
    std::unordered_map<std::string, Param*> mParams;
    int mNumFilesParsed = 0;
};

#endif // FMO_DESKTOP_PARSER_HPP
