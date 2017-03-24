#include "parser.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

template <typename T>
struct ParamImplBase : public Parser::Param {
    ParamImplBase(const char* aDoc, T aVal) : Param(aDoc), val(aVal) {}

protected:
    static void testHaveToken(Parser::TokenIter& i, Parser::TokenIter ie) {
        if (i == ie) {
            std::cerr << "unexpected end of input -- missing parameter value\n";
            throw std::runtime_error("unexpected end of input -- missing parameter value");
        }
    }

    // data
    T val;
};

struct FlagParam : public ParamImplBase<bool*> {
    using ParamImplBase::ParamImplBase;
    using ParamImplBase::val;

    virtual void parse(Parser::TokenIter&, Parser::TokenIter) override { *val = true; }
};

struct IntParam : public ParamImplBase<int*> {
    using ParamImplBase::ParamImplBase;
    using ParamImplBase::val;

    virtual void parse(Parser::TokenIter& i, Parser::TokenIter ie) override {
        testHaveToken(i, ie);
        try {
            *val = std::stoi(*i++);
        } catch (std::exception& e) {
            std::cerr << "failed to read an integer\n";
            throw e;
        }
    }
};

struct Uint8Param : public ParamImplBase<uint8_t*> {
    using ParamImplBase::ParamImplBase;
    using ParamImplBase::val;

    virtual void parse(Parser::TokenIter& i, Parser::TokenIter ie) override {
        testHaveToken(i, ie);
        try {
            int got = std::stoi(*i++);

            if (got < 0 || got > 255) {
                std::cerr << got << " is not in range 0..255\n";
                throw std::runtime_error("integer out of range");
            }

            *val = uint8_t(got);
        } catch (std::exception& e) {
            std::cerr << "failed to read an 8-bit unsigned integer\n";
            throw e;
        }
    }
};

struct FloatParam : public ParamImplBase<float*> {
    using ParamImplBase::ParamImplBase;
    using ParamImplBase::val;

    virtual void parse(Parser::TokenIter& i, Parser::TokenIter ie) override {
        testHaveToken(i, ie);
        try {
            *val = std::stof(*i++);
        } catch (std::exception& e) {
            std::cerr << "failed to read a floating-point number\n";
            throw e;
        }
    }
};

struct StringParam : public ParamImplBase<std::string*> {
    using ParamImplBase::ParamImplBase;
    using ParamImplBase::val;

    virtual void parse(Parser::TokenIter& i, Parser::TokenIter ie) override {
        testHaveToken(i, ie);
        *val = *i++;
    }
};

struct StringListParam : public ParamImplBase<std::vector<std::string>*> {
    using ParamImplBase::ParamImplBase;
    using ParamImplBase::val;

    virtual void parse(Parser::TokenIter& i, Parser::TokenIter ie) override {
        testHaveToken(i, ie);
        val->emplace_back(*i++);
    }
};

struct CallbackParam : public ParamImplBase<std::function<void()>> {
    using ParamImplBase::ParamImplBase;
    using ParamImplBase::val;

    virtual void parse(Parser::TokenIter&, Parser::TokenIter) override { val(); }
};

struct CallbackStringParam : public ParamImplBase<std::function<void(const std::string&)>> {
    using ParamImplBase::ParamImplBase;
    using ParamImplBase::val;

    virtual void parse(Parser::TokenIter& i, Parser::TokenIter ie) override {
        testHaveToken(i, ie);
        val(*i++);
    }
};

void Parser::add(const std::string& key, const char* doc, bool& val) {
    mParams.emplace(key, new FlagParam(doc, &val));
}

void Parser::add(const std::string& key, const char* doc, int& val) {
    mParams.emplace(key, new IntParam(doc, &val));
}

void Parser::add(const std::string& key, const char* doc, uint8_t& val) {
    mParams.emplace(key, new Uint8Param(doc, &val));
}

void Parser::add(const std::string& key, const char* doc, float& val) {
    mParams.emplace(key, new FloatParam(doc, &val));
}

void Parser::add(const std::string& key, const char* doc, std::string& val) {
    mParams.emplace(key, new StringParam(doc, &val));
}

void Parser::add(const std::string& key, const char* doc, std::vector<std::string>& val) {
    mParams.emplace(key, new StringListParam(doc, &val));
}

void Parser::add(const std::string& key, const char* doc, std::function<void()> callback) {
    mParams.emplace(key, new CallbackParam(doc, callback));
}

void Parser::add(const std::string& key, const char* doc,
                 std::function<void(const std::string&)> callback) {
    mParams.emplace(key, new CallbackStringParam(doc, callback));
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
            throw std::runtime_error("failed to parse parameter");
        }

        try {
            auto& param = found->second;
            param->parse(i, ie);
        } catch (std::exception& e) {
            std::cerr << "while parsing parameter '" << key << "'\n";
            throw e;
        }
    }
}

void Parser::printHelp() {
    static constexpr int COLUMNS = 80;
    static constexpr int MAX_PAD = 11;
    auto& out = std::cerr;
    std::vector<std::string> keys;
    for (auto& entry : mParams) { keys.push_back(entry.first); }
    int maxKeyLen = 0;
    for (auto& key : keys) { maxKeyLen = std::max(maxKeyLen, int(key.size())); }

    std::sort(begin(keys), end(keys));
    const int pad = std::min(MAX_PAD, maxKeyLen);
    const int maxWidth = COLUMNS - pad;
    std::string word;
    std::vector<std::string> tokens;

    for (auto& key : keys) {
        // write key name
        out << key;
        int spaces = pad - int(key.size());
        for (int i = 0; i < spaces; i++) { out << ' '; }
        int widthRemaining = COLUMNS - std::max(pad, int(key.size()));

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
