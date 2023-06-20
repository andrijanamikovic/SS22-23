#include "../../inc/linker.h"
#include <iostream>
#include <iomanip>
using namespace std;

void Linker::link(bool is_hax, bool relocatable_output, list<string> input_files, list<string> places, string output_file)
{
  this->is_hax = is_hax;
  this->relocatable_output = relocatable_output;
  this->places = places;
  this->input_files = input_files;
  this->output_file = output_file;
  this->linker_combined_file.open("linker.txt", ios::out);
  // Za svaki ulazni da ucitam binarni fajl i to da pakujem negde?

  for (string file : input_files)
  {
    if (load_data_for_linker(file) < 0)
      return;
  }
  // this->printSectionTableInFile();
  int ret = 0;
  ret = this->map_section_table();
  if (ret < 0)
  {
    cout << "Error while mapping sections" << endl;
    return;
  }
  ret = this->map_symbol_table();
  if (ret < 0)
  {
    cout << "Error while mapping symbols" << endl;
    return;
  }

  ret = this->map_relocation_table();
  if (ret < 0)
  {
    cout << "Error while rellocations" << endl;
    return;
  }
  this->linker_combined_file << endl
                             << endl
                             << endl;
  this->printSectionTableInFile();
  this->printSectionTableLinker();
  this->printSymbolTableLinker();
  this->printRelocationTableLinker();
  this->linker_combined_file.close();
  if (this->is_hax)
    this->make_hex_file();
  // else if (this->relocatable_output)
  //   this->relocate();
}

int Linker::load_data_for_linker(string file)
{
  int ret = 0;
  string in = "./" + file;
  input_data.open(in, ios::binary);
  if (!this->input_data.is_open())
  {
    cout << "Error while opening binary input: " + in << endl;
  }

  this->linker_help_file.open(file + "_linker.txt", ios::out);

  unsigned size;
  input_data.read((char *)(&size), sizeof(unsigned));

  unsigned sections_size = size;
  this->linker_help_file << endl
                         << "Velicina tabele sekcija: " << sections_size << endl;
  unordered_map<string, SectionTableNode> file_sections;

  for (int i = 0; i < sections_size; i++)
  {
    SectionTableNode *current_section = new SectionTableNode();
    int section_id;
    unsigned current_size;
    input_data.read((char *)(&current_size), sizeof(unsigned)); // section size
    this->linker_help_file << " Size: " << current_size;
    current_section->size = current_size;
    unsigned len;
    input_data.read((char *)(&len), sizeof(unsigned)); // section name len
    this->linker_help_file << " Len: " << len;
    input_data.read((char *)(&section_id), sizeof(int)); // section id
    this->linker_help_file << " Section_id: " << section_id;
    current_section->section_id = section_id;
    long address;
    input_data.read((char *)(&address), sizeof(long)); // section address
    this->linker_help_file << " Address: " << address;
    current_section->address = address;
    string name;
    name.resize(len);
    input_data.read((char *)name.c_str(), len);
    this->linker_help_file << " Name: " << (string)name << endl;
    current_section->name = name;
    char data;
    for (int i = 0; i < current_size; i++)
    {
      // cout << "Data? " << endl;
      input_data.read((char *)&data, sizeof(char)); // section data
      current_section->data.push_back(data);
      // cout << data;
    }

    file_sections.insert({current_section->name, *current_section});
  }

  this->sections.insert({file, file_sections});
  input_data.read((char *)&size, sizeof(size));

  int symbol_table_size = size;
  this->linker_help_file << endl
                         << "Velicina tabele simbola: " << symbol_table_size << endl;

  unordered_map<string, SymbolTableNode> file_symbols;

  for (int i = 0; i < symbol_table_size; i++)
  {
    SymbolTableNode *current_symbol = new SymbolTableNode();
    int symbol_id;
    input_data.read((char *)&symbol_id, sizeof(symbol_id)); // symbol id
    this->linker_help_file << " Symbol_id: " << symbol_id;
    current_symbol->symbol_id = symbol_id;
    int section_id;
    input_data.read((char *)&section_id, sizeof(section_id)); // section id
    this->linker_help_file << " Section_id: " << section_id;
    current_symbol->section_id = section_id;
    char data;
    input_data.read((char *)&data, sizeof(data)); // flags
    current_symbol->extern_sym = ((data >> 0) & 1);
    current_symbol->local = ((data >> 1) & 1);
    current_symbol->defined = ((data >> 2) & 1);
    this->linker_help_file << " Defined: " << current_symbol->defined << " local: " << current_symbol->local << " extern: " << current_symbol->extern_sym << endl; // kako da ih pokupim???
    unsigned len;
    input_data.read((char *)&len, sizeof(unsigned)); // symbol name len
    this->linker_help_file << " Len: " << len;
    string name;
    name.resize(len);
    input_data.read((char *)name.c_str(), len);
    this->linker_help_file << " Name: " << (string)name << endl;
    current_symbol->name = name;
    int value;
    input_data.read((char *)&value, sizeof(long)); // value
    this->linker_help_file << " Value: " << value;
    current_symbol->value = value;
    input_data.read((char *)&len, sizeof(unsigned)); // section name len
    this->linker_help_file << " Len: " << len;
    name.resize(len);
    input_data.read((char *)name.c_str(), len);
    this->linker_help_file << " Section name: " << (string)name << endl;
    current_symbol->section_name = name;
    input_data.read((char *)&len, sizeof(unsigned)); // symbol name len
    this->linker_help_file << " Len: " << len;
    name.resize(len);
    input_data.read((char *)name.c_str(), len);
    this->linker_help_file << " Tip: " << (string)name << endl;
    current_symbol->type = name;
    file_symbols.insert({current_symbol->name, *current_symbol});
  }

  this->symbols.insert({file, file_symbols});

  input_data.read((char *)&size, sizeof(size));

  int relocation_table_size = size;
  this->linker_help_file << endl
                         << "Velicina tabele relokacije: " << relocation_table_size << endl;

  unordered_map<string, RelocationTableNode> file_relocations;

  for (int i = 0; i < relocation_table_size; i++)
  {
    RelocationTableNode *current_relocation = new RelocationTableNode();
    unsigned len;
    input_data.read((char *)&len, sizeof(unsigned)); // symbol name len
    this->linker_help_file << " Len: " << len;
    string name;
    name.resize(len);
    input_data.read((char *)name.c_str(), len);
    this->linker_help_file << " Name: " << (string)name << endl;
    current_relocation->name = name;
    input_data.read((char *)&len, sizeof(unsigned)); // symbol name len
    this->linker_help_file << " Len section_name: " << len;
    name.resize(len);
    input_data.read((char *)name.c_str(), len);
    this->linker_help_file << " Section_name: " << (string)name << endl;
    current_relocation->section_name = name;
    int relocation_id;
    input_data.read((char *)&relocation_id, sizeof(relocation_id)); // section id
    this->linker_help_file << " Relocation_id: " << relocation_id;
    current_relocation->relocation_id = relocation_id;
    long addend;
    input_data.read((char *)&addend, sizeof(addend)); // addend
    this->linker_help_file << " Addend: " << addend;
    current_relocation->addend = addend;
    long value;
    input_data.read((char *)&value, sizeof(value)); // value
    this->linker_help_file << " Value: " << value;
    current_relocation->value = value;
    input_data.read((char *)&len, sizeof(unsigned)); // symbol name len
    this->linker_help_file << " Len: " << len;
    name.resize(len);
    input_data.read((char *)name.c_str(), len);
    this->linker_help_file << " Tip: " << (string)name << endl;
    current_relocation->type = name;
    file_relocations.insert({current_relocation->name, *current_relocation});
  }

  this->relocations.insert({file, file_relocations});
  this->linker_help_file.close();
  this->input_data.close();
  return ret;
}

Linker::Linker()
{
  this->_section_id = 0;
  this->_symbol_id = 0;
}

int Linker::map_section_table()
{
  int ret = 0;
  int next_address = 0;
  for (string file : input_files)
  {
    unordered_map<string, SectionTableNode> &current_section_table = sections.at(file);
    unordered_map<string, RelocationTableNode> &current_relocation_table = relocations.at(file);
    unordered_map<string, SymbolTableNode> &current_symbols_table = symbols.at(file);
    for (auto it = current_section_table.begin(); it != current_section_table.end(); ++it)
    {
      if (it->first == "UND")
        continue;
      SectionTableNode &current = it->second;
      if (output_sections.find(it->first) != output_sections.end())
      {
        // I already have section with the same name
        SectionTableNode &old = output_sections.at(it->first);
        current.address += old.address + old.size;
        next_address += old.size;
        for (auto it1 = current_symbols_table.begin(); it1 != current_symbols_table.end(); ++it1)
        {
          SymbolTableNode &symbol = it1->second;
          if (symbol.section_id == current.section_id)
          {
            symbol.section_id = old.section_id;
          }
        }
        for (auto it1 = current_relocation_table.begin(); it1 != current_relocation_table.end(); ++it1)
        {
          RelocationTableNode &reloc = it1->second;
          if (reloc.section_name == current.name)
          {
            reloc.offset += current.address;
          }
        }
        current.section_id = old.section_id;
        for (char c : current.data)
        {
          old.data.push_back(c);
        }
        old.size += current.size;
        output_sections.erase(it->first);
        output_sections.insert({it->first, old});
      }
      else
      {
        current.address = next_address;
        for (auto it1 = current_symbols_table.begin(); it1 != current_symbols_table.end(); ++it1)
        {
          SymbolTableNode &symbol = it1->second;
          if (symbol.section_id == current.section_id)
          {
            symbol.section_id = _section_id + 1;
          }
        }
        // for (auto it1 = current_relocation_table.begin(); it1 != current_relocation_table.end(); ++it1)
        // {
        //   RelocationTableNode &reloc = it1->second;
        //   if (reloc.section_id == current.section_id)
        //   {
        //     reloc.section_id = _section_id + 1;
        //   }
        // }
        current.section_id = ++_section_id;
        output_sections.insert({it->first, current});
      }
    }
  }
  int old_address = -1;
  int old_size = 0;
  for (auto it = output_sections.begin(); it != output_sections.end(); ++it)
  {
    if (old_address == -1)
    {
      old_address = it->second.address;
      old_size = it->second.size;
    }
    else
    {
      SectionTableNode &current = it->second;
      current.address += old_address + old_size;
      old_address = current.address;
      old_size = current.size;
    }
  }
  return ret;
}
int Linker::map_symbol_table()
{
  int ret = 0;
  int next_address = 0;
  unordered_map<string, SymbolTableNode> extern_symbols;
  // First I add sections
  for (auto it = output_sections.begin(); it != output_sections.end(); ++it)
  {
    SectionTableNode &current = output_sections.at(it->first);
    SymbolTableNode *symbol = new SymbolTableNode(++_symbol_id, current.section_id, true, true, false);
    symbol->value = current.address;
    symbol->type = "SCTN";
    output_symbols.insert({current.name, *symbol});
  }
  for (string file : input_files)
  {
    unordered_map<string, SymbolTableNode> current_symbol_table = symbols.at(file);
    unordered_map<string, SectionTableNode> current_section_table = sections.at(file);
    for (auto it = current_symbol_table.begin(); it != current_symbol_table.end(); ++it)
    {
      SymbolTableNode &symbil_in_file = it->second;
      if (it->second.extern_sym)
      {
        if (output_symbols.find(it->first) != output_symbols.end())
          continue;
        if (extern_symbols.find(it->first) == extern_symbols.end() && output_symbols.find(it->first) == output_symbols.end())
        {
          extern_symbols.insert({it->first, it->second});
        }
      }
      else if (!it->second.local)
      {
        if (output_symbols.find(it->first) != output_symbols.end())
        {
          SymbolTableNode &local_symbol = output_symbols.at(it->first);
          if (local_symbol.local)
            continue;
          cout << "Global symbol already defined: " << it->first << "file: " << file << endl;
          return -1;
        }
        else
        {
          SectionTableNode &current_section = current_section_table.at(it->second.section_name); // ili output_sections to je bilo pre?
          int offset = it->second.value + current_section.address;
          SymbolTableNode *symbol = new SymbolTableNode(++_symbol_id, it->second.section_id, it->second.local, it->second.defined, it->second.extern_sym);
          symbol->value = offset;
          it->second.value = offset;
          symbol->type = it->second.type;
          output_symbols.insert({it->first, *symbol});
        }
      }
      else if (it->second.local)
      {
        unordered_map<string, SectionTableNode> &file_section_table = sections.at(file);
        SymbolTableNode *symbol = new SymbolTableNode(++_symbol_id, it->second.section_id, it->second.local, it->second.defined, it->second.extern_sym);
        if (it->second.section_name != "")
        {
          SectionTableNode &current_section = file_section_table.at(it->second.section_name);
          int offset = it->second.value + current_section.address;
          it->second.value = offset;
          symbol->value = offset;
        }
        symbol->type = it->second.type;
        output_symbols.insert({it->first, *symbol});
      }
    }
  }
  for (auto it = extern_symbols.begin(); it != extern_symbols.end(); ++it)
  {
    if (output_symbols.find(it->first) != output_symbols.end())
    {
      continue;
    }
    if (it->second.section_id == 0)
    {
      cout << "Undefined symbol: " << it->first << endl;
    }
  }
  return ret;
}
int Linker::map_relocation_table()
{
  int ret = 0;
  int next_address = 0;
  for (string file : input_files)
  {
    unordered_map<string, SectionTableNode> &current_section_table = sections.at(file);
    unordered_map<string, RelocationTableNode> &current_relocation_table = relocations.at(file);
    unordered_map<string, SymbolTableNode> &current_symbols_table = symbols.at(file);
    for (auto it = current_relocation_table.begin(); it != current_relocation_table.end(); ++it)
    {
      if (it->second.section_name == "UND")
        continue;
      SymbolTableNode current_sym = current_symbols_table.at(it->first);
      RelocationTableNode *new_relloc = new RelocationTableNode();
      SymbolTableNode global_sym;
      if (output_symbols.find(it->first) == output_symbols.end())
      {
        global_sym = output_symbols.at(it->second.section_name);
      }
      else
      {
        global_sym = output_symbols.at(it->first);
      }
      new_relloc->relocation_id = global_sym.symbol_id;
      new_relloc->section_name = it->second.section_name;
      new_relloc->name = it->second.name;
      new_relloc->type = it->second.type;
      new_relloc->value = it->second.value;
      new_relloc->filename = file;
      new_relloc->value = output_sections.at(it->second.section_name).address - current_section_table.at(it->second.section_name).address; // ja na kraju to dodam na addend?
      new_relloc->addend = it->second.addend;
      output_relocations.insert({it->first, *new_relloc});
    }
  }
  return ret;
}
void Linker::printSymbolTableLinker()
{

  this->linker_combined_file << "Combined symbol table: " << endl;
  this->linker_combined_file << endl
                             << endl
                             << endl
                             << "Symbol table: " << endl;
  this->linker_combined_file << "Symbol_id Symbol_name  Section_id Section_name defined local extern type value" << endl;
  for (auto it = output_symbols.cbegin(); it != output_symbols.end(); ++it)
  {
    this->linker_combined_file << it->second.symbol_id << " " << it->first << " " << it->second.section_id << "  " << it->second.section_name << "  " << it->second.defined << "" << it->second.local << "" << it->second.extern_sym << " " << it->second.type << " " << it->second.value << endl;
  }
  for (string file : input_files)
  {
    this->linker_combined_file << endl
                               << endl
                               << "In file: " << file << endl;
    unordered_map<string, SymbolTableNode> symbol = symbols.at(file);
    this->linker_combined_file << endl
                               << endl
                               << "Symbol table: " << endl;
    this->linker_combined_file << "Symbol_id Symbol_name  Section_id Section_name defined local extern type value" << endl;
    for (auto it = symbol.cbegin(); it != symbol.end(); ++it)
    {
      this->linker_combined_file << it->second.symbol_id << " " << it->first << " " << it->second.section_id << "  " << it->second.section_name << "  " << it->second.defined << "" << it->second.local << "" << it->second.extern_sym << " " << it->second.type << " " << it->second.value << endl;
    }
  }
}

void Linker::printRelocationTableLinker()
{
  for (string file : input_files)
  {
    unordered_map<string, RelocationTableNode> reloc = relocations.at(file);
    this->linker_combined_file << endl
                               << "Relocation table file: " << file << endl;
    this->linker_combined_file << "Relocation_id Symbol_name Section_name addend type value" << endl;
    for (auto it = reloc.cbegin(); it != reloc.end(); ++it)
    {
      this->linker_combined_file << it->second.relocation_id << " " << it->first << "  " << it->second.section_name << " " << it->second.addend << "  " << it->second.type << " " << it->second.value << endl;
    }
  }
  this->linker_combined_file << endl
                             << "Relocation table combined: " << endl;
  this->linker_combined_file << "Relocation_id Symbol_name Section_name addend type value" << endl;
  for (auto it = output_relocations.cbegin(); it != output_relocations.end(); ++it)
  {
    this->linker_combined_file << it->second.relocation_id << " " << it->first << "  " << it->second.section_name << " " << it->second.addend << "  " << it->second.type << " " << it->second.value << endl;
  }
}

void Linker::printSectionTableInFile()
{
  for (auto it1 = sections.cbegin(); it1 != sections.end(); ++it1)
  {
    this->linker_combined_file << endl
                               << "Section table: in file: " << it1->first << endl;
    this->linker_combined_file << "Section_id Section_name  Section_address Section_size Data_len" << endl;
    for (auto it = it1->second.cbegin(); it != it1->second.end(); ++it)
    {
      this->linker_combined_file << it->second.section_id << " " << it->first << " " << it->second.address << "  " << it->second.size << " " << it->second.data.size() << " ";
      // for (auto c : it->second.data)
      // {
      //   linker_combined_file << (char)c;
      // }
      linker_combined_file << endl;
    }
  }
}

void Linker::printSectionTableLinker()
{
  this->linker_combined_file << endl
                             << "Section table: " << endl;
  this->linker_combined_file << "Section_id Section_name  Section_address Section_size Data_len" << endl;
  for (auto it = output_sections.cbegin(); it != output_sections.end(); ++it)
  {
    this->linker_combined_file << it->second.section_id << " " << it->first << " " << it->second.address << "  " << it->second.size << " " << it->second.data.size() << " ";
    // for (auto c : it->second.data)
    // {
    //   linker_combined_file << (char)c;
    // }
    linker_combined_file << endl;
  }
}

int Linker::resolve_relocations()
{
  for (string file : input_files)
  {
    unordered_map<string, RelocationTableNode> &relocation_table = relocations.at(file);
    unordered_map<string, SymbolTableNode> &symbol_table = symbols.at(file);
    unordered_map<string, SectionTableNode> &section_file = sections.at(file);
    for (auto it = relocation_table.begin(); it != relocation_table.end(); ++it)
    {
      SymbolTableNode &current_symbol = output_symbols.at(it->first);
      SectionTableNode &current_section_table = section_file.at(it->second.section_name);
      if (!current_symbol.local)
      {
      }
      if (&current_section_table == nullptr)
        continue;
      SectionTableNode &current_output = output_sections.at(current_section_table.name);
      if (it->second.type == "R_X86_64_32")
      {
      }
      else if (it->second.type == "R_X86_64_PC32")
      {
      }
    }
  }
  return 0;
}

void Linker::make_hex_file()
{
  ofstream *hex_file = new ofstream(output_file, ios::out | ios::binary);
  ofstream *hex_help = new ofstream("hex_help.txt", ios::out);
  if (!hex_file->is_open())
  {
    cout << "Error while opening hex file" << endl;
    return;
  }
  if (!hex_help->is_open())
  {
    cout << "Error while opening hex file" << endl;
    return;
  }
  // int startAddr = 0x4000000;
  int startAddr = 0;
  int i = 0;
  int flag = false;
  for (auto it = output_sections.begin(); it != output_sections.end(); ++it)
  {
  repeat:
    SectionTableNode current_Section = it->second;
    if (it->second.address == startAddr)
    {
      for (unsigned char c : current_Section.data)
      {
        if (i % 16 == 0)
        {
          *hex_help << std::setfill('0') << std::setw(8) << hex << (int)i << ": ";
        }
        hex_file->write((char *)&c, sizeof(c));
        *hex_help << setfill('0') << std::setw(4) << hex << (int)c << " ";
        if (++i % 16 == 0)
        {
          *hex_help << endl;
        }
        flag = true;
      }
    }
    if (flag)
    {
      flag = false;
      startAddr += it->second.size;
      it = output_sections.begin();
      goto repeat;
    }
  }
  unsigned char c = '\n';
  hex_file->write((char *)&c, sizeof(c));

  hex_file->close();
  hex_help->close();
}