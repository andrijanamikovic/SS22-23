#ifndef _TABLES_H
#define _TABLES_H

#include <string>
#include <vector>
using namespace std;

class SymbolTableNode {
  public:
    int value;
    int section_id;
    string section_name;
    string name;
    string type;
    bool local;
    bool defined;
    bool extern_sym;
    int offset;
    int symbol_id;
    SymbolTableNode(int symbol_id, int sec_id, bool is_local, bool is_defined, bool is_extern);
    SymbolTableNode();
};

class RelocationTableNode {
  public:
    int section_id;
    string type;
    string name;
    int addend;
    int symbol_id;
    int value;
    int offset;
    RelocationTableNode(int symbol_id, int section_id, string name);
    RelocationTableNode();

};

class SectionTableNode {
  public:
    string name;
    int size;
    int address; //start address?
    int section_id;
    vector<char> data;
    SectionTableNode(string name, int address, int size, int section_id);
    SectionTableNode();
};


#endif