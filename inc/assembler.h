#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <list>
#include <string>
#include <fstream>
using namespace std;


#include "regexs.h"
#include "tables.h"
#include <unordered_map>

class Assembler {
  public:
    Assembler();
    void assemble(string input_file, string output_file);
  private:
    string input_file;
    string output_file;
    bool end;
    list<string> lines;
    Regexes rx;
    int location_counter;
    int current_line;
    int _symbol_id;
    int _section_id;
    unordered_map<string,SectionTableNode> sections;
    unordered_map<string,SymbolTableNode> symbols;
    vector<RelocationTableNode> relocations;
    typedef unordered_map<string, LiteralPoolTable> poolData;
    unordered_map<string, poolData> pool;
    
    int pool_distance;
    string current_section;
    int current_section_id;
    ofstream* binary_output;
    ofstream assembler_help_file;
    vector<int> data;

    int process();
    int process_second_pass();
    void process_input_file();
    int process_label(string label);
    int process_command(string command, bool first);
    int process_extern_dir(smatch match);
    int process_global_dir(smatch match);
    int process_section_dir(smatch match);
    int process_word_dir(smatch match);
    int process_skip_dir(smatch match);
    int process_ascii_dir(smatch match);
    int process_end_dir(smatch match);
    int end_dir_second(smatch match);

    int section_dir_second(smatch match);
    int word_dir_second(smatch match);
    int ascii_dir_second(smatch match);
    int skip_dir_second(smatch match);

    
    //instructions:
    int process_halt_inst(smatch match);
    int process_int_inst(smatch match);
    int process_iret_inst(smatch match);
    int process_call_inst(smatch match);
    int process_ret_inst(smatch match);
    int process_jmp_inst(smatch match);
    int process_beq_inst(smatch match);
    int process_bne_inst(smatch match);
    int process_bgt_inst(smatch match);
    int process_push_inst(smatch match);
    int process_pop_inst(smatch match);
    int process_xchg_inst(smatch match);
    int process_add_inst(smatch match);
    int process_sub_inst(smatch match);
    int process_mul_inst(smatch match);
    int process_div_inst(smatch match);
    int process_not_inst(smatch match);
    int process_and_inst(smatch match);
    int process_or_inst(smatch match);
    int process_xor_inst(smatch match);
    int process_shl_inst(smatch match);
    int process_shr_inst(smatch match);
    int process_ld_inst(smatch match);
    int process_st_inst(smatch match);
    int process_csrrd_inst(smatch match);
    int process_csrwr_inst(smatch match);
    //second:
    int halt_inst_second(smatch match);
    int int_inst_second(smatch match);
    int iret_inst_second(smatch match);
    int call_inst_second(smatch match);
    int ret_inst_second(smatch match);
    int jmp_inst_second(smatch match);
    int beq_inst_second(smatch match);
    int bne_inst_second(smatch match);
    int bgt_inst_second(smatch match);
    int push_inst_second(smatch match);
    int pop_inst_second(smatch match);
    int xchg_inst_second(smatch match);
    int add_inst_second(smatch match);
    int sub_inst_second(smatch match);
    int mul_inst_second(smatch match);
    int div_inst_second(smatch match);
    int not_inst_second(smatch match);
    int and_inst_second(smatch match);
    int or_inst_second(smatch match);
    int xor_inst_second(smatch match);
    int shl_inst_second(smatch match);
    int shr_inst_second(smatch match);
    int ld_inst_second(smatch match);
    int st_inst_second(smatch match);
    int csrrd_inst_second(smatch match);
    int csrwr_inst_second(smatch match);

    int process_operand(string operand,int reg, bool load);
    void printSymbolTable();
    void printRelocationTable();
    void printSectionTable();
    void printLiteralPool();
    void outputTables();
    list<string> split(string s, string delimeter);
    bool isLiteral(string literal);
    long getLiteralValue(string literal);
    long getValue(bool *literal, bool *big, string *operand);
    void process_literal_first(string operand, string current_section);
    void process_literal_second(string operand);
    void mem_operand(string operands,string *operand,string *reg, string current_section, bool store);
    void savePoolData(string current_section, SectionTableNode *current_section_node);
    void calculatePoolData(string current_section, SectionTableNode *current_section_node);
    void GetOperandBJmps(string label, string *r1, string *r2, string *operand);
    void convert_to_little_endian(vector<char> *data, int size);
};
#endif 