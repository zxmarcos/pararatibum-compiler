// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <cstring>
#include <format.h>
#include "lexer.h"
#include "tokens.h"

namespace ptb
{


// Exceção lançada quando o fim do buffer de caractéres é encontrado
struct eof_except : public std::exception {};


// Retorna o caractér na posição atual do buffer
char lexer::get_char()
{
    if (m_line >= m_lines.size())
        throw eof_except();

    const std::string& line = m_lines[m_line];
    if (m_position >= line.size())
        return '\n';

    return line.at(m_position);
}


// Avança uma posição no buffer de caracteres
void lexer::next_char()
{
    if (m_line >= m_lines.size())
        throw eof_except();

    const std::string& line = m_lines[m_line];
    if (m_position >= line.size()) {
        m_line++;
        m_position = 0;
        return;
    }
    m_position++;
}


// Volta uma posição no buffer de caracteres
void lexer::back_char()
{
    if (m_position == 0) {
        if (m_line == 0)
            return;
        m_line--;
        const std::string& line = m_lines[m_line];
        m_position = line.size() - 1;
        return;
    }
    m_position--;
}


// Retorna qual será o próximo caractér no texto.
char lexer::peek_next()
{
    auto line = m_line;
    auto pos = m_position;

    next_char();
    char c;
    try {
        c = get_char();
    } catch (eof_except& e) {
        c = '\0';
    }
    m_line = line;
    m_position = pos;
    return c;
}


// Ignora espaço em branco (tabs, espaços e quebras de linhas) no buffer
void lexer::ignore_spaces()
{
    char c;
    while ((c = get_char()), std::isspace(c))
        next_char();
}


// Ignora comentários e espaços em branco
// Os comentários de linha são iniciados por #
// Os comentários de bloco são iniciados e terminados por ##
void lexer::ignore_comments()
{
    ignore_spaces();
    char c = peek_next();

    if (is_char('#')) {
        // comentários de linha
        if (c != '#') {
            while ((c = get_char()), c != '\n')
                next_char();
        }
        // comentários de bloco
        else {
            // avança as duas posições de ##
            next_char();
            next_char();
            char next = peek_next();
            while (!(is_char('#') && next == '#')) {
                next_char();
                next = peek_next();
            }
            // avança as duas posições de ##
            next_char();
            next_char();
        }
        ignore_comments();
    }
}


// Verifica se o carácter na posição atual for um digito
bool lexer::is_digit()
{
    return std::isdigit(get_char());
}


// Verifica se o caractér na posição atual do texto é c
bool lexer::is_char(char c)
{
    return c == get_char();
}


// Verifica se o carácter na posição atual é um alphanumérico
bool lexer::is_alnum()
{
    char c = get_char();
    return std::isalnum(c) || is_char('_');
}


// Verifica se o c é um delimitador
bool lexer::is_delim(char c)
{
    return strchr("[](){}+-*/<>=!&~%^|.,;:?", c) != nullptr;
}


// Verifica se o caractér na posição atual é um delimitador
bool lexer::is_delim()
{
    return is_delim(get_char());
}

// Verifica se a palavra armazenada em m_token é reservada,
// se for, seu tipo é setado.
bool lexer::check_for_keyword()
{
    // verifica se é uma palavra reservada
    if (m_keywords.find(m_token) != m_keywords.end()) {
        set_type(m_keywords[m_token]);
        return true;
    }
    return false;
}

// Copia o token do buffer de caracteres para m_token
// Passando a posição inicial do token, essa função irá copiar até a posição atual
void lexer::copy_token(size_t start)
{
    auto size = m_position - start;
    const std::string& line = m_lines[m_line];
    m_token = line.substr(start, size);
    m_tok_info.lineno = m_line;
    m_tok_info.start = start;
    m_tok_info.end = m_position;
}


lexer::lexer()
{
    m_keywords["("] = tok::l_par;
    m_keywords[")"] = tok::r_par;
    m_keywords["["] = tok::l_bracket;
    m_keywords["]"] = tok::r_bracket;
    m_keywords["{"] = tok::l_curlbracket;
    m_keywords["}"] = tok::r_curlbracket;
    m_keywords[","] = tok::comma;
    m_keywords[":"] = tok::colon;
    m_keywords[";"] = tok::semicolon;
    m_keywords["."] = tok::dot;
    m_keywords["+"] = tok::plus;
    m_keywords["-"] = tok::minus;
    m_keywords["*"] = tok::mul;
    m_keywords["/"] = tok::div;
    m_keywords["%"] = tok::mod;
    m_keywords[":="] = tok::assign;
    m_keywords["!"] = tok::neg;
    m_keywords["="] = tok::eq;
    m_keywords["!="] = tok::ne;
    m_keywords[">="] = tok::ge;
    m_keywords["<="] = tok::le;
    m_keywords[">"] = tok::gt;
    m_keywords["<"] = tok::lt;
    m_keywords["tu"] = tok::b_or;
    m_keywords["eu"] = tok::b_and;

    m_keywords["^ela"] = tok::char_;
    m_keywords["^essa"] = tok::bool_;
    m_keywords["^menino"] = tok::int_;
    m_keywords["^novinha"] = tok::string_;
    m_keywords["^deixa"] = tok::void_;
    m_keywords["^pedindo_mais"] = tok::while_;
    m_keywords["^parara"] = tok::if_;
    m_keywords["^tibum"] = tok::else_;
    m_keywords["^senta"] = tok::return_;
    m_keywords["^esqueca"] = tok::true_;
    m_keywords["^faz"] = tok::false_;
    m_keywords["^mexer_com"] = tok::read_;
    m_keywords["^mostrar"] = tok::write_;
    m_keywords["@agora_eu_vou"] = tok::main_;
    m_keywords["@novinha"] = tok::itos_;
    m_keywords["@menino"] = tok::stoi_;

}

bool lexer::is_alpha()
{
    char c = get_char();
    return std::isalpha(c) || is_char('_');
}

int lexer::get_token()
{
    // verifica se o último token foi consumido
    if (!m_consumed)
        return m_type;

    m_consumed = false;
    set_type(tok::eof);
    m_token.clear();

    try {
        ignore_comments();
        // vamos trabalhar com dois caracteres em seguida
        char ahead = peek_next();

        int digits = 0;

        // Verifica por números
        if (is_digit()) {
            auto start = m_position;

            while (is_digit()) {
                next_char();
                digits++;
            }
            copy_token(start);
            set_type(tok::integer);
        }
        // identificadores ou keywords
        else if (is_alpha()) {
            auto start = m_position;
            while (is_alnum())
                next_char();

            copy_token(start);

            // verifica se é uma palavra reservada
            if (!check_for_keyword())
                set_type(tok::identifier);
        }
        else if (is_char('^') || is_char('@')) {
            auto start = m_position;
            auto lstart = m_line;
            next_char();
            while (is_alpha()) {
                next_char();
            }

            copy_token(start);
            // verifica se é uma palavra reservada
            if (!check_for_keyword()) {
                auto em = fmt::sprintf("Palavra reservada <font color=\"red\"><b>%s</b></font>"
                                       " inválida na linha %d:%d\n",
                                       m_token.c_str(), (lstart + 1), (start + 1));
                throw lexer_error(em);
            }
        }
        else if (is_char('"')) {
            auto start = m_position;
            auto lstart = m_line;
            set_type(tok::string);

            try {
                next_char();
                while (!is_char('"'))
                    next_char();
                next_char();
            } catch (const eof_except& e) {
                auto em = fmt::sprintf("Final do arquivo inesperado.\n"
                                       "Problema na string iniciada na linha %d:%d.\n"
                                       "Provavelmente a string não foi fechada!",
                                       (lstart + 1), (start + 1));
                throw lexer_error(em);
            }

            copy_token(start);
        }
        // delimitadores
        else if (is_delim()) {
            auto start = m_position;
            auto lstart = m_line;

            switch (get_char()) {
            case '!': {
                if (ahead == '=')
                    next_char();
                break;
            }

            case ':': {
                if (ahead == '=')
                    next_char();
                break;
            }

            case '<': {
                if (ahead == '=')
                    next_char();
                break;
            }

            case '>': {
                if (ahead == '=')
                    next_char();
                break;
            }

            default:
                break;
            }

            next_char();
            copy_token(start);

            // verifica se é uma palavra reservada
            if (!check_for_keyword()) {
                auto em = fmt::sprintf("Sequência inválida `%s` na linha %d:%d.",
                                       m_token, (lstart + 1), (start + 1));
                throw lexer_error(em);
            }

        } else {
            // encontrou alguma coisa inválida...
            auto em = fmt::sprintf("Caractér inválido `%c` na linha %d:%d.",
                                   get_char(), (m_line + 1), (m_position + 1));
            throw lexer_error(em);
        }

    } catch (const eof_except& e) {
        set_type(tok::eof);
    }
    return m_type;
}


// Função que devolve a classe do próximo token no buffer de caractéres, sem
// alterar o estado do analisador léxico. 
int lexer::peek_next_token()
{
    const auto& pos = get_local();
    auto consumed = m_consumed;
    auto token = m_token;

    int type = tok::eof;
    try {
        m_consumed = true;
        type = get_token();
    } catch (const eof_except& e) {
        type = tok::eof;
    }

    m_token = token;
    m_consumed = consumed;
    set_local(pos);
    return type;
}


// Abre um arquivo de texto e copia todo seu conteúdo para o buffer
// de caractéres.
void lexer::open(const std::string& name)
{
    reset();

    // Tenta abrir o arquivo
    std::ifstream file;
    file.open(name, std::ios_base::in);
    if (!file.is_open())
        throw std::runtime_error("Não foi possível abrir o arquivo");

    std::string line;
    while (std::getline(file, line))
        m_lines.push_back(line);

    m_is_open = true;
}

void lexer::load_text(const std::string& text)
{
    reset();

    std::istringstream textstream(text);
    std::string line;
    while (std::getline(textstream, line, '\n'))
        m_lines.push_back(line);
    m_is_open = true;
}

}
