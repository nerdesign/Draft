/*
 * Copyright (C) Rida Bazzi, 2020
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */

// Name: Mark Ahn
// ID: 1226405754
// Course: CSE 340
#include <iostream>
#include <cstdlib>
#include "parser.h"

using namespace std;

// this syntax error function needs to be 
// modified to produce the appropriate message
void Parser::syntax_error()
{
    cout << "SNYTAX ERORR\n";
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

// Parses an expression from the input stream and constructs a REG (Regular Expression Graph) object
// based on the syntax of the expression. Handles parentheses for grouping, operators like dot (.)
// for concatenation, OR for alternatives, and STAR (*) for repetition.
// The method utilizes depth tracking to manage parentheses and constructs the graph accordingly.
REG* Parser::parse_expr()
{
    REG* res = nullptr;
    std::vector<REG*> series;
    std::vector<REG*> parallel;
    Token t;
    t = lexer.peek(1);
    TokenType prev = END_OF_FILE, next;

    if (
        t.token_type == COMMA ||
        t.token_type == HASH ||
        t.token_type == INPUT_TEXT
    ) {
        //cout << "SNYTAX ERORR" << endl;
        //cout << "SYNTAX ERORR" << endl;
        cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
        exit(1);
    } 

    while (t.token_type != COMMA && t.token_type != HASH) {

        t = lexer.GetToken();
        if (t.token_type == LPAREN) {
            /* Left Parenthesis */
            next = lexer.peek(1).token_type;
            if (
                next != LPAREN &&
                next != RPAREN &&
                next != UNDERSCORE &&
                next != SYMBOL &&
                next != CHAR
            ) { 
                cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
                exit(1);
                //syntax_error();
            }

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
            next = lexer.peek(1).token_type;
            if (
                prev == OR ||
                prev == DOT ||
                (
                next == UNDERSCORE ||
                next == SYMBOL ||
                next == CHAR 
                )
            ) { 
                cout << "SNYTAX ERORR" << endl;
                //cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
                exit(1);
                //cout << "Prev, Next: " << prev << ", " << next << endl;
                //syntax_error();
            }

            dep.outward(lexer.peek(1).token_type);
            
            if (!res) {
                cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
                exit(1);
            }
            /* Right Parenthesis */
            break;
        }
        else if (t.token_type == DOT) {
            if (
                next == STAR ||
                next == COMMA ||
                next == HASH ||
                next == OR
            ) {
                cout << "SNYTAX ERORR" << endl;
                //cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
                exit(1);    
            }
            /* DOT */ series.push_back(res);
        }
        else if (t.token_type == OR) {
            next = lexer.peek(1).token_type;
            if (
                next == STAR ||
                next == COMMA ||
                next == HASH ||
                next == DOT
            ) {
                cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
                exit(1);    
            }
            /* OR */ parallel.push_back(res);
        }
        else if (t.token_type == STAR) {
            if (series.size() > 0 || parallel.size() > 0) {
                cout << "SNYTAX ERORR" << endl;
                exit(1);
            }
            next = lexer.peek(1).token_type;
            if (
                next != COMMA &&
                //next != DOT &&
                next != RPAREN &&
                next != HASH
            ) { 
                cout << "SNYTAX ERORR" << endl;
                exit(1);
            }
            
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
            next = lexer.peek(1).token_type;
            if (
                next == LPAREN ||
                next == UNDERSCORE ||
                next == CHAR ||
                next == STAR ||
                next == DOT ||
                next == OR
            ) { 
                cout << tokenID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
                exit(1);
                //syntax_error();
            }

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
        }
        prev = t.token_type;
        t = lexer.peek(1);
        if (t.token_type == INPUT_TEXT || t.token_type == END_OF_FILE) {
            cout << "SNYTAX ERORR" << endl;
            exit(1);
        }
    }

    /* Merging all the other REG into one REG. */
    if (series.size() > 0 && parallel.size() > 0) { cout << "Size not matched" << endl; syntax_error(); }
    else if (series.size() > 0) {

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

    return res;
}

// Parsing a single token and its associated regular expression from the input.
// It validates the token identifier, then parses and constructs the REG object for the expression.
// parse_token() also handles syntax validation for the expression.
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

// Parsing a list of tokens and their expressions until a HASH (#) token is encountered.
// It uses the parse_token method to parse each token and expects a comma to separate tokens.
int Parser::parse_token_list()
{
    TokenType next;
    while (parse_token() != 1) { 

        if (dep.depth != 0) { cout << tokenID << "HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl; exit(1); }
        next = lexer.peek(1).token_type;
        if (next == INPUT_TEXT) { cout << "SNYTAX ERORR" << endl; exit(1); }
        expect(COMMA); continue;    

        }

    Token t = expect(HASH);
    return 0;
}

// Initiating the parsing of the tokens section of the input.
// This is the entry point for parsing the definitions of tokens and their associated regular expressions.
int Parser::parse_tokens_section()
{
    parse_token_list();
    
    if (semantic == true) {
        for (auto i: semantic_text) { cout << i << endl; }
        exit(1);
    }
    return 0;
}

// The main entry point for the parser. It starts the parsing process by first parsing
// the tokens section and then proceeding to process the input text against the parsed tokens.
void Parser::parse_input()
{
    semantic = false;
    parse_tokens_section();
    Token t;
    t = lexer.GetToken();
    if (t.token_type != INPUT_TEXT) { cout << "SNYTAX ERORR" << endl; exit(1); }
    check_epsilon();

    while (t.token_type != END_OF_FILE) { 
        if (t.token_type != INPUT_TEXT) { cout << "SNYTAX ERORR" << endl; exit(1); }
        if (lexer.peek(1).token_type != END_OF_FILE) { cout << "SNYTAX ERORR" << endl; exit(1); }
        if (check_text(t) == false) { cout << "SNYTAX ERORR" << endl; exit(1); }
        match(t.lexeme);
        t = lexer.GetToken();
    }
}

// Checks when if the given token text is properly enclosed in the quotes.
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
// Depth class constructor initializes member variables to default values.
Depth::Depth() : 
    depth(0), direc(0), inward_consecutive(false), 
    outward_consecutive(false), dual(false), ID("NULL") {}

// Resets the Depth object's state to its initial values.
void Depth::reset() {
    depth = 0;
    direc = 0;
    inward_consecutive = outward_consecutive = false;
    dual = false;
    ID = "NULL";
}

// Adjusts depth tracking for an inward movement, typically when encountering an opening parenthesis.
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

// Adjusts depth tracking for an outward movement, typically when encountering a closing parenthesis.
void Depth::outward(TokenType next) { 
    depth--;
    if (direc == -1) {
        outward_consecutive = true;
        if (inward_consecutive == true && depth > 0
        ) {
            dual = true;

            make_error(ID);
        }
    }
    else { outward_consecutive = false; }
    direc = -1;
}

// Generates a syntax error for the Depth object, typically it is called when an inconsistency is found.
void Depth::make_error(std::string ID) {
    cout << ID << " HAS A SYNTAX ERROR IN ITS EXPRESSION" << endl;
    exit(1);
}

/*  Class Parser    */
// Checking for the presence of epsilon(empty string) in the regular expressions parsed.
// Epsilon transitions are not allowed as tokens.
void Parser::check_epsilon() {
    bool sum = false;
    for (auto i: regex) {
        if (check_epsilon_reg(i) == true) {
            if (sum == false) { cout << "EPSILON IS NOOOOOOOT A TOKEN !!! "; }
            sum = true;
            cout << i->get_ID() << " ";
        }
    }
    if (sum == true) { 
        //cout << endl;
        exit(1); }
    return;
}

// Determines if the given REG object can accept the epsilon (empty string).
bool Parser::check_epsilon_reg(REG* reg) {
    int res = reg->check_epsilon_node(reg->start);
    if (res > 0) { return true; }
    return false;
}

// Verifies the uniqueness of the token identifier and adds the REG object to the list of parsed expressions.
void Parser::check_regex(REG* reg) {
    string tmp;
    //bool copy = false;
    for (auto i : regex) {
        //copy = false;
        // (1) Have the same ID
        if (i->get_ID() == reg->get_ID()) {
            tmp = "Line ";
            tmp += std::to_string(reg->get_line());
            tmp += ": ";
            tmp += i->get_ID();
            tmp += " already declared on line ";
            tmp += std::to_string(i->get_line());
            
            semantic = true;

            //for (auto j: semantic_text) {
            //    if (j == tmp) { copy = true; break; }
            //}
            semantic_text.push_back(tmp);
            //cout << "Line " << reg->get_line() << ": " << 
            //i->get_ID() << " already declared on line " << i->get_line() << endl;
            return;
        }
    }
    this->regex.push_back(reg);
}

// Printing the matches found during the input text processing.
void Parser::print_match() {
    for (int i = 0; i < real_accepted.size(); i++) {
        cout << real_accepted.at(i)->get_ID() << ", "; // Print token name
        cout << '"' << match_str.at(i) << '"' << endl;
    }
}

// Matching the input text against the compiled regular expressions and identifies the tokens present.
void Parser::match(std::string raw_str) {

    // (1) At first, all regex are available to match string.
    curr_accepted = regex;
    real_accepted.clear(); // Reset "real_accepted"
    int len = raw_str.length() - 2; // Except for quotation marks
    string str = raw_str.substr(1, len); // Except for quotation marks
    vector<string> strs; // Substrings splited by whitespaces

    split_string(str, strs, ' '); // Spliting by whitespaces

    int start = 0; // Starting Position
    bool flag = false, restart_available = false;
    int restart_pos = 0;
    vector<REG*> restart_temp;

    // (2) Iteration
    for (auto iter: strs) {

        start = restart_pos = 0;
        flag = restart_available = false;
        
        for (int i = 0; i < iter.length(); i++) {

            for (auto reg: curr_accepted) {

                if (match_substr(reg, iter, start, i) == true) {
                    temp_accepted.push_back(reg);

                }
            }
            
            // No match result -> retry with new position
            if (temp_accepted.size() == 0) {

                if (i < iter.length() - 1) {
                    flag = true;

                    temp_accepted.clear();
                    curr_accepted.clear();
                    curr_accepted = regex;
                    continue;
                }
                else if (i == iter.length() - 1) {

                    i = restart_pos;
                    restart_pos = 0;
                    // No match result -> Print "ERROR" and Exit
                    if (flag == true && restart_available == false) { print_match(); cout << "ERROR" << endl; exit(1); }

                    real_accepted.push_back(restart_temp.front());
                    match_str.push_back(iter.substr(start, i - start));
                    start = i--; // New starting position
                    flag = true;
                    restart_available = false;
                    temp_accepted.clear(); // Throw out previous results
                    curr_accepted = regex; // Reset "curr_accepted"
                    continue;
                }
                else if (restart_pos == 0) { flag = true; restart_available = false; }
            } else { flag = false; restart_available = true; restart_pos = i + 1; restart_temp = temp_accepted; }

            curr_accepted.clear(); // Throw out previous results

            curr_accepted = regex;

            // Final iteration
            if (i == iter.length() - 1) {
                real_accepted.push_back(temp_accepted.front());
                match_str.push_back(iter.substr(start, i - start + 1));

            }
            temp_accepted.clear(); // Throw out copied results
        }
    }
    print_match();
}

// Attempting to match a substring of the input text with a given REG object.
bool Parser::match_substr(REG* reg, std::string str, int start, int end) {

    if (end - start < 0) { cout << "SNYTAX ERORR" << endl; exit(1); }
    bool flag = false;
    string tmp = str.substr(start, end - start + 1);

    // Find path
    RegNode* iter = reg->start;
    if (iter_node(reg, iter, tmp, 0) > 0) { return true; }
    return false;
}

/*  Class REG   */
// Constructor for the REG class, initializes the REG object.
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

// Creates a new node in the REG graph with specified labels for transitions.
RegNode* REG::create_node(char first_label, char second_label) {
    RegNode* new_node = new RegNode();
    new_node->first_neighbor = nullptr;
    new_node->second_neighbor = nullptr;
    new_node->first_label = first_label;
    new_node->second_label = second_label;
    nodes.push_back(new_node);
    return new_node;
}

// Adds a new node to the REG graph, connecting it to specified neighbors with labeled transitions.
void REG::add_node(
    RegNode* first_neighbor, RegNode* second_neighbor,
    char first_label, char second_label)    
{
    RegNode* new_node = this->create_node(first_label, second_label);
    new_node->first_neighbor = first_neighbor;
    new_node->second_neighbor = second_neighbor;
}

// Sets the unique identifier for this REG object.
void REG::set_ID(std::string text) { this->ID = text; }

// Sets the line number associated with this REG object. Useful for error reporting.
void REG::set_line(int num) { this->num = num; }

// Retrieves the line number associated with this REG object.
int REG::get_line() { return this->num; }

// Retrieves the unique identifier of this REG object.
std::string REG::get_ID() { return this->ID; }

// Printing all nodes in the REG object for debugging purposes, showing each node's transitions.
void REG::print_node() {
    cout << "Printing all nodes..." << endl;
    for (auto i: nodes) { 
        cout << "Access " << i << " {(" << i->first_label << ", " << i->first_neighbor << 
                "), (" << i->second_label << ", " << i->second_neighbor << ")}" << endl;
    }
}

// Checking when if there exists an epsilon transition (indicated by '_') from the given node to the accepting state.
// check_epsilon_node() also recursively explores all paths starting from the given node to determine if an epsilon path exists.
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
// Traversing the REG graph to check if the given string matches the regular expression represented by the REG.
// iter_node() also uses recursion to explore all possible paths through the graph.
int iter_node(REG* reg, RegNode* iter, string str, int pos) {
    int res = 0;
    if (pos == str.length()) { 

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

// Splits the input string into a vector of substrings based on a delimiter character.
// Leading, and trailing whitespaces characters are ignored.
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
    // parser.reset();
    // parser.readAndPrintAllInput();
	return EXIT_SUCCESS;
}
