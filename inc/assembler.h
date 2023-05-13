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
    map<string,SectionTableNode> sections;
    map<string,SymbolTableNode> symbols;
    map<string,RelocationTableNode> relocations;
    string current_section;
    int current_section_id;
    ofstream* binary_output;
    ofstream assembler_help_file;
    vector<char> data;

    int process();
    void process_input_file();
    int process_label(string label);
    int process_command(string command);
    int process_extern_dir(smatch match);
    int process_global_dir(smatch match);
    int process_section_dir(smatch match);
    int process_word_dir(smatch match);
    int process_skip_dir(smatch match);
    int process_ascii_dir(smatch match);
    int process_end_dir(smatch match);
    
    //instructions:
    int process_halt_inst(smatch match);
    int process_int_inst(smatch match);
    int process_iret_inst(smatch match);
    int process_call_inst(smatch match);
    int process_ret_inst(smatch match);
    int process_jmp_inst(smatch match);
    //
    int process_jeq_inst(smatch match);
    int process_jne_inst(smatch match);
    int process_jgt_inst(smatch match);
    //
    int process_push_inst(smatch match);
    int process_pop_inst(smatch match);
    int process_xchg_inst(smatch match);
    int process_add_inst(smatch match);
    int process_sub_inst(smatch match);
    int process_mul_inst(smatch match);
    int process_div_inst(smatch match);
    int process_cmp_inst(smatch match);
    int process_not_inst(smatch match);
    int process_and_inst(smatch match);
    int process_or_inst(smatch match);
    int process_xor_inst(smatch match);
    int process_test_inst(smatch match);
    int process_shl_inst(smatch match);
    int process_shr_inst(smatch match);
    //changed it was with reg
    int process_ld_inst(smatch match);
    int process_st_inst(smatch match);
    //added
    int process_csrrd_inst(smatch match);
    int process_csrwr_insst(smatch match);

    int process_operand(string operand,string reg, bool load_store);
    void printSymbolTable();
    void printRelocationTable();
    void printSectionTable();
    void outputTables();
    list<string> split(string s, string delimeter);
};
#endif 