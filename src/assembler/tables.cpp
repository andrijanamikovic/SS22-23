#include "../../inc/tables.h"

SymbolTableNode::SymbolTableNode(int symbol_id, int sec_id, bool is_local, bool is_defined, bool is_extern){
  this->value = 0;
  this->section_id = sec_id;
  this->local = is_local;
  this->defined = is_defined;
  this->extern_sym = is_extern;
  this->symbol_id = symbol_id;
}
SymbolTableNode::SymbolTableNode(){}


SectionTableNode::SectionTableNode(string name,long address, long size, int section_id){
  this->name = name;
  this->size = size;
  this->section_id = section_id;
  this->address = address;
}
SectionTableNode::SectionTableNode(){}


RelocationTableNode::RelocationTableNode(int symbol_id, int section_id, string name){
  this->symbol_id = symbol_id;
  this->section_id = section_id;
  this->name = name;
}
RelocationTableNode::RelocationTableNode(){}

LiteralPoolTable::LiteralPoolTable(long name, long offset) {
  this->name = name;
  this->offset = offset;
}

LiteralPoolTable::LiteralPoolTable(){}