
#include "scm_parser.h"
#include <iostream>
#include <map>
#include <memory>

using namespace std;

bool is_operator(const string& str) {
    if (str.compare("+") == 0)
        return true;
    if (str.compare("-") == 0)
        return true;
    if (str.compare("*") == 0)
        return true;
    if (str.compare("/") == 0)
        return true;
    if (str.compare(">") == 0)
        return true;
    if (str.compare("<") == 0)
        return true;
    if (str.compare(">=") == 0)
        return true;
    if (str.compare("<=") == 0)
        return true;
    if (str.compare("=") == 0)
        return true;
    if (str.compare("or") == 0)
        return true;
    if (str.compare("and") == 0)
        return true;
    return false;
    }

bool is_logic_operator(const string& str) {
    if (str.compare("or") == 0)
        return true;
    if (str.compare("and") == 0)
        return true;
    return false;
    }

scheme_node* string_node::make_copy() {
    return new string_node(*this);
}

void string_node::apply() {

    if (m_str.compare(0, 1, "\"") == 0) {
        m_translator->apply_string(m_str);
        }
    else if (m_str.compare(0, 1, "\'") == 0) {
        m_translator->apply_string(m_str);
        }
    else if (m_str.compare(0, 2, "#f") == 0) {
        m_translator->apply_bool(false);
        }
    else if (m_str.compare(0, 2, "#t") == 0) {
        m_translator->apply_bool(true);
        }
    else {
        m_translator->apply_identifier(m_str);
        }
    }

void comment_node::apply() {

    m_translator->apply_comment(m_str);
    }

scheme_node* comment_node::make_copy() {
    return new comment_node(*this);
}

list_node::list_type quote_node::type() {
    m_type = quote_t;
    return m_type;
    }

class root_node : public scheme_node {
public:
    std::list<scheme_node*> m_list;
    root_node(scheme_translator* parent) : scheme_node(parent) {}

    ~root_node() {
        for (auto n : m_list) {
            delete n;
            }
    }

    void apply() override {
        for (auto n : m_list) {
            n->apply();
            }
        }
    bool returns() override { return false; }
private:
    scheme_node* make_copy() override {
        return nullptr;
    }
    };

bool could_be_function(scheme_node* node) {
    string_node* str = dynamic_cast<string_node*>(node);
    if (!str)
        return false;
    char c = str->m_str[0];
    if (std::isdigit(c))
        return false;
    if (c == '"')
        return false;
    if (c == '-')
        return false;
    return true;    
}

list_node::~list_node() {
    for (auto n : m_list) {
        delete n;
    }
}

scheme_node* list_node::make_copy() {
    list_node* copy = new list_node(m_parent);
    for (auto n : m_list) {
        copy->m_list.push_back(n->make_copy());
    }
    return copy;
}

list_node::list_type list_node::type() {

    if (m_list.empty()) return unset_t;

    if (m_type != unset_t) return m_type;

    string_node* strnode = dynamic_cast<string_node*>(m_list.front());

    if (strnode) {
        string& str = strnode->m_str;
        if (str.compare("define") == 0 && m_list.size() > 2) m_type = define_t;
        else if (str.compare("if") == 0 && m_list.size() > 2) m_type = if_t;
        else if (str.compare("list") == 0) m_type = array_t;
        else if (str.compare("list-ref") == 0 && m_list.size() == 3) m_type = index_t;
        else if (str.compare("car") == 0 && m_list.size() == 2) m_type = car_t;
        else if (str.compare("cdr") == 0 && m_list.size() == 2) m_type = cdr_t;
        else if (str.compare("map") == 0 && m_list.size() == 3) m_type = map_t;
        else if (str.compare("lambda") == 0 && m_list.size() >= 3) m_type = lambda_t;
        else if (str.compare("begin") == 0 && m_list.size() > 1) m_type = begin_t;
        else if (str.compare("set!") == 0 && m_list.size() > 2) m_type = set_t;
        else if (str.compare("let") == 0 && m_list.size() > 2) m_type = let_t;
        else if (str.compare("not") == 0 && m_list.size() == 2) m_type = not_t;
        else if (str.compare("length") == 0 && m_list.size() == 2) m_type = length_t;
        else if (is_operator())  m_type = operator_t;
        else if (!could_be_function(m_list.front())) m_type = literal_list_t;
        else  m_type = call_t;
        }

    return m_type;
    }

void list_node::apply() {
    m_translator->apply_list(*this);
    }

bool list_node::is_operator() {

    string_node* str = dynamic_cast<string_node*>(m_list.front());

    if (str) {
        if (::is_operator(str->m_str) && m_list.size() == 3) {
            return true;
            }
        }
    return false;
    }

bool list_node::is_logic_operator() {

    string_node* str = dynamic_cast<string_node*>(m_list.front());

    if (str) {
        if (::is_logic_operator(str->m_str) && m_list.size() == 3) {
            return true;
            }
        }
    return false;
    }

bool list_node::returns() {
    static const std::map<int, bool> returns_map = {
            { define_t, false },
            { if_t, false },
            { array_t, true },
            { index_t, true },
            { car_t, true },
            { cdr_t, true },
            { map_t, false },
            { lambda_t, true },
            { begin_t, false },
            { set_t, false },
            { let_t, false },
            { not_t, true },
            { operator_t, true },
            { call_t, true }
        };
    auto it = returns_map.find(type());
    if (it != returns_map.end())
        return it->second;
    return false;
    }

void add_node(scheme_node* cur_node, scheme_node* new_node) {
    root_node* node = dynamic_cast<root_node*>(cur_node);
    if (node)
        node->m_list.push_back(new_node);
    else {
        list_node* node2 = dynamic_cast<list_node*>(cur_node);
        if (node2)
            node2->m_list.push_back(new_node);
        }
    }

scheme_node* add_list_node(scheme_node* cur_node) {
    list_node* new_list = new list_node(cur_node);
    add_node(cur_node, new_list);
    return new_list;
    }

void add_string_node(string& str, scheme_node* cur_node) {
    string_node* new_string = new string_node(cur_node, str);
    add_node(cur_node, new_string);
    }

void add_comment_node(string& str, scheme_node* cur_node) {
    comment_node* new_comment = new comment_node(cur_node, str);
    add_node(cur_node, new_comment);
    }

scheme_node* add_quote_node(scheme_node* cur_node) {
    quote_node* new_quote = new quote_node(cur_node);
    add_node(cur_node, new_quote);
    return new_quote;
    }


bool special_char(int c) {
    if (c == '(')
        return true;
    if (c == ')')
        return true;
    if (c == '\'')
        return true;
    return false;
    }

bool white_space(int c) {
    if (c == ' ')
        return true;
    if (c == '\t')
        return true;
    if (c == '\r')
        return true;
    if (c == '\n')
        return true;
    return false;
    }

void read_string(stringstream& buffer, string& token) {
    char c = buffer.get();
    char last = c;
    while (c != EOF) {
        token += c;
        // TODO proper escape handling
        if (c == '"' && last != '\\') {
            break;
        }
        
        last = c;
        c = buffer.get();
    }
}

string next_token(stringstream& buffer) {

    int c = buffer.get();
    while (white_space(c)) {
        c = buffer.get();
        }
    string token;
    token += c;
    if (special_char(c) || c == EOF)
        return token;

    if (c == '"') {
        read_string(buffer, token);
        return token;
    }

    if (c == ';') {
        //A comment
        c = buffer.get();
        while (c != '\n' && c != EOF) {
            token += c;
            c = buffer.get();
            }
        buffer.putback(c);
        return token;
        }

    c = buffer.get();
    while (!white_space(c) && !special_char(c) && c != EOF) {
        token += c;
        c = buffer.get();
        }
    buffer.putback(c);

    return token;
    }

root_node* scheme_translator::build_syntax_tree(std::stringstream& buffer) {

    string error;
    string token = next_token(buffer);

    root_node* root = new root_node(this);
    scheme_node* cur_node = root;
    while (token[0] != EOF) {

        if (token[0] == '(') {
            cur_node = add_list_node(cur_node);
            }
        else if (token[0] == ')') {
            cur_node = cur_node->m_parent;
            if (dynamic_cast<quote_node*>(cur_node))
                cur_node = cur_node->m_parent;
            }
        else if (token[0] == '\'') {
            cur_node = add_quote_node(cur_node);
            }
        else if (token[0] == ';') {
            add_comment_node(token, cur_node);
            }
        else {
            add_string_node(token, cur_node);
            if (dynamic_cast<quote_node*>(cur_node))
                cur_node = cur_node->m_parent;
            }

        if (!cur_node) {
            error = "Parsing error!";
            break;
            }

        //std::cout << token << "\n";
        token = next_token(buffer);
        }

    if (error.empty() && cur_node != root)
        error = "Unexpected end of input!";

    std::cout << error << "\n";
    return root;
    }

void scheme_translator::translate(stringstream& in_buffer) {

    std::unique_ptr<root_node> root(build_syntax_tree(in_buffer));

    root->apply();
    }
