#ifndef LINKER_H
#define LINKER_H


#include <list>
#include <string>
#include "./tables.h"
#include <unordered_map>
#include <fstream>
#include <regex>


using namespace std;

class Linker {
  
  public:
      void link(bool is_hax, bool relocatable_output, list<string> input_files, list<string> places, string output_file);
      Linker();
      static long memory_mapped_registers;

  private:
    regex place_reg = regex("-place=([a-zA-Z][_A-Za-z0-9]*)@0x([0-9A-Fa-f]+)"); 
    bool is_hax;
    bool relocatable_output;
    int _section_id;
    int _symbol_id;
    long startAddress;
    long endAddress;
    list<string> input_files;
    list<string> places;
    map<long, string> sorted_sections;
    string output_file;
    unordered_map<string, long> startAddr;
    int load_data_for_linker(string file);
    void process_place();
    int map_section_table();
    int map_symbol_table();
    int map_relocation_table();
    int move_sections();
    int resolve_relocations();
    void make_hex_file();
    void make_relocatable_file();
    int check_overlapping();

    void printSymbolTableLinker();
    void printRelocationTableLinker();
    void printSectionTableLinker();
    void printSectionTableInFile();

    ifstream input_data;
    ofstream output_linker;
    ofstream linker_help_file;
    ofstream linker_combined_file;

    //map of tables data from every file
    unordered_map<string, unordered_map<string,SectionTableNode>> sections;
    unordered_map< string, unordered_map<string,SymbolTableNode>> symbols;
    unordered_map<string, unordered_map<string,RelocationTableNode>> relocations;


    unordered_map<string,SectionTableNode> output_sections;
    unordered_map<string,SymbolTableNode> output_symbols;
    unordered_map<string,RelocationTableNode> output_relocations;

};


#endif