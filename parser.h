/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */

// Name: Mark Ahn
// ID: 1226405754
// Course: CSE 340
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"

// Forward declaration
typedef struct _node RegNode;
class Depth;
class Parser;
class REG;

int iter_node(REG* reg, RegNode* iter, std::string str, int pos);
size_t split_string(std::string &str, std::vector<std::string> &strs, char ch);

// Class to track depth and direction of parsing.
class Depth {
    public:
        Depth();
        std::string ID;
        int depth;
        int direc;
        bool inward_consecutive;
        bool outward_consecutive;
        bool dual;

        // Method to handle inward, outward movement
        void inward(TokenType next);
        void outward(TokenType next);

        // Reset method and error handling method
        void reset();
        void make_error(std::string ID);
};

// Main parser class responsible for parsing input and generating regex structures
class Parser {
    public:
        void parse_input();
        void readAndPrintAllInput();

    private:
        // Lexer instance for token generation
        LexicalAnalyzer lexer;
        void syntax_error();

        // Method to expect a specific token type
        Token expect(TokenType expected_type);

        // Parsing methods for different parts of the grammar
        int parse_tokens_section();
        int parse_token_list();
        int parse_token();
        REG* parse_expr();

        /* User variables */
        bool semantic;
        Depth dep;
        REG* curr_reg;
        
        std::vector<REG*> regex;
        std::vector<std::string> texts;

        // Attributes for handling matching and accepted regex
        std::vector<REG*> temp_accepted;
        std::vector<REG*> curr_accepted;
        std::vector<REG*> real_accepted;
        std::vector<std::string> match_str;
        std::vector<std::string> semantic_text;

        std::string tokenID;

        /* User functions */
        void process_input();
        void check_regex(REG* reg);
        bool check_epsilon_reg(REG* reg);
        void check_epsilon();
        bool check_text(Token& t);
        void print_match();
        void match(std::string raw_str);
        bool match_substr(REG* reg, std::string str, int start, int end);
};

// Node structure 
// This structure stands for 'the circle' in the diagram in Class-note.
typedef struct _node {
    struct _node *first_neighbor;  
    struct _node *second_neighbor;
    char first_label;         
    char second_label;        
} RegNode;

// REG structure
// This structure stands for "Regular Expression Graph".
class REG {
    private:
        std::string ID;
        int num;
    public:
        REG();
        ~REG();
        RegNode* create_node(char first_label, char second_label);
        void add_node(  RegNode* first_neighbor,
                        RegNode* second_neighbor,
                        char first_label, 
                        char second_label );
        void print_node();
        std::vector<RegNode*> nodes;
        RegNode* start;
        RegNode* accept;
        void set_line(int num);
        void set_ID(std::string text);
        int get_line();
        std::string get_ID();
        int check_epsilon_node(RegNode* node);
};

#endif

