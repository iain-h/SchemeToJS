#include "scm2js.h"
#include <boost/algorithm/string.hpp>

scheme_to_javascript::scheme_to_javascript() {
    m_indents = 0;
    m_in_block.push_back(true);
    m_returns.push_back(false);
    }

scheme_to_javascript::~scheme_to_javascript() {

    }

void scheme_to_javascript::new_line() {

    m_buffer << "\n";
    for (int i = 0; i < m_indents; ++i) {
        m_buffer << "  ";
        }
    }

void scheme_to_javascript::semicolon() {

    if (m_in_block.back()) {
        m_buffer << ";";
        new_line();
        }
    }

void scheme_to_javascript::apply_identifier(const std::string& str) {

    if (str.length() == 1 && str[0] == '\n' && m_in_block.back()) {
        new_line();
        return;
        }

    std::string newStr = str;
    replace(newStr.begin(), newStr.end(), ':', '.');

    // Replace e.g. entity? with is_entity
    if (newStr[newStr.size() - 1] == '?') {
        newStr = std::string("is_").append(newStr.substr(0, newStr.size() - 1));
        }

    replace(newStr.begin(), newStr.end(), '?', '_');
    auto second = newStr.begin();
    replace(++second, newStr.end(), '-', '_');

    m_buffer << newStr;
    }

class params {
    const std::list<scheme_node*>* m_list;
    int m_start;
public:
    params(const std::list<scheme_node*>& list, int start = 1) : m_list(&list), m_start(start) {}
    friend std::ostream& operator<<(std::ostream& os, params const& rhs);
    };

bool is_comment(scheme_node* node) {
    string_node* str = dynamic_cast<string_node*>(node);
    if (str && str->m_str == "\n")
        return true;
    return dynamic_cast<comment_node*>(node) != nullptr;
    }

std::ostream& operator<<(std::ostream& os, params const& rhs) {
    auto it = rhs.m_list->begin();
    scheme_node* last_param = nullptr;
    for (; it != rhs.m_list->end(); ++it) {
        if (!is_comment(*it)) last_param = *it;
        }
    it = rhs.m_list->begin();
    for (int i = 0; i < rhs.m_start; ++i) ++it;
    for (; it != rhs.m_list->end(); ++it) {
        (*it)->apply();
        if (!is_comment(*it) && *it != last_param) {
            os << ", ";
            }
        }
    return os;
    }

void scheme_to_javascript::apply_indented(scheme_node* node) {
    ++m_indents;
    new_line();
    if (m_returns.back() && node->returns()) {
        m_buffer << "return ";
        }
    node->apply();
    --m_indents;
    }


scheme_node* find_return_lines(
    std::list<scheme_node*>::const_iterator start,
    std::list<scheme_node*>::const_iterator end ) {

    scheme_node* last = nullptr;
    auto it = start;
    while (it != end) {
        if ((*it)->returns()) {
            last = *it;
            }
        ++it;
        }

    return last;
    }

void scheme_to_javascript::apply_indented(
    std::list<scheme_node*>::const_iterator start,
    std::list<scheme_node*>::const_iterator end ) {
    ++m_indents;

    auto return_node = find_return_lines(start, end);

    if (return_node)
        m_returns.push_back(false);

    new_line();
    auto it = start;
    while (it != end) {
        auto next_it = it;
        next_it++;
        if (return_node == *it) {
            m_buffer << "return ";
            }
        (*it)->apply();
        it = next_it;
        }
    --m_indents;

    if (return_node)
        m_returns.pop_back();
    }

bool is_dot(scheme_node* node) {
    string_node* str = dynamic_cast<string_node*>(node);
    if (str && str->m_str == ".") {
        return true;
    }
    return false;
}

void scheme_to_javascript::apply_define_or_set(const std::list<scheme_node*>& list, bool is_define) {
    auto it = list.begin();
    scheme_node* a = *++it;
    scheme_node* b = *++it;

    list_node* lnode = dynamic_cast<list_node*>(a);

    if (lnode) {
        // Function definition
        string_node* fname = dynamic_cast<string_node*>(lnode->m_list.front());
        if (is_define) m_buffer << "var ";
        fname->apply();
        auto it2 = lnode->m_list.begin();
        ++it2;
        bool vargs = false;
        if (it2 != lnode->m_list.end()) {
            vargs = is_dot(*it2);
        }
        if (vargs) {
            m_buffer << " = function() {";
            new_line();
            if (it2 != lnode->m_list.end()) {
                ++it2;
                m_buffer << "  var ";
                (*it2)->apply();
                m_buffer << " = Array.from(arguments);";
            }
        } else
            m_buffer << " = function(" << params(lnode->m_list) << ") {";

        m_in_block.push_back(true);
        m_returns.push_back(true);
        new_line();
        apply_indented(it, list.end());
        new_line();
        m_in_block.pop_back();
        m_returns.pop_back();
        m_buffer << "}";
        semicolon();
        }
    else {
        lnode = dynamic_cast<list_node*>(b);
        if (is_define) m_buffer << "var ";
        a->apply();
        m_buffer << " = ";
        m_in_block.push_back(false);
        b->apply();
        m_in_block.pop_back();
        semicolon();
        }
    }

void scheme_to_javascript::apply_define(const std::list<scheme_node*>& list) {
    apply_define_or_set(list, true);
    }

void scheme_to_javascript::apply_set(const std::list<scheme_node*>& list) {
    apply_define_or_set(list, false);
    }

void scheme_to_javascript::apply_if(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* a = *++it;
    scheme_node* b = *++it;

    m_buffer << "if (";
    m_in_block.push_back(false);
    a->apply();
    m_in_block.pop_back();
    m_buffer << ") {";
    m_in_block.push_back(true);
    apply_indented(b);
    new_line();
    m_in_block.pop_back();
    m_buffer << "}";
    if (++it != list.end()) {
        scheme_node* c = *it;
        m_buffer << " else {";
        m_in_block.push_back(true);
        apply_indented(c);
        new_line();
        m_in_block.pop_back();
        m_buffer << "}";
        new_line();
        }
    }

void scheme_to_javascript::apply_array(const std::list<scheme_node*>& list) {
    
    std::list<scheme_node*> list_copy = list;
    auto it = list_copy.begin();
    ++it;
    scheme_node* fname = *it;

    string_node* strNode = dynamic_cast<string_node*>(fname);
    if (strNode) {
        std::string str = strNode->m_str;
        if (str.compare(0, 4, "smi-") == 0 && str.find(':') != -1) {
            ++it;
            if (it != list_copy.end()) {
                string_node* strNode2 = dynamic_cast<string_node*>(*it);
                if (strNode2) {
                    strNode->m_str = strNode2->m_str + ".";
                    bool afterColon = false;
                    for (size_t i = 0; i < str.length(); ++i) {
                        if (str[i] == ':') afterColon = true;
                        else if (afterColon) {
                            if (str[i] == '-') strNode->m_str += '_';
                            else strNode->m_str += str[i];
                        }
                    }
                    list_copy.erase(it);
                }
            }
        }
    }

    m_in_block.push_back(false);
    m_buffer << "[";
    m_buffer << params(list_copy);
    m_buffer << "]";
    m_in_block.pop_back();
    semicolon();
    }

void scheme_to_javascript::apply_index_operator(const std::list<scheme_node*>& list) {
    auto it = list.begin();

    scheme_node* a = *++it;
    scheme_node* b = *++it;

    m_in_block.push_back(false);
    a->apply();
    m_buffer << "[";
    b->apply();
    m_buffer << "]";
    m_in_block.pop_back();
    semicolon();
    }

void scheme_to_javascript::apply_length(const std::list<scheme_node*>& list) {
    auto it = list.begin();

    scheme_node* a = *++it;
    scheme_node* b = *++it;

    m_in_block.push_back(false);
    a->apply();
    m_buffer << ".length";
    m_in_block.pop_back();
    semicolon();
    }

void scheme_to_javascript::apply_car(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* a = *++it;
    m_in_block.push_back(false);
    a->apply();
    m_buffer << "[0]";
    m_in_block.pop_back();
    semicolon();
    }

void scheme_to_javascript::apply_cdr(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* a = *++it;
    m_in_block.push_back(false);
    a->apply();
    m_buffer << ".slice(1)";
    m_in_block.pop_back();
    semicolon();
    }

void scheme_to_javascript::apply_cadr(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* a = *++it;
    m_in_block.push_back(false);
    a->apply();
    m_buffer << "[1]";
    m_in_block.pop_back();
    semicolon();
    }

void scheme_to_javascript::apply_cons(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* a = *++it;
    scheme_node* b = *++it;
    m_in_block.push_back(false);
    m_buffer << "[";
    a->apply();
    m_buffer << ", ";
    b->apply();
    m_buffer << "]";
    m_in_block.pop_back();
    semicolon();
    }

void scheme_to_javascript::apply_map(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    assert(list.size() == 3);
    scheme_node* a = *++it;
    scheme_node* b = *++it;
    m_in_block.push_back(false);
    b->apply();
    m_in_block.pop_back();
    m_buffer << ".forEach(";
    m_in_block.push_back(true);
    if (dynamic_cast<string_node*>(a)) {
        m_buffer << "x => ";
        a->apply();
        m_buffer << "(x)";
        }
    else
        a->apply();
    m_in_block.pop_back();
    m_buffer << ")";
    semicolon();
    }

void scheme_to_javascript::apply_lambda(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    
    scheme_node* args = *++it;
    scheme_node* b = *++it;
    m_in_block.push_back(false);
    m_buffer << "(";
    list_node* lnode = dynamic_cast<list_node*>(args);
    if (lnode)
        m_buffer << params(lnode->m_list, 0);
    m_in_block.pop_back();
    m_buffer << ") => {";
    m_in_block.push_back(true);
    apply_indented(it, list.end());
    m_in_block.pop_back();
    m_buffer << "}";
    }

scheme_node* first_item(scheme_node* node) {
    list_node* args;
    args = dynamic_cast<list_node*>(node);
    if (args && !args->m_list.empty()) {
        return  args->m_list.front();
    }
    return nullptr;
}

scheme_node* second_item(scheme_node* node) {
    list_node* args;
    args = dynamic_cast<list_node*>(node);
    if (args && args->m_list.size() > 1) {
        return *(++args->m_list.begin());
    }
    return nullptr;
}

void scheme_to_javascript::apply_let(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* a = *++it;
    scheme_node* b = *++it;

    string_node lambda_node((*it)->m_parent, "lambda");
    list_node temp_args((*it)->m_parent);
    list_node temp_values((*it)->m_parent);
    list_node* args;
    args = dynamic_cast<list_node*>(a);
    if (args) {
        auto args_list = args->m_list;
        for (auto it2 = args_list.begin(); it2 != args_list.end(); ++it2) {
            temp_args.m_list.push_back(first_item(*it2)->make_copy());
            temp_values.m_list.push_back(second_item(*it2)->make_copy());
        }
    }

    std::list<scheme_node*> lambda_list;
    lambda_list.push_back(&lambda_node);
    lambda_list.push_back(&temp_args);
    for (auto it2 = it; it2 != list.end(); ++it2) {
        lambda_list.push_back(*it2);
    }
    m_buffer << "(";
    apply_lambda(lambda_list);
    m_buffer << ")(" << params(temp_values.m_list, 0) << ")";

    semicolon();
    }

void scheme_to_javascript::apply_not(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* a = *++it;
    list_node* lnode = dynamic_cast<list_node*>(a);
    if (lnode && lnode->is_operator()) {
        string_node* op = dynamic_cast<string_node*>(*lnode->m_list.begin());
        if (op && op->m_str == "=") {
            op->m_str = "!==";
            apply_operator(lnode->m_list);
        } else {
            m_buffer << "!(";
            a->apply();
            m_buffer << ")";
        }
        }
    else {
        m_buffer << "!";
        a->apply();
        }
    semicolon();
    }

void scheme_to_javascript::apply_begin(const std::list<scheme_node*>& list) {

    auto it = list.begin();
    ++it;
    scheme_node* returns = nullptr;
    if (m_returns.back()) {
        returns = find_return_lines(it, list.end());
        }
    for (; it != list.end(); ++it) {
        if (returns == *it) {
            m_buffer << "return ";
            }
        (*it)->apply();
        }
    }

void scheme_to_javascript::apply_function_call(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* fname = *it;

    string_node* strNode = dynamic_cast<string_node*>(fname);
    if (strNode) {
        std::string& str = strNode->m_str;
        if (str.compare(0, 4, "smi-") == 0 && str.find(':') != -1) {
            auto smiObj = *(++it);
            smiObj->apply();
            m_buffer << ".";
            bool afterColon = false;
            for (size_t i = 0; i < str.length(); ++i) {
                if (str[i] == ':') afterColon = true;
                else if (afterColon) {
                    if (str[i] == '-') m_buffer << '_';
                    else m_buffer << str[i];
                    }
                }
            m_buffer << "(";
            m_in_block.push_back(false);
            m_buffer << params(list, 2);
            m_in_block.pop_back();
            m_buffer << ")";
            semicolon();
            return;
            }
        }

    fname->apply();
    m_buffer << "(";
    m_in_block.push_back(false); 
    m_buffer << params(list);
    m_in_block.pop_back();
    m_buffer << ")";
    semicolon();
    }

void scheme_to_javascript::apply_operator_name(scheme_node* op) {
    string_node* node = dynamic_cast<string_node*>(op);
    if (!node) {
        return;
        }
    const std::string& str = node->m_str;
    if (str.compare("or") == 0) m_buffer << "||";
    else if (str.compare("and") == 0) m_buffer << "&&";
    else if (str.compare("=") == 0) m_buffer << "===";
    else m_buffer << str;
    }

void scheme_to_javascript::apply_maybe_brackets(scheme_node* node) {

    list_node* lnode = dynamic_cast<list_node*>(node);
    if (lnode && lnode->is_logic_operator()) {
        m_buffer << "(";
        node->apply();
        m_buffer << ")";
        }
    else
        node->apply();
    }

void scheme_to_javascript::apply_operator(const std::list<scheme_node*>& list) {
    auto it = list.begin();
    scheme_node* op = *it;
    scheme_node* lhs = *++it;
    scheme_node* rhs = *++it;
    apply_maybe_brackets(lhs);
    m_buffer << " ";
    apply_operator_name(op);
    m_buffer << " ";
    apply_maybe_brackets(rhs);
    semicolon();
    }

void scheme_to_javascript::apply_comment(const std::string& str) {
    std::string newStr = str;
    replace(newStr.begin(), newStr.end(), ';', '/');
    boost::replace_all(newStr, "Scheme", "JavaScript");
    m_buffer << "/" << newStr;
    new_line();
    }

void scheme_to_javascript::apply_string(const std::string& str) {
    std::string newStr = str;

    if (newStr[0] == '\'') {
        newStr += "'";
        }

    m_buffer << newStr;
    }

std::string scheme_to_javascript::get_string() {

    return m_buffer.str();
    }

void scheme_to_javascript::apply_bool(bool val) {
    if (val) {
        m_buffer << "true";
        }
    else {
        m_buffer << "false";
        }
    }


void scheme_to_javascript::apply_quote(const std::list<scheme_node*>& list) {

    if (list.size() != 1) return;

    auto it = list.begin();

    if (dynamic_cast<string_node*>(*it)) {
        m_buffer << "'";
        (*it)->apply();
        m_buffer << "'";
        }
    else {
        list_node* lnode = dynamic_cast<list_node*>(*it);
        if (lnode) {
            
            list_node temp_list((*it)->m_parent);
            temp_list.m_type = list_node::literal_list_t;
            auto it2 = lnode->m_list.begin();
            for (; it2 != lnode->m_list.end(); ++it2) {
                if (dynamic_cast<list_node*>(*it2)) {
                    auto temp_quote = new list_node(&temp_list);
                    temp_quote->m_type = list_node::quote_t;
                    temp_quote->m_list.push_back((*it2)->make_copy());
                    temp_list.m_list.push_back(temp_quote);
                } else {
                    temp_list.m_list.push_back((*it2)->make_copy());
                }
            }
            temp_list.apply();
        } else {
            (*it)->apply();
        }
    }
    semicolon();
}


void scheme_to_javascript::apply_list(list_node& node) {

    switch (node.type()) {
        case list_node::define_t:
            apply_define(node.m_list);
            break;
        case list_node::if_t:
            apply_if(node.m_list);
            break;
        case list_node::array_t:
            apply_array(node.m_list);
            break;
        case list_node::index_t:
            apply_index_operator(node.m_list);
            break;
        case list_node::length_t:
            apply_length(node.m_list);
            break;
        case list_node::car_t:
            apply_car(node.m_list);
            break;
        case list_node::cdr_t:
            apply_cdr(node.m_list);
            break;
        case list_node::cadr_t:
            apply_cadr(node.m_list);
            break;
        case list_node::cons_t:
            apply_cons(node.m_list);
            break;
        case list_node::map_t:
            apply_map(node.m_list);
            break;
        case list_node::lambda_t:
            apply_lambda(node.m_list);
            break;
        case list_node::begin_t:
            apply_begin(node.m_list);
            break;
        case list_node::call_t:
            apply_function_call(node.m_list);
            break;
        case list_node::set_t:
            apply_set(node.m_list);
            break;
        case list_node::let_t:
            apply_let(node.m_list);
            break;
        case list_node::not_t:
            apply_not(node.m_list);
            break;
        case list_node::quote_t:
            apply_quote(node.m_list);
            break;
        case list_node::operator_t:
            apply_operator(node.m_list);
            break;
        case list_node::literal_list_t:
            node.m_list.push_front(new string_node(node.m_parent, "list"));
            apply_array(node.m_list);
            break;
       }
    }
