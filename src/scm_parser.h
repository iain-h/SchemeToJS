
#include <list>
#include <sstream>

class scheme_translator;

class scheme_node {
protected:
    scheme_translator* m_translator;
public:
    scheme_node* m_parent;
    scheme_node(scheme_node* parent) : m_parent(parent), m_translator(parent->m_translator) {}
    scheme_node(scheme_translator* parent) : m_parent(nullptr), m_translator(parent) {}
    virtual ~scheme_node() {}
    virtual void apply() = 0;
    virtual bool returns() = 0;
    virtual scheme_node* make_copy() = 0;
    };

class list_node : public scheme_node {

public:

    enum list_type {
        unset_t,
        define_t,
        if_t,
        array_t,
        index_t,
        car_t,
        cdr_t,
        cadr_t,
        map_t,
        lambda_t,
        begin_t,
        call_t,
        set_t,
        let_t,
        not_t,
        quote_t,
        operator_t,
        literal_list_t,
        length_t
        };

    std::list<scheme_node*> m_list;
    list_node(scheme_node* parent) : scheme_node(parent), m_type(unset_t) {}
    ~list_node();
    virtual list_type type();
    void apply() override;
    bool is_operator();
    bool is_logic_operator();
    bool returns() override;
    scheme_node* make_copy() override;

    list_type m_type;
    };

class string_node : public scheme_node {
public:
    std::string m_str;
    string_node(scheme_node* parent, std::string str) : scheme_node(parent), m_str(str) {}
    void apply() override;
    bool returns() override { return true; }
    scheme_node* make_copy() override;
    };

class comment_node : public scheme_node {
public:
    std::string m_str;
    comment_node(scheme_node* parent, std::string str) : scheme_node(parent), m_str(str) {}
    void apply() override;
    bool returns() override { return false; }
    scheme_node* make_copy() override;
    };

class quote_node : public list_node {
public:
    
    quote_node(scheme_node* parent) : list_node(parent) {}
    list_type type() override;
    bool returns() override { return true; }
    };

class root_node;

class scheme_translator {

protected:

    virtual void apply_list(list_node& node) = 0;
    virtual void apply_identifier(const std::string& str) = 0;
    virtual void apply_comment(const std::string& str) = 0;
    virtual void apply_string(const std::string& str) = 0;
    virtual void apply_bool(bool val) = 0;
    
    root_node* build_syntax_tree(std::stringstream& in_buffer);

public:
    scheme_translator* m_parent;
    virtual ~scheme_translator() {}

    void translate(std::stringstream& buffer);

    virtual std::string get_string() = 0;

    friend class string_node;
    friend class comment_node;
    friend class list_node;
    friend class quote_node;
    };
