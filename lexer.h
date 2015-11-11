// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

namespace ptb
{

struct lexer_error : public std::runtime_error {
    lexer_error(const std::string& w) : std::runtime_error(w) {
    }
};

struct token_info {
    size_t lineno;
    size_t start;
    size_t end;
    std::string token;
    int type;
};

class lexer
{
    bool m_is_open = false;
    std::vector<std::string> m_lines;
    std::string m_token;
    int m_type = 0;
    bool m_consumed = true;
    size_t m_line = 0;
    size_t m_position = 0;
    token_info m_tok_info;

    inline char get_char();
    inline void next_char();
    inline void back_char();
    inline char peek_next();
    inline void ignore_spaces();
    inline void ignore_comments();
    inline bool is_digit();
    inline bool is_char(char c);
    inline bool is_alpha();
    inline bool is_alnum();
    inline bool is_delim(char c);
    inline bool is_delim();

    std::unordered_map<std::string, int> m_keywords;

    typedef std::pair<size_t, size_t> local_t;

    local_t get_local() {
        return std::make_pair(m_line, m_position);
    }

    void set_local(const local_t& pos) {
        m_line = pos.first;
        m_position = pos.second;
    }

    bool check_for_keyword();
    void copy_token(size_t start);

    inline void set_type(int type) {
        m_type = type;
        m_tok_info.type = type;
    }

    void reset() {
        m_line = 0;
        m_position = 0;
        m_lines.clear();
        m_token.clear();
        m_is_open = false;
        m_consumed = true;
    }

public:
    // construtor padrão pelo compilador
    lexer();

    // Não é copiável
    lexer(const lexer&) = delete;

    int get_token();
    const token_info& get_token_info() const {
        return m_tok_info;
    }
    int peek_next_token();
    const std::string& token() const {
        return m_token;
    }

    void open(const std::string& name);
    void load_text(const std::string& text);
    bool is_open() const {
        return m_is_open;
    }

    void consume() {
        m_consumed = true;
    }
};

}
