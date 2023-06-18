#ifndef LINKER_H
#define LINKER_H


#include <list>
#include <string>
#include "./tables.h"
#include <unordered_map>
#include <fstream>


using namespace std;

class Linker {
  
  public:
      void link(bool is_hax, list<string> input_files, string output_file);
      Linker();

  private:
    bool is_hax;
    int _section_id;
    int _symbol_id;
    list<string> input_files;
    string output_file;
    int load_data_for_linker(string file);
    int map_section_table();
    int map_symbol_table();
    int resolve_relocations();
    void make_hex_file();

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