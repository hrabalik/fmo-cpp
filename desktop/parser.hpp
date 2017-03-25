#ifndef FMO_DESKTOP_PARSER_HPP
#define FMO_DESKTOP_PARSER_HPP

#include <forward_list>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// Allows to read parameters from command line.
struct Parser {
    using TokenIter = std::vector<std::string>::const_iterator;

    void add(const std::string& key, const char* doc, bool& val);
    void add(const std::string& key, const char* doc, int& val);
    void add(const std::string& key, const char* doc, uint8_t& val);
    void add(const std::string& key, const char* doc, float& val);
    void add(const std::string& key, const char* doc, std::string& val);
    void add(const std::string& key, const char* doc, std::vector<std::string>& val);
    void add(const std::string& key, const char* doc, std::function<void()> callback);
    void add(const std::string& key, const char* doc,
             std::function<void(const std::string&)> callback);

    void parse(const std::string& filename);
    void parse(int argc, char** argv);
    void parse(const std::vector<std::string>& tokens);

    void printHelp();

    struct Param {
        Param(const char* aDoc) : doc(aDoc) {}
        virtual ~Param() = default;
        virtual void parse(TokenIter& i, TokenIter ie) = 0;
        virtual void write(std::ostream& out, const std::string& name, char sep) const = 0;

        // data
        const char* doc;
    };

private:
    std::unordered_map<std::string, std::unique_ptr<Param>> mParams;
    int mNumFilesParsed = 0;
};

#endif // FMO_DESKTOP_PARSER_HPP
