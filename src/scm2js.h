
#include "scm_parser.h"
#include <list>
#include <sstream>

class scheme_to_javascript : public scheme_translator {

    std::stringstream m_buffer;
    int               m_indents;
    std::list<bool>   m_in_block;
    std::list<bool>   m_returns;

    void apply_indented(scheme_node* node);
    void apply_indented(
        std::list<scheme_node*>::const_iterator start,
        std::list<scheme_node*>::const_iterator end);
    void new_line();
    void semicolon();
    void apply_define_or_set(const std::list<scheme_node*>& list, bool is_define);
    void apply_operator_name(scheme_node* op);
    void apply_maybe_brackets(scheme_node* op);

    void apply_define(const std::list<scheme_node*>& list);
    void apply_if(const std::list<scheme_node*>& list);
    void apply_array(const std::list<scheme_node*>& list);
    void apply_index_operator(const std::list<scheme_node*>& list);
    void apply_length(const std::list<scheme_node*>& list);
    void apply_car(const std::list<scheme_node*>& list);
    void apply_cdr(const std::list<scheme_node*>& list);
    void apply_cadr(const std::list<scheme_node*>& list);
    void apply_cons(const std::list<scheme_node*>& list);
    void apply_map(const std::list<scheme_node*>& list);
    void apply_lambda(const std::list<scheme_node*>& list);
    void apply_begin(const std::list<scheme_node*>& list);
    void apply_function_call(const std::list<scheme_node*>& list);
    void apply_operator(const std::list<scheme_node*>& list);
    void apply_set(const std::list<scheme_node*>& list);
    void apply_let(const std::list<scheme_node*>& list);
    void apply_not(const std::list<scheme_node*>& list);
    void apply_quote(const std::list<scheme_node*>& list);

public:

    scheme_to_javascript();
    ~scheme_to_javascript();

    void apply_identifier(const std::string& str) override;
    void apply_list(list_node& node) override;
    void apply_comment(const std::string& str) override;
    void apply_string(const std::string& str) override;
    void apply_bool(bool val) override;

    std::string get_string() override;
    };
