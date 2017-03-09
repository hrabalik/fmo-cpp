#include "parser.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

void Parser::add(const std::string& key, const char* doc, FlagFunc callback) {
    mFlags.emplace_front();
    auto& param = mFlags.front();
    param.doc = doc;
    param.callback = callback;
    mParams.insert({key, &param});
}

void Parser::add(const std::string& key, const char* doc, IntFunc callback) {
    mInts.emplace_front();
    auto& param = mInts.front();
    param.doc = doc;
    param.callback = callback;
    mParams.insert({key, &param});
}

void Parser::add(const std::string& key, const char* doc, StringFunc callback) {
    mStrings.emplace_front();
    auto& param = mStrings.front();
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
    std::vector<std::string> tokens(argv + 1, argv + argc);

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

void Parser::printHelp() {
    static constexpr int COLUMNS = 80;
    auto& out = std::cerr;
    std::vector<std::string> keys;
    for (auto& entry : mParams) { keys.push_back(entry.first); }
    int maxKeyLen = 0;
    for (auto& key : keys) { maxKeyLen = std::max(maxKeyLen, int(key.size())); }

    std::sort(begin(keys), end(keys));
    const int pad = maxKeyLen;
    const int maxWidth = COLUMNS - pad;
    std::string word;
    std::vector<std::string> tokens;

    for (auto& key : keys) {
        // write key name
        out << key;
        int spaces = pad - int(key.size());
        for (int i = 0; i < spaces; i++) { out << ' '; }
        int widthRemaining = maxWidth;

        // split doc string into words
        std::istringstream iss(mParams[key]->doc);
        tokens.clear();
        while (iss >> word) { tokens.push_back(word); }

        // put words
        for (auto& token : tokens) {
            int len = int(token.size()) + 1;

            // break line if out of space
            if (len > widthRemaining) {
                out << '\n';
                for (int i = 0; i < pad; i++) { out << ' '; }
                widthRemaining = maxWidth;
            }

            out << ' ' << token;
            widthRemaining -= len;
        }

        // endline
        out << '\n';
    }
}
