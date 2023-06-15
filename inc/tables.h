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
    int section_id;
    string type;
    string name;
    long addend;
    int symbol_id;
    long value;
    long offset;
    RelocationTableNode(int symbol_id, int section_id, string name);
    RelocationTableNode();

};

class SectionTableNode {
  public:
    string name;
    long size;
    long address; //start address?
    int section_id;
    vector<int> data;
    vector<int> pool;
    SectionTableNode(string name, long address, long size, int section_id);
    SectionTableNode();
};

class LiteralPoolTable {
  public:
  long name;
  long offset;
  bool defined = false; //da li sam vec definisala pomeraj ili ne
  bool stored = false;
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
  CALL = 0x20,
  JMP = 0x30,
  BEQ = 0x31,
  BNE = 0x32,
  BGT = 0x33,
  LD = 0x80,
  ST = 0x90,
  CSRRD = 0x90,
  CSRWR = 0x94,
  RET,
  IRET,
  PUSH = 0x81,
  POP = 0x93
};


#endif