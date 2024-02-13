/*
 * Copyright (C) Rida Bazzi, 2020
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"

using namespace std;

// this syntax error function needs to be 
// modified to produce the appropriate message
void Parser::syntax_error()
{
    cout << "SYNTAX ERROR\n";
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type) {
        syntax_error();
    }
    return t;
}

REG* Parser::parse_expr()
{
    REG* res = nullptr;
    std::vector<REG*> series;
    std::vector<REG*> parallel;
    Token t;
    t = lexer.peek(1);
    TokenType prev = END_OF_FILE;

    while (t.token_type != COMMA && t.token_type != HASH) {
        //if (res) res->print_node();
        t = lexer.GetToken();
        if (t.token_type == LPAREN) {
            /* Left Parenthesis */
            dep.inward(lexer.peek(1).token_type);
            res = parse_expr();
            if (lexer.peek(1).token_type == HASH || lexer.peek(1).token_type == COMMA
            ) {
                if (
                    (parallel.size() == 0 && series.size() == 0)
                ) { 
                    cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
                    exit(1);
                }
            }
        }
        else if (t.token_type == RPAREN) {
            dep.outward(lexer.peek(1).token_type);
            if (!res) {
                cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
                exit(1);
            }
            /* Right Parenthesis */
            break;
        }
        else if (t.token_type == DOT) {
            /* DOT */ series.push_back(res);
        }
        else if (t.token_type == OR) {
            /* OR */ parallel.push_back(res);
        }
        else if (t.token_type == STAR) {
            if (series.size() > 0 || parallel.size() > 0) {
                cout << "SNYTAX ERORR" << endl;
                exit(1);
            }
            Token tmps;
            tmps = lexer.peek(1);
            if (
                tmps.token_type != HASH &&
                tmps.token_type != COMMA &&
                tmps.token_type != RPAREN
            ) { cout << "SNYTAX ERORR" << endl; exit(1); }
            
            dep.direc = 0;
            /* STAR */
            // (1) Make 2 RegNodes for this STAR
            RegNode* node_prev = res->create_node('_', '_');
            RegNode* node_next = res->create_node('_', '_');
            // (2) Connect 'node_prev' and 'res->start'
            node_prev->first_neighbor = res->start;
            node_prev->first_label = '_';
            node_prev->second_neighbor = node_next;
            node_prev->second_label = '_';
            // (3) Connect 'res->accept' and 'node_next'
            res->accept->first_neighbor = node_next;
            res->accept->first_label = '_';
            // (4) Connect 'res->accept' and 'res->start'
            res->accept->second_neighbor = res->start;
            res->accept->second_label = '_';
            // (5) Designate this 2 nodes as 'start', and 'accept'
            res->start = node_prev;
            res->accept = node_next;
        }
        else if (t.token_type == CHAR || t.token_type == UNDERSCORE) {
            /* CHAR */
            REG* tmp = new REG(); // Construct REG
            // Make 2 RegNodes for this CHAR
            char tchar = (t.token_type == CHAR) ? t.lexeme[0] : '_';
            RegNode* node_prev = tmp->create_node(tchar, '\0');
            RegNode* node_next = tmp->create_node('\0', '\0');
            node_prev->first_neighbor = node_next; // Connect them
            tmp->start = node_prev;
            tmp->accept = node_next;
            
            res = tmp; // Return the REG for this CHAR
        }
        else if (t.token_type == ID) {
            cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
            exit(1);
            // syntax_error();
        }
        prev = t.token_type;
        t = lexer.peek(1);
    }

    /* Merging all the other REG into one REG. */
    if (series.size() > 0 && parallel.size() > 0) { cout << "Size not matched" << endl; syntax_error(); }
    else if (series.size() > 0) {
        //res->print_node();
        if (res != nullptr) { series.push_back(res); }
        REG *tmp = new REG();
        for (auto i : series) {
            /* Concatenating series REGs into one REG. */
            if (i == series.front()) {
                tmp->start = i->start;
                tmp->set_ID(i->get_ID());
                tmp->set_line(i->get_line());
                tmp->accept = i->accept;
                tmp->nodes = i->nodes;
            }
            else {
                for (auto iter: i->nodes) { tmp->nodes.push_back(iter); }
                tmp->accept->first_neighbor = i->start;
                tmp->accept->first_label = '_';
                tmp->accept = i->accept;
            }
        }
        res = tmp;
    }
    else if (parallel.size() > 0) {
        if (res != nullptr) { parallel.push_back(res); }
        /* Tieing parallel REGs into one REG. */
        REG *tmp = new REG();
        tmp->start = tmp->create_node('_', '_');
        tmp->accept = tmp->create_node('\0', '\0');
        tmp->start->first_neighbor = parallel.back()->start;
        parallel.back()->accept->first_neighbor = tmp->accept;
        parallel.back()->accept->first_label = '_';
        for (auto tmp_reg: parallel.back()->nodes) { tmp->nodes.push_back(tmp_reg); }
        parallel.pop_back();
        tmp->start->second_neighbor = parallel.back()->start;
        parallel.back()->accept->second_neighbor = tmp->accept;
        parallel.back()->accept->second_label = '_';
        for (auto tmp_reg: parallel.back()->nodes) { tmp->nodes.push_back(tmp_reg); }
        parallel.pop_back();
        res = tmp;
    }

    if (res) { res->set_line(t.line_no); }

    //res->print_node();
    return res;
}

int Parser::parse_token()
{
    dep.reset();
    curr_reg = nullptr;
    // (1) Token ID
    Token t = lexer.GetToken();
    if (t.token_type != ID) { cout << "SNYTAX ERORR" << endl; exit(1); }
    REG* reg = nullptr;
    this->tokenID = t.lexeme;
    dep.ID = t.lexeme;
    // (2) Regular Expression
    reg = parse_expr();
    if (!reg) {
        cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
        exit(1); } 
    reg->set_ID(this->tokenID); // ID set-up
    this->check_regex(reg);

    t = lexer.peek(1);
    if (t.token_type == COMMA) { return 0; }
    else if (t.token_type == HASH) { return 1; }
    return 2;
}

int Parser::parse_token_list()
{
    while (parse_token() != 1) { expect(COMMA); continue; }
    Token t = expect(HASH);
    return 0;
}

int Parser::parse_tokens_section()
{
    parse_token_list();
    if (semantic == true) { exit(1); }
    return 0;
}

void Parser::parse_input()
{
    semantic = false;
    parse_tokens_section();
    check_epsilon();
    //for (auto i : regex) { cout << "[Check REG]" << endl; i->print_node(); }
    Token t;
    t = lexer.GetToken();
    while (t.token_type != END_OF_FILE) { 
        if (t.token_type != INPUT_TEXT) { cout << "SNYTAX ERORR" << endl; exit(1); }
        if (check_text(t) == false) { cout << "SNYTAX ERORR" << endl; exit(1); }
        match(t.lexeme);
        t = lexer.GetToken();
    }
}

bool Parser::check_text(Token& t) {
    string tmp = t.lexeme;
    if (tmp.front() != '"') { return false; }
    else if (tmp.back() != '"') { return false; }
    return true;
}

//===================================  
//  User Functions
//===================================

/*  Class Depth     */
Depth::Depth() : 
    depth(0), direc(0), inward_consecutive(false), 
    outward_consecutive(false), dual(false), ID("NULL") {}

void Depth::reset() {
    depth = 0;
    direc = 0;
    inward_consecutive = outward_consecutive = false;
    dual = false;
    ID = "NULL";
}

void Depth::inward(TokenType next) { 
    depth++;
    if (direc == 1) { 
        inward_consecutive = true;
        if (outward_consecutive == true && depth > 0
        ) { dual = true; }
    }
    else { inward_consecutive = false; outward_consecutive = false; }
    direc = 1;
}
void Depth::outward(TokenType next) { 
    depth--;
    if (direc == -1) {
        outward_consecutive = true;
        if (inward_consecutive == true && depth > 0
        ) {
            dual = true;
            //cout << "TokenType: " << next << endl;
            make_error(ID);
        }
    }
    else { outward_consecutive = false; }
    direc = -1;
}
void Depth::make_error(std::string ID) {
    cout << ID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
    exit(1);
}

/*  Class Parser    */
void Parser::check_epsilon() {
    bool sum = false;
    for (auto i: regex) {
        if (check_epsilon_reg(i) == true) {
            if (sum == false) { cout << "EPSILON IS NOOOOOOOT A TOKEN !!! "; }
            sum = true;
            cout << i->get_ID() << " ";
        }
    }
    if (sum == true) { cout << endl; exit(1); }
    return;
}
bool Parser::check_epsilon_reg(REG* reg) {
    int res = reg->check_epsilon_node(reg->start);
    if (res > 0) { return true; }
    return false;
}

void Parser::check_regex(REG* reg) {
    for (auto i : regex) {
        // (1) Have the same ID
        if (i->get_ID() == reg->get_ID()) {
            cout << "Line " << reg->get_line() << ": " << 
            i->get_ID() << " already declared on line " << i->get_line() << endl;
            semantic = true;
            return;
        }
    }
    this->regex.push_back(reg);
}

void Parser::print_match() {
    for (int i = 0; i < real_accepted.size(); i++) {
        cout << real_accepted.at(i)->get_ID() << ", "; // Print token name
        cout << '"' << match_str.at(i) << '"' << endl;
    }
}

void Parser::match(std::string raw_str) {
    // (1) At first, all regex are available to match string.
    curr_accepted = regex;
    real_accepted.clear(); // Reset "real_accepted"
    int len = raw_str.length() - 2; // Except for quotation marks
    string str = raw_str.substr(1, len); // Except for quotation marks
    vector<string> strs; // Substrings splited by whitespaces
    //cout << "Original match: " << str << endl;
    split_string(str, strs, ' '); // Spliting by whitespaces
    //for (auto iter: strs) { cout << "|" << iter << "|" << endl; }
    int start = 0; // Starting Position
    bool flag = false, restart_available = false;
    int restart_pos = 0;

    // (2) Iteration
    for (auto iter: strs) {
        //cout << "Parse target: " << iter << endl;
        start = restart_pos = 0;
        flag = restart_available = false;
        
        for (int i = 0; i < iter.length(); i++) {
            //cout << "Iteration (" << iter << ")" << " - " << iter.at(i) << ", " << i << endl;
            //cout << "Current accepted" << endl;
            for (auto reg: curr_accepted) {
                //cout << "For Loop: " <<  reg->get_ID() << ", start = " << start << ", i = " << i << endl;
                if (match_substr(reg, iter, start, i) == true) {
                    temp_accepted.push_back(reg);
                    //cout << "Temp accepted: " << reg->get_ID() << endl;
                }
            }
            
            // No match result -> retry with new position
            if (temp_accepted.size() == 0) {
                //cout << "Flag: " << flag << ", restart_pos = " << restart_pos << endl;
                if (i < iter.length() - 1) {
                    flag = true;
                    //cout << "No match: " << iter.substr(start, i - start + 1) << endl;
                    
                    temp_accepted.clear();
                    curr_accepted.clear();
                    curr_accepted = regex;
                    continue;
                }
                else if (i == iter.length() - 1) {
                    //cout << "Restarts? (" << iter << ',' << iter.length() << ")" << " i = " << i << ", temp_accepted = " << temp_accepted.size() << endl;
                    i = restart_pos;
                    restart_pos = 0;
                    // No match result -> Print "ERROR" and Exit
                    if (flag == true && restart_available == false) { print_match(); cout << "ERROR" << endl; exit(1); }
                    //cout << "Restarts!" << " i = " << i << ", temp_accepted = " << temp_accepted.size() << endl;
                    real_accepted.push_back(curr_accepted.front());
                    match_str.push_back(iter.substr(start, i - start));
                    //cout << "Add cut: " << temp_accepted.front() << ", " << iter.substr(start, i - start) << endl;
                    start = i--; // New starting position
                    flag = true;
                    restart_available = false;
                    temp_accepted.clear(); // Throw out previous results
                    curr_accepted = regex; // Reset "curr_accepted"
                    continue;
                }
                else if (restart_pos == 0) { flag = true; restart_available = false; }
            } else { flag = false; restart_available = true; restart_pos = i + 1; }

            curr_accepted.clear(); // Throw out previous results
            //curr_accepted = temp_accepted; // Copy the iteration set
            curr_accepted = regex;

            // Final iteration
            if (i == iter.length() - 1) {
                real_accepted.push_back(temp_accepted.front());
                match_str.push_back(iter.substr(start, i - start + 1));
                //cout << "Add: " << temp_accepted.front() << ", " << iter.substr(start, i - start + 1) << endl;
            }
            temp_accepted.clear(); // Throw out copied results
        }
    }
    print_match();
}

bool Parser::match_substr(REG* reg, std::string str, int start, int end) {
    if (end - start < 0) { cerr << "Fatal Error (str: " << str << ", start = " << start << ", end = " << end << ")" << endl; exit(1); }
    bool flag = false;
    string tmp = str.substr(start, end - start + 1);
    // Debug
    //cout << reg->get_ID() << " Check for: " << str.substr(start, end - start + 1) << endl;
    
    // Find path
    RegNode* iter = reg->start;
    if (iter_node(reg, iter, tmp, 0) > 0) { return true; }
    return false;
}

/*  Class REG   */

REG::REG() : 
    start(nullptr), accept(nullptr), num(0), ID("") {}

REG::~REG() {
    RegNode *tmp;
    while (nodes.size() > 0) {
        tmp = nodes.back();
        nodes.pop_back();
        delete tmp;
    }
}

RegNode* REG::create_node(char first_label, char second_label) {
    RegNode* new_node = new RegNode();
    new_node->first_neighbor = nullptr;
    new_node->second_neighbor = nullptr;
    new_node->first_label = first_label;
    new_node->second_label = second_label;
    nodes.push_back(new_node);
    return new_node;
}

void REG::add_node(
    RegNode* first_neighbor, RegNode* second_neighbor,
    char first_label, char second_label)    
{
    RegNode* new_node = this->create_node(first_label, second_label);
    new_node->first_neighbor = first_neighbor;
    new_node->second_neighbor = second_neighbor;
}

void REG::set_ID(std::string text) { this->ID = text; }
void REG::set_line(int num) { this->num = num; }
int REG::get_line() { return this->num; }
std::string REG::get_ID() { return this->ID; }
void REG::print_node() {
    cout << "Printing all nodes..." << endl;
    for (auto i: nodes) { 
        cout << "Access " << i << " {(" << i->first_label << ", " << i->first_neighbor << 
                "), (" << i->second_label << ", " << i->second_neighbor << ")}" << endl;
    }
}

int REG::check_epsilon_node(RegNode* node) {
    int res = 0;
    if (this->accept == node) { return 1; }
    else {
        if (node->first_label == '_' && node->first_neighbor) {
            // Proceed to first_neighbor
            res += check_epsilon_node(node->first_neighbor);
            if (res > 0) { return res; }
        }
        if (node->second_label == '_' && node->second_neighbor) {
            res += check_epsilon_node(node->second_neighbor);
            if (res > 0) { return res; }
        }
    }
    return res;
}

/*  General Functions   */
int iter_node(REG* reg, RegNode* iter, string str, int pos) {
    int res = 0;
    if (pos == str.length()) { 
        //cout << "Checking '" << str << "': " << str.at(pos - 1) << endl; 
        if (reg->accept == iter) { return 1; }
        if (reg->check_epsilon_node(iter) > 0) { return 1; }
        return 0;
    }

    if (iter->first_neighbor && iter->first_label == str.at(pos)) {
        // Next position and proceed to first_neighbor
        res += iter_node(reg, iter->first_neighbor, str, pos + 1);
    }
    if (iter->second_neighbor && iter->second_label == str.at(pos)) {
        // Next position and proceed to second_neighbor
        res += iter_node(reg, iter->second_neighbor, str, pos + 1);
    }
    if (iter->first_neighbor && iter->first_label == '_') {
        if (res > 0) { return res; }
        // Maintain current position and proceed to first_neighbor
        res += iter_node(reg, iter->first_neighbor, str, pos);
    }
    if (iter->second_neighbor && iter->second_label == '_') {
        if (res > 0) { return res; }
        // Maintain current position and proceed to second_neighbor
        res += iter_node(reg, iter->second_neighbor, str, pos);
    }
    return res;
}

size_t split_string(std::string &str, std::vector<std::string> &strs, char ch)
{
    int start = 0, end = str.size() - 1;
    for (std::string::iterator c = str.begin(); c != str.end(); c++) {
        if (!isspace(*c)) { break; }
        start++;
    }
    for (std::string::reverse_iterator c = str.rbegin(); c != str.rend(); c++) {
        if (!isspace(*c)) { break; }
        end--;
    }
    std::string txt = str.substr(start, end - start + 1);
    size_t initialPos = 0;
    size_t pos = txt.find( ch, initialPos );
    strs.clear();
    int tmp;

    // Decompose statement
    while( pos != std::string::npos ) {
        std::string tmp = txt.substr( initialPos, pos - initialPos );
        strs.push_back(tmp);
        if (pos < txt.length()) { 
            initialPos = txt.find_first_not_of(ch, pos);
        }
        pos = txt.find(ch, initialPos);
    }
    strs.push_back(txt.substr(initialPos, std::min(pos, txt.length()) - initialPos));
    return strs.size();
}

// This function simply reads and prints all tokens
// I included it as an example. You should compile the provided code
// as it is and then run ./a.out < tests/test0.txt to see what this function does
// This function is not needed for your solution and it is only provided to
// illustrate the basic functionality of getToken() and the Token type.

void Parser::readAndPrintAllInput()
{
    Token t;

    // get a token
    t = lexer.GetToken();

    // while end of input is not reached
    while (t.token_type != END_OF_FILE) 
    {
        t.Print();         	// pringt token
        t = lexer.GetToken();	// and get another one
    }
        
    // note that you should use END_OF_FILE and not EOF
}

int main()
{
    // note: the parser class has a lexer object instantiated in it (see file
    // parser.h). You should not be declaring a separate lexer object. 
    // You can access the lexer object in the parser functions as shown in 
    // the example  method Parser::readAndPrintAllInput()
    // If you declare another lexer object, lexical analysis will 
    // not work correctly
    Parser parser;

    
    parser.parse_input();
    //parser.reset();
    //parser.readAndPrintAllInput();
	return EXIT_SUCCESS;
}
