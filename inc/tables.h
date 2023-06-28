#ifndef _TABLES_H
#define _TABLES_H

#include <string>
#include <vector>
using namespace std;

class SymbolTableNode {
  public:
    long value;
    int section_id;
    string section_name;
    string name;
    string type;
    bool local;
    bool defined;
    bool extern_sym;
    int symbol_id;
    bool used = false; //za bazen literala
    SymbolTableNode(int symbol_id, int sec_id, bool is_local, bool is_defined, bool is_extern);
    SymbolTableNode();
};

class RelocationTableNode {
  public:
    int relocation_id;
    string type;
    string name;
    string section_name;
    string filename;
    long addend;
    long offset;
    bool local = false;
    bool section = false;
    RelocationTableNode(int relocation_id, string name, string section_name);
    RelocationTableNode();

};

class SectionTableNode {
  public:
    string name;
    int size;
    long address; //start address?
    int section_id;
    vector<char> data;
    vector<char> pool;
    SectionTableNode(string name, long address, long size, int section_id);
    SectionTableNode();
};

class LiteralPoolTable {
  public:
  long name;
  long offset;
  bool defined = false; //da li sam vec definisala pomeraj ili ne
  bool stored = false;
  bool symbol = false;
  LiteralPoolTable(long name, long offset);
  LiteralPoolTable();
};

enum INSTRUCTIONS{
  HALT = 0x00,
  INT = 0x10,
  XCHG = 0x40,
  ADD = 0x50,
  SUB = 0x51,
  MUL = 0x52,
  DIV = 0x53,
  NOT = 0x60,
  AND = 0x61,
  OR = 0x62,
  XOR = 0x63,
  SHL = 0x70,
  SHR = 0x71,
  CALL1 = 0x20,
  CALL2 = 0x21,
  JMP1 = 0x30,
  JMP2 = 0x38,
  BEQ1 = 0x31,
  BEQ2 = 0x39,
  BNE1 = 0x32,
  BNE2 = 0x3A,
  BGT1 = 0x33,
  BGT2 = 0x3B,
  LD1 = 0x91,
  LD2 = 0x92,
  ST1 = 0x80,
  ST2 = 0x82,
  ST3 = LD1, 
  CSRRD = 0x90,
  CSRWR = 0x94,
  RET,
  IRET,
  PUSH = 0x81,
  POP = 0x93
};


#endif