#include "parser.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

inline void Parser::add(const std::string& key, const char* doc, FlagFunc callback) {
    mFlags.emplace_back();
    auto& param = mFlags.back();
    param.doc = doc;
    param.callback = callback;
    mParams.insert({key, &param});
}

inline void Parser::add(const std::string& key, const char* doc, IntFunc callback) {
    mInts.emplace_back();
    auto& param = mInts.back();
    param.doc = doc;
    param.callback = callback;
    mParams.insert({key, &param});
}

inline void Parser::add(const std::string& key, const char* doc, StringFunc callback) {
    mStrings.emplace_back();
    auto& param = mStrings.back();
    param.doc = doc;
    param.callback = callback;
    mParams.insert({key, &param});
}

void Parser::parse(const std::string& filename) {
    static constexpr int MAX_FILES = 32;

    if (mNumFilesParsed++ == MAX_FILES) {
        // die when too many files have been parsed to avoid potential infinite loops
        std::cerr << "exceeded the maximum of " << MAX_FILES << " files parsed in one run\n";
        throw std::runtime_error("too many files parsed");
    }

    std::vector<std::string> tokens;
    {
        std::ifstream in{filename};

        if (!in) {
            std::cerr << "failed to open parameter file " << filename << "\n";
            throw std::runtime_error("failed to open file");
        }

        std::string token;
        while (in >> std::quoted(token)) { tokens.emplace_back(token); }
    }

    try {
        parse(tokens);
    } catch (std::exception& e) {
        std::cerr << "while parsing parameters from file '" << filename << "'\n";
        throw e;
    }
}

void Parser::parse(int argc, char** argv) {
    std::vector<std::string> tokens(argv, argv + argc);

    try {
        parse(tokens);
    } catch (std::exception& e) {
        std::cerr << "while parsing parameters from command line\n";
        throw e;
    }
}

void Parser::parse(const std::vector<std::string>& tokens) {
    auto i = begin(tokens);
    auto ie = end(tokens);
    std::string key;

    while (i != ie) {
        key = *i++;
        auto found = mParams.find(key);

        if (found == end(mParams)) {
            std::cerr << "unrecognized parameter '" << key << "'\n";
            std::runtime_error("failed to parse parameter");
        }

        try {
            auto* param = found->second;
            param->parse(i, ie);
        } catch (std::exception& e) {
            std::cerr << "while parsing parameter '" << key << "'\n";
            throw e;
        }
    }
}

void Parser::FlagParam::parse(TokenIter&, TokenIter) { callback(); }

void Parser::IntParam::parse(TokenIter& i, TokenIter ie) {
    if (i == ie) {
        std::cerr << "unexpected end of input -- missing parameter value\n";
        throw std::runtime_error("unexpected end of input -- missing parameter value");
    }

    int value;
    try {
        value = std::stoi(*i++);
    } catch (std::exception& e) {
        std::cerr << "failed to read a number\n";
        throw e;
    }

    callback(value);
}

void Parser::StringParam::parse(TokenIter& i, TokenIter ie) {
    if (i == ie) {
        std::cerr << "unexpected end of input -- missing parameter value\n";
        throw std::runtime_error("unexpected end of input -- missing parameter value");
    }
    callback(*i++);
}
