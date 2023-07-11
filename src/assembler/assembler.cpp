#include "../../inc/assembler.h"

//------------------------Helpers-------------------------------------------------------

int getRegNum(string r)
{
  int ret = 0;
  if (r == "%pc")
  {
    ret = 15;
  }
  else if (r == "%sp")
  {
    ret = 14;
  }
  else
  {
    r.erase(0, 2); //%r
    ret = stoi(r);
  }
  return ret;
}

int getCssrRegNum(string reg)
{
  int val = 0;
  if (reg == "status")
  {
    val = 0;
  }
  else if (reg == "handler")
  {
    val = 1;
  }
  else if (reg == "cause")
  {
    val = 2;
  }
  else
  {
    val = -1;
  }
  return val;
}

bool Assembler::isLiteral(string literal)
{
  bool ret = false;
  smatch match;
  if (regex_match(literal, match, rx.hex))
  {
    ret = true;
  }
  else if (regex_match(literal, match, rx.integer))
  {
    ret = true;
  }
  return ret;
}

bool isBig(int literalVal)
{
  if (literalVal < -2048 || literalVal >= 2047)
    return true;
  else
  {
    return false;
  }
}

void Assembler::mem_operand(string operands, string *operand, string *reg, string current_section, bool store)
{
  switch (store)
  {
  case true:
    *reg = operands.substr(0, operands.find(","));
    *operand = operands.substr(operands.find(",") + 1, operands.size());
    break;
  case false:
    *operand = operands.substr(0, operands.find(","));
    *reg = operands.substr(operands.find(",") + 1, operands.size());
    break;
  }
  string regVal = *reg;
  string operandVal = *operand;
  int val = getRegNum(regVal);
  smatch match;
  switch (store)
  {
  case true:
    if (regex_search(operandVal, match, rx.register_relative_operand_regex))
    {
      string operand = match.str(0);
      smatch match2;
      operand = operand.substr(operand.find("+"), operand.size());
      operand = operand.substr(1, operand.find("]") - 1);
      process_literal_first(operand, current_section);
    }
    else if (regex_search(operandVal, match, rx.register_absolute_operand_regex))
    {
    }
    else if (regex_search(operandVal, match, rx.reg_dir_regex))
    {
    }
    else if (regex_search(operandVal, match, rx.absolute_operand_regex))
    {
    }
    else if ((regex_search(operandVal, match, rx.memory_direct_operand_regex)))
    {
      string operand = match.str(0);
      process_literal_first(operand, current_section);
    }
    break;
  case false:
    if (regex_search(operandVal, match, rx.register_relative_operand_regex))
    {
      int r = 0;
      string operand = match.str(0);
      smatch match2;
      operand = operand.substr(operand.find("+"), operand.size());
      operand = operand.substr(1, operand.find("]") - 1);
      process_literal_first(operand, current_section);
    }
    else if (regex_search(operandVal, match, rx.register_absolute_operand_regex))
    {
    }
    else if (regex_search(operandVal, match, rx.reg_dir_regex))
    {
    }
    else if (regex_search(operandVal, match, rx.absolute_operand_regex))
    {
      string operand = match.str(0);
      operand = operand.substr(1, operand.size());
      process_literal_first(operand, current_section);
    }
    else if ((regex_search(operandVal, match, rx.memory_direct_operand_regex)))
    {
      string operand = match.str(0);
      process_literal_first(operand, current_section);
      location_counter += 4;
    }
    break;
  }
}

long Assembler::getValue(bool *literal, bool *big, string *operand)
{
  int ret = -1;
  long literalVal = 0;
  smatch match2;
  string operandVal = *operand;
  if (regex_match(operandVal, match2, rx.lit_regex))
  {
    literalVal = getLiteralValue(operandVal);
    *literal = true;
    if (literalVal < -2048 || literalVal >= 2047)
      *big = true;
    else
    {
      *big = false;
    }
  }
  else if (regex_match(operandVal, match2, rx.sym_regex))
  {
    *literal = false;
    *operand = match2.str(0);
  }
  return literalVal;
}

void Assembler::savePoolData(string current_section, SectionTableNode *current_section_node)
{
  if (pool.find(current_section) == pool.end())
  {
    return;
  }
  poolData &data = pool.at(current_section);
  if (data.size() == 0)
    return;
  int literalVal = data.size() * 4;
  current_section_node->data.push_back((char)(0x30));
  current_section_node->data.push_back((char)0xF0);
  current_section_node->data.push_back((char)((0x0 << 4) | ((literalVal >> 8) & 0x0F)));
  current_section_node->data.push_back((char)(literalVal & 0xFF));
  location_counter += 4;
  current_section_node->size += 4;
  for (auto &it : data)
  {
    LiteralPoolTable &second = it.second;
    if (!second.stored)
    {
      //mozda ovo obrnuto?
      current_section_node->data.push_back((char)(second.name >> 24));
      current_section_node->data.push_back((char)(second.name >> 16));
      current_section_node->data.push_back((char)(second.name >> 8));
      current_section_node->data.push_back((char)(second.name));
      if (!second.defined)
      {
        second.offset = location_counter; // - 4; // - 4;
        second.defined = true;
      }
      if (second.symbol)
      {
        if (symbols.find(it.first) == symbols.end())
        {
          cout << "Nema ovaj simbol u tabeli: " << it.first << endl;
          continue;
        }
        SymbolTableNode &current_symbol = symbols.at(it.first);
        if (current_symbol.local)
        {
          RelocationTableNode *relocation_data = new RelocationTableNode(current_symbol.section_id, current_symbol.section_name, current_section);
          relocation_data->type = "R_X86_64_PC32";
          relocation_data->addend = current_symbol.value; // 0; // second.offset; dodovicu je 0
          relocation_data->local = true;
          relocation_data->offset = location_counter;
          if (current_symbol.type == "SCTN")
            relocation_data->section = true;
          relocations.push_back(*relocation_data);
        }
        else
        {
          RelocationTableNode *relocation_data = new RelocationTableNode(current_symbol.symbol_id, current_symbol.name, current_section);
          relocation_data->type = "R_X86_64_PC32";
          relocation_data->addend = 0;
          relocation_data->local = false;
          relocation_data->offset = location_counter;
          if (current_symbol.type == "SCTN")
            relocation_data->section = true;
          relocations.push_back(*relocation_data);
        }
      }
      location_counter += 4;
      current_section_node->size += 4;
    }
  }
}

void Assembler::calculatePoolData(string current_section, SectionTableNode *current_section_node)
{
  if (pool.find(current_section) == pool.end())
  {
    return;
  }
  poolData &data = pool.at(current_section);
  if (data.size() == 0)
    return;
  // jmp
  location_counter += 4;
  current_section_node->size += 4;
  for (auto &it : data)
  {
    LiteralPoolTable &second = it.second;
    if (!second.defined)
    {
      second.offset = location_counter; // - 4;
      second.defined = true;
    }
    location_counter += 4;
    current_section_node->size += 4;
  }
}

Assembler::Assembler()
{
  this->_symbol_id = 0;
  this->_section_id = 0;
  this->end = false;
}

void Assembler::assemble(string input_file, string output_file)
{
  this->input_file = input_file;
  this->output_file = output_file;
  this->assembler_help_file.open(this->output_file + ".txt", ios::out);
  this->binary_output = new ofstream(this->output_file, ios::out | ios::binary);
  this->process_input_file();
  if (!this->binary_output->is_open())
  {
    cout << "Error while opening binary output file" << endl;
  }

  if (!this->assembler_help_file.is_open())
  {
    cout << "Error while opening txt help file" << endl;
  }
  if (this->lines.empty())
  {
    cout << "Error while processing input file" << endl;
    return;
  }
  if (this->process() != 0)
  {
    this->printSymbolTable();
    cout << "Error while processing assembler data" << endl;
    return;
  }
  // Ja treba i tabele da zapisem u binarni fajl sad??
  this->outputTables();
  this->printSectionTable();
  this->printSymbolTable();
  this->printRelocationTable();
  this->printLiteralPool();
  this->binary_output->close();
  this->assembler_help_file.close();
}

void Assembler::process_input_file()
{
  lines.clear();
  string current_line;
  ifstream input("./tests/" + input_file);
  if (!input.is_open())
  {
    return;
  }
  while (getline(input, current_line))
  {
    current_line = regex_replace(current_line, rx.square_bracket_space, " [$1] ");
    current_line = regex_replace(current_line, rx.tab, " ");
    current_line = regex_replace(current_line, rx.disp_space, "$1+$2");
    current_line = regex_replace(current_line, rx.comma_space, ",");
    current_line = regex_replace(current_line, rx.label_space, ":");
    current_line = regex_replace(current_line, rx.comments, "$1");
    current_line = regex_replace(current_line, rx.more_spaces, " ");
    current_line = regex_replace(current_line, rx.boundary_space, "$2");
    if (current_line != "" && current_line != " ")
    {
      lines.push_back(current_line);
    }
  }
  input.close();
}

int Assembler::process()
{
  this->location_counter = 0;
  this->current_line = 0;
  this->current_section = "UND";
  this->current_section_id = 0;

  SectionTableNode *sectionNode = new SectionTableNode(this->current_section, 0, 0, 0);
  sections.insert({"UND", *sectionNode});
  // create_elf_header();
  for (string line : lines)
  {
    if (this->end)
      break;
    this->current_line++;
    smatch match;
    // cout << "Trenutn linija je: " << line << endl;
    if (regex_search(line, match, rx.only_label))
    {
      int ret = this->process_label(match.str(1));
      if (ret < 0)
        return ret;
    }
    else if (regex_search(line, match, rx.label_with_data))
    {
      int ret = this->process_label(match[1]);
      if (ret < 0)
        return ret;
      ret = this->process_command(match[2], true);
      if (ret < 0)
        return ret;
    }
    else
    {
      int ret = this->process_command(line, true);
      if (ret < 0)
        return ret;
    }
  }
  this->end = false;
  this->location_counter = 0;
  this->current_line = 0;
  this->current_section = "UND";
  this->current_section_id = 0;
  return this->process_second_pass();
}

int Assembler::process_second_pass()
{
  cout << "Drugi kurg assemeblera" << endl;
  int ret = 0;
  for (string line : lines)
  {
    if (this->end)
      break;
    this->current_line++;
    smatch match;
    if (!regex_search(line, match, rx.only_label) and !regex_search(line, match, rx.label_with_data))
    {
      int ret = this->process_command(line, false);
      if (ret < 0)
        return ret;
    }
  }
  return ret;
}

int Assembler::process_label(string label)
{
  label = label.substr(0, label.find(" "));
  if (symbols.find(label) == symbols.end())
  {
    // not found
    if (current_section == "UND")
    {
      cout << "Label is not in a section" << endl
           << "Error at line: " << this->current_line << endl;
      return -1;
    }
    SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, this->_section_id, true, true, false);
    symbol->name = label;
    symbol->value = location_counter;
    symbol->section_name = this->current_section;
    symbol->type = "NOTYP";
    symbols.insert({label, *symbol});
  }
  else
  {
    // found
    SymbolTableNode &symbol = symbols.at(label);
    if (symbol.defined)
    {
      cout << "Label already defined" << endl
           << "Error at line: " << this->current_line << endl;
      return -1;
    }
    else
    {
      if (symbol.extern_sym)
      {
        cout << "Error label symbol is defined as an extern symbol" << endl
             << "Error at line: " << this->current_line << endl;
        return -1;
      }
      cout << "Zasto nisi definisao? " << symbol.name << endl;
      symbol.defined = true;
      symbol.value = location_counter;
      symbol.section_id = this->_section_id;
      symbol.section_name = this->current_section;
      symbol.name = label;
      // poolData &current_pool = pool.at(current_section);
      // if (symbol.used)
      // {
      //   if (current_pool.find(label) == current_pool.end())
      //   {
      //     LiteralPoolTable *literal = new LiteralPoolTable(symbol.value, location_counter);
      //     literal->symbol = true;
      //     current_pool.insert({label, *literal});
      //   }
      //   else
      //   {
      //     LiteralPoolTable &literal = current_pool.at(label);
      //     literal.symbol = true;
      //     literal.offset = location_counter;
      //   }
      // }
    }
    cout << "Vec postoji simbol labela: " << endl;
    // for (auto it = relocations.begin(); it != relocations.end(); ++it)
    // {
    //   if (it->name == label && it->local == false)
    //   {
    //     cout << "Jesi usao ovde za nesto da prepravis leba ti" << endl
    //          << endl;
    //     it->local = true;
    //     it->name = it->section_name;
    //     // it->addend = location_counter;
    //     it->offset = location_counter;
    //     break;
    //   }
    // }
  }
  return 0;
}

int Assembler::process_command(string command, bool first)
{
  cout << "Zove proces command: " << command << "First? " << first << endl;
  smatch match;
  int ret = 0;
  if (regex_search(command, match, rx.extern_dir))
  {
    if (first)
      ret = process_extern_dir(match);
  }
  else if (regex_search(command, match, rx.global_dir))
  {
    if (first)
      ret = process_global_dir(match);
  }
  else if (regex_search(command, match, rx.section_dir))
  {
    if (first)
      ret = process_section_dir(match);
    else
      ret = section_dir_second(match);
  }
  else if (regex_search(command, match, rx.word_dir))
  {
    if (first)
      ret = process_word_dir(match);
    else
      ret = word_dir_second(match);
  }
  else if (regex_search(command, match, rx.skip_dir))
  {
    if (first)
      ret = process_skip_dir(match);
    else
      ret = skip_dir_second(match);
  }
  else if (regex_search(command, match, rx.ascii_dir))
  {
    if (first)
      ret = process_ascii_dir(match);
    else
      ret = ascii_dir_second(match);
  }
  else if (regex_search(command, match, rx.end_dir))
  {
    if (first)
      ret = process_end_dir(match);
    else
      ret = end_dir_second(match);
  }
  else if (regex_search(command, match, rx.halt_inst))
  {
    if (first)
      ret = process_halt_inst(match);
    else
      ret = halt_inst_second(match);
  }
  else if (regex_search(command, match, rx.int_inst))
  {
    if (first)
      ret = process_int_inst(match);
    else
      ret = int_inst_second(match);
  }
  else if (regex_search(command, match, rx.iret_inst))
  {
    if (first)
      ret = process_iret_inst(match);
    else
      ret = iret_inst_second(match);
  }
  else if (regex_search(command, match, rx.call_inst))
  {
    if (first)
      ret = process_call_inst(match);
    else
    {
      ret = call_inst_second(match);
    }
  }
  else if (regex_search(command, match, rx.ret_inst))
  {
    if (first)
      ret = process_ret_inst(match);
    else
      ret = ret_inst_second(match);
  }
  else if (regex_search(command, match, rx.jmp_inst))
  {
    if (first)
      ret = process_jmp_inst(match);
    else
      ret = jmp_inst_second(match);
  }
  else if (regex_search(command, match, rx.beq_inst))
  {
    if (first)
      ret = process_beq_inst(match);
    else
      ret = beq_inst_second(match);
  }
  else if (regex_search(command, match, rx.bne_inst))
  {
    if (first)
      ret = process_bne_inst(match);
    else
      ret = bne_inst_second(match);
  }
  else if (regex_search(command, match, rx.bgt_inst))
  {
    if (first)
      ret = process_bgt_inst(match);
    else
      ret = bgt_inst_second(match);
  }
  else if (regex_search(command, match, rx.push_inst))
  {
    if (first)
      ret = process_push_inst(match);
    else
      ret = push_inst_second(match);
  }
  else if (regex_search(command, match, rx.pop_inst))
  {
    if (first)
      ret = process_pop_inst(match);
    else
      ret = pop_inst_second(match);
  }
  else if (regex_search(command, match, rx.xchg_inst))
  {
    if (first)
      ret = process_xchg_inst(match);
    else
      ret = xchg_inst_second(match);
  }
  else if (regex_search(command, match, rx.add_inst))
  {
    if (first)
      ret = process_add_inst(match);
    else
      ret = add_inst_second(match);
  }
  else if (regex_search(command, match, rx.sub_inst))
  {
    if (first)
      ret = process_sub_inst(match);
    else
      ret = sub_inst_second(match);
  }
  else if (regex_search(command, match, rx.mul_inst))
  {
    if (first)
      ret = process_mul_inst(match);
    else
      ret = mul_inst_second(match);
  }
  else if (regex_search(command, match, rx.div_inst))
  {
    if (first)
      ret = process_div_inst(match);
    else
      ret = div_inst_second(match);
  }
  else if (regex_search(command, match, rx.not_inst))
  {
    if (first)
      ret = process_not_inst(match);
    else
      ret = not_inst_second(match);
  }
  else if (regex_search(command, match, rx.and_inst))
  {
    if (first)
      ret = process_and_inst(match);
    else
      ret = and_inst_second(match);
  }
  else if (regex_search(command, match, rx.or_inst))
  {
    if (first)
      ret = process_or_inst(match);
    else
      ret = or_inst_second(match);
  }
  else if (regex_search(command, match, rx.xor_inst))
  {
    if (first)
      ret = process_xor_inst(match);
    else
      ret = xor_inst_second(match);
  }
  else if (regex_search(command, match, rx.shl_inst))
  {
    if (first)
      ret = process_shl_inst(match);
    else
      ret = shl_inst_second(match);
  }
  else if (regex_search(command, match, rx.shr_inst))
  {
    if (first)
      ret = process_shr_inst(match);
    else
      ret = shr_inst_second(match);
  }
  else if (regex_search(command, match, rx.ld_inst))
  {
    if (first)
      ret = process_ld_inst(match);
    else
      ret = ld_inst_second(match);
  }
  else if (regex_search(command, match, rx.st_inst))
  {
    if (first)
      ret = process_st_inst(match);
    else
      ret = st_inst_second(match);
  }
  else if (regex_search(command, match, rx.csrrd_inst))
  {
    if (first)
      ret = process_csrrd_inst(match);
    else
      ret = csrrd_inst_second(match);
  }
  else if (regex_search(command, match, rx.csrwr_inst))
  {
    if (first)
      ret = process_csrwr_inst(match);
    else
      ret = csrwr_inst_second(match);
  }
  return ret;
}

void Assembler::printSymbolTable()
{
  this->assembler_help_file << endl
                            << "Symbol table: " << endl;
  this->assembler_help_file << "Symbol_id Symbol_name  Section_id Section_name defined local extern value" << endl;
  for (auto it = symbols.cbegin(); it != symbols.end(); ++it)
  {
    this->assembler_help_file << it->second.symbol_id << " " << it->first << " " << it->second.section_id << "  " << it->second.section_name << "  " << it->second.defined << " " << it->second.local << "  " << it->second.extern_sym << " " << it->second.value << endl;
  }
}

void Assembler::printRelocationTable()
{
  this->assembler_help_file << endl
                            << "Relocation table" << endl;
  this->assembler_help_file << "Relocation_id Symbol_name  type addend offset section_name" << endl;
  for (auto it = relocations.cbegin(); it != relocations.end(); ++it)
  {
    this->assembler_help_file << it->relocation_id << " " << it->name << " " << it->type << "  " << hex << it->addend << " " << hex << it->offset << " " << it->section_name << endl;
  }
}

void Assembler::printSectionTable()
{
  this->assembler_help_file << endl
                            << "Section table: " << endl;
  this->assembler_help_file << "Section_id Section_name  Section_address Section_size Data_len" << endl;
  for (auto it = sections.cbegin(); it != sections.end(); ++it)
  {
    this->assembler_help_file << it->second.section_id << " " << it->first << " " << it->second.address << "  " << it->second.size << " " << it->second.data.size() << " ";
    // for (auto c : it->second.data)
    // {
    //   assembler_help_file << (char)c;
    // }
    assembler_help_file << endl;
  }
}

void Assembler::printLiteralPool()
{
  this->assembler_help_file << endl
                            << "Literal pool: " << endl;
  this->assembler_help_file << "Section_name  " << endl;
  for (auto it = pool.cbegin(); it != pool.end(); ++it)
  {
    this->assembler_help_file << it->first << endl;
    this->assembler_help_file << "key name  offset  used" << endl;
    for (auto c : it->second)
    {
      assembler_help_file << c.first << " " << hex << c.second.name << " " << hex << c.second.offset << " " << c.second.defined << endl;
    }
    assembler_help_file << endl;
  }
}

int Assembler::process_extern_dir(smatch match)
{
  int ret = 0;
  string extern_label = ((string)match[0]);
  list<string> symbol_list = this->split(extern_label.substr(extern_label.find(" ") + 1, extern_label.size()), ",");
  extern_label = extern_label.substr(0, extern_label.find(" "));
  if (pool.find("UND") == pool.end())
  {
    poolData data;
    pool.insert({"UND", data});
  }
  poolData &current_pool = pool.at("UND");
  for (string s : symbol_list)
  {
    if (symbols.find(s) == symbols.end())
    {
      // not found
      SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, this->_section_id, false, false, true);
      symbol->name = s;
      symbol->value = location_counter;
      symbol->type = "NOTYP";
      symbol->section_name = "UND";
      symbols.insert({s, *symbol});
      assembler_help_file << ".extern "
                          << "symbol_name: " << s << " location counter: " << location_counter << " section: " << this->current_section << endl;

      // LiteralPoolTable *literal = new LiteralPoolTable(symbol->value, 0);
      // literal->symbol = true;
      // current_pool.insert({symbol->name, *literal});
    }
    else
    {
      // found
      SymbolTableNode &symbol = symbols.at(s);
      if (symbol.defined || !symbol.extern_sym)
      {
        cout << "Error while importing symbol, already exists" << endl
             << "Error at line: " << this->current_line << endl;
        return -1;
      }
    }
  }
  return ret;
}
int Assembler::process_global_dir(smatch match)
{
  int ret = 0;
  string global_label = ((string)match[0]);
  list<string> symbol_list = this->split(global_label.substr(global_label.find(" ") + 1, global_label.size()), ",");
  global_label = global_label.substr(0, global_label.find(" "));
  if (pool.find("UND") == pool.end())
  {
    poolData data;
    pool.insert({"UND", data});
  }
  poolData &current_pool = pool.at("UND");
  for (string s : symbol_list)
  {
    if (symbols.find(s) == symbols.end())
    {
      // not found

      SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, 0, false, false, false);
      symbol->name = s;
      symbol->type = "NOTYP";
      symbol->section_name = "UND"; // ili UND?
      symbols.insert({s, *symbol});
      assembler_help_file << ".global "
                          << "symbol_name: " << s << " location counter: " << location_counter << " section: " << symbol->section_name << endl;
      // LiteralPoolTable *literal = new LiteralPoolTable(symbol->value, 0);
      // literal->symbol = true;
      // current_pool.insert({symbol->name, *literal});
    }
    else
    {
      // found
      SymbolTableNode &symbol = symbols.at(s);
      if (symbol.defined)
      {
        symbol.local = false;
      }
      else
      {
        cout << "Error at line: " << this->current_line << endl;
        return -1;
      }
    }
  }
  return ret;
}
int Assembler::process_section_dir(smatch match)
{
  int ret = 0;
  string section_label = ((string)match[0]);
  string section_name = section_label.substr(section_label.find(" ") + 1, section_label.size());
  section_label = section_label.substr(0, section_label.find(" "));
  if (sections.find(section_name) != sections.end())
  {
    cout << "Section with the same name already defined" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  else if (symbols.find(section_name) != symbols.end())
  { // da li sekcija i simbol mogu da imaju isto ime
    cout << "Symbol with the same name already defined" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, ++this->_section_id, true, true, false);
  symbol->name = section_name;
  symbol->value = location_counter;
  symbol->type = "SCTN";
  symbols.insert({section_name, *symbol});
  if (this->current_section != "")
  {
    if (sections.find(this->current_section) != sections.end())
    {
      SectionTableNode &current_section_node = sections.at(this->current_section);
      // current_section_node.size = current_section_node.data.size();
      current_section_node.size = location_counter;
      assembler_help_file << "End of section: " << current_section_node.name << " with size: " << current_section_node.size << endl;
      calculatePoolData(this->current_section, &current_section_node);
    }
    else
    {
      SectionTableNode current_section_node = SectionTableNode(this->current_section, this->location_counter, 0, this->current_section_id);
      current_section_node.size = location_counter;
      sections.insert({this->current_section, current_section_node});
      assembler_help_file << "End of section: " << current_section_node.name << " with size: " << current_section_node.size << endl;
      calculatePoolData(this->current_section, &current_section_node);
    }
  }
  SectionTableNode *newSection = new SectionTableNode(section_name, this->location_counter, 0, this->_section_id);
  sections.insert({section_name, *newSection});
  poolData data;
  pool.insert({section_name, data}); // meni je ovde druga mapa prazna, ne znam dal to moze tako
  this->current_section = section_name;
  this->location_counter = 0;
  this->pool_distance = 0;
  this->current_section_id = _section_id;
  return ret;
}

int Assembler::section_dir_second(smatch match)
{
  string section_label = ((string)match[0]);
  string section_name = section_label.substr(section_label.find(" ") + 1, section_label.size());
  section_label = section_label.substr(0, section_label.find(" "));
  SectionTableNode &current_section_node = sections.at(this->current_section);
  if (current_section != "UND")
    savePoolData(this->current_section, &current_section_node);
  this->current_section = section_name;
  this->location_counter = 0;
  this->pool_distance = 0;
  this->current_section_id = _section_id;
  return 0;
}
int Assembler::process_word_dir(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Word directive is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  SectionTableNode &sectionNode = sections.at(this->current_section);
  string word_label = ((string)match[0]);
  list<string> symbol_list = this->split(word_label.substr(word_label.find(" ") + 1, word_label.size()), ",");
  this->location_counter = this->location_counter + symbol_list.size() * 4; // uvec za alociran prostor sa word
  return ret;
}

int Assembler::word_dir_second(smatch match)
{
  SectionTableNode &sectionNode = sections.at(this->current_section);
  string word_label = ((string)match[0]);
  list<string> symbol_list = this->split(word_label.substr(word_label.find(" ") + 1, word_label.size()), ",");
  word_label = word_label.substr(0, word_label.find(" "));
  for (string s : symbol_list)
  {
    if (regex_match(s, rx.sym_regex))
    {
      if (symbols.find(s) == symbols.end())
      {
        cout << "Word used undifiend symbol" << endl
             << "Error at line: " << this->current_line << endl;
        return -1;
      }
      SymbolTableNode &symbol = symbols.at(s);
      if (symbol.defined || symbol.extern_sym)
      {

        if (symbol.local)
        {
          // pogledam vrednost iz tabele i metnem u fajl
          assembler_help_file << "Word with symbol value: " << symbol.value << " location_counter : " << location_counter << endl;
          sectionNode.data.push_back((char)((symbol.value >> 24) & 0xFF));
          sectionNode.data.push_back((char)((symbol.value >> 16) & 0xFF));
          sectionNode.data.push_back((char)((symbol.value >> 8) & 0xFF));
          sectionNode.data.push_back((char)((symbol.value) & 0xFF));

          //
          RelocationTableNode *relocation_data = new RelocationTableNode(symbol.section_id, symbol.section_name, current_section);
          relocation_data->local = true;
          relocation_data->type = "R_X86_64_32";
          relocation_data->addend = symbol.value; // 0; // location_counter; dodovic 0
          relocation_data->offset = location_counter;
          relocations.push_back(*relocation_data);
          // relocation_data->offset = location_counter;
        }
        else
        {

          sectionNode.data.push_back((char)(0x00));
          sectionNode.data.push_back((char)(0x00));
          sectionNode.data.push_back((char)(0x00));
          sectionNode.data.push_back((char)(0x00));


          symbol.value = location_counter; // current_line;
          symbol.section_id = current_section_id;
          symbol.section_name = current_section;
          symbol.name = current_section;
          RelocationTableNode *relocation_data = new RelocationTableNode(symbol.symbol_id, symbol.name, current_section);
          relocation_data->type = "R_X86_64_32";
          relocation_data->addend = 0;
          relocation_data->offset = location_counter;
          relocations.push_back(*relocation_data);
        }
      }
      else if (!symbol.extern_sym)
      {
        cout << "Word symbol is not defined: " << endl
             << "Error at line: " << this->current_line << "za symbol:" << s << endl;
        return -1;
      }
    }
    else
    {
      // ako je broj bacim allocate za toliki prostor samo

      // int dataInt = (getLiteralValue(s) >> 24) & 0xFF000000;
      // sectionNode.data.push_back((int)dataInt);
      // dataInt = getLiteralValue(s) & 0xFFFFFFFF;
      // sectionNode.data.push_back((int)dataInt);
      int dataInt = getLiteralValue(s);
      sectionNode.data.push_back((char)((dataInt >> 24) & 0xFF));
      sectionNode.data.push_back((char)((dataInt >> 16) & 0xFF));
      sectionNode.data.push_back((char)((dataInt >> 8) & 0xFF));
      sectionNode.data.push_back((char)((dataInt) & 0xFF));

      assembler_help_file << "Word directive with number: " << s << "location_counter"
                          << " : " << location_counter << endl;
    }
    sectionNode.size += 4;
    this->location_counter += 4;
  }
  return 0;
}

int Assembler::process_skip_dir(smatch match)
{
  int ret = 0;
  string skip_label = ((string)match[0]);
  string skip_val = skip_label.substr(skip_label.find(" "), skip_label.size());
  skip_label = skip_label.substr(0, skip_label.find(" "));

  if (current_section == "UND")
  {
    cout << "Skip directive is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  SectionTableNode &sectionNode = sections.at(this->current_section);
  location_counter += getLiteralValue(skip_val);
  return ret;
}

int Assembler::skip_dir_second(smatch match)
{
  int ret = 0;
  int val;
  string skip_label = ((string)match[0]);
  string skip_val = skip_label.substr(skip_label.find(" "), skip_label.size());
  skip_label = skip_label.substr(0, skip_label.find(" "));
  SectionTableNode &sectionNode = sections.at(this->current_section);
  val = getLiteralValue(skip_val);
  location_counter += val;
  sectionNode.size += val;
  assembler_help_file << "second pass .skip  with value: " << val << endl;
  for (int i = 0; i < val; i++)
  {
    int temp = 0 & 0xFFFF;
    sectionNode.data.push_back((char)0x0);
    sectionNode.data.push_back((char)0x0);
    sectionNode.data.push_back((char)0x0);
    sectionNode.data.push_back((char)0x0);

  }
  return ret;
}
int Assembler::process_ascii_dir(smatch match)
{
  int ret = 0;
  string ascii = match.str(0);
  if (current_section == "UND")
  {
    cout << "Ascii directive is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  SectionTableNode &sectionNode = sections.at(this->current_section);
  assembler_help_file << "first pass .ascii  with value: " << ascii << endl;
  location_counter += ascii.size();
  return ret;
}

int Assembler::ascii_dir_second(smatch match)
{
  int ret = 0;
  string ascii = match.str(0);
  SectionTableNode &sectionNode = sections.at(this->current_section);
  for (int i = 0; i < ascii.size(); i++)
  {
    char temp = ascii[i];
    sectionNode.data.push_back((char)temp);
    sectionNode.size++;
    location_counter++;
  }
  return ret;
}
int Assembler::process_end_dir(smatch match)
{
  int ret = 0;
  assembler_help_file << ".end " << endl;
  if (sections.find(this->current_section) != sections.end())
  {
    SectionTableNode &current_section_node = sections.at(this->current_section);
    calculatePoolData(this->current_section, &current_section_node);
  }
  this->end = true;
  return ret;
}
int Assembler::end_dir_second(smatch match)
{
  // da proverim dal su svi definisani simboli
  int ret = 0;
  if (current_section != "")
  {
    if (sections.find(this->current_section) == sections.end())
    {
      SectionTableNode current_section_node = SectionTableNode(this->current_section, this->location_counter, 0, this->current_section_id);
      current_section_node.size = location_counter;
      savePoolData(this->current_section, &current_section_node);
      sections.insert({this->current_section, current_section_node});
      assembler_help_file << ".end of section: " << current_section_node.name << " with size: " << current_section_node.size << endl;
    }
    else
    {
      SectionTableNode &current_section_node = sections.at(this->current_section);
      current_section_node.size = location_counter;
      savePoolData(this->current_section, &current_section_node);
      assembler_help_file << ".end of section: " << current_section_node.name << " with size: " << current_section_node.size << endl;
    }
  }
  for (auto it = symbols.cbegin(); it != symbols.end(); ++it)
  {
    SymbolTableNode current_symbol = it->second;
    if (!current_symbol.defined && !current_symbol.extern_sym)
    {
      cout << "Error symbol is not defined" << it->first << endl;
      ret = -1;
    }
  }
  this->location_counter = 0;
  this->pool_distance = 0;
  this->end = true;
  return ret;
}

list<string> Assembler::split(string s, string delimeter)
{
  list<string> ret;
  int pos = 0;
  while ((pos = s.find(delimeter)) != string::npos)
  {
    ret.push_back(s.substr(0, pos));
    s = s.erase(0, pos + delimeter.length());
  }
  ret.push_back(s);
  return ret;
}

int Assembler::process_halt_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Halt instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;

  return ret;
}

int Assembler::halt_inst_second(smatch match)
{
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  if (sections.find(this->current_section) != sections.end())
  {
    SectionTableNode &current_section_node = sections.at(this->current_section);
    assembler_help_file << "Halt in section: " << current_section_node.name << " with size: " << current_section_node.size << endl;
  }
  else
  {
    SectionTableNode *current_section_node = new SectionTableNode(this->current_section, this->location_counter, 0, this->current_section_id);
    sections.insert({this->current_section, *current_section_node});
    assembler_help_file << "Halt in section: " << current_section_node->name << " with size: " << current_section_node->size << endl;
  }
  int temp = HALT;
  sectionNode.data.push_back((char)(temp << 4) | 0x0);
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((char)0);

  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}

int Assembler::process_int_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Int instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}

int Assembler::int_inst_second(smatch match)
{
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  string int_label = ((string)match[0]);

  assembler_help_file << "int " << endl;
  sectionNode.data.push_back((char)(INT));
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_iret_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Iret instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }

  location_counter += 8;
  return ret;
}
int Assembler::iret_inst_second(smatch match)
{
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);

  if (sections.find(this->current_section) != sections.end())
  {
    SectionTableNode &current_section_node = sections.at(this->current_section);
    assembler_help_file << "Iret in section: " << current_section_node.name << " with size: " << current_section_node.size << endl;
  }
  else
  {
    SectionTableNode *current_section_node = new SectionTableNode(this->current_section, this->location_counter, 0, this->current_section_id);
    sections.insert({this->current_section, *current_section_node});
    assembler_help_file << "Iret in section: " << current_section_node->name << " with size: " << current_section_node->size << endl;
  }
  // pop pc;
  sectionNode.data.push_back(POP);
  sectionNode.data.push_back((char)(0xE0));
  sectionNode.data.push_back((char)(0xF0));
  sectionNode.data.push_back((char)(0x04));
  // pop status;
  sectionNode.data.push_back((char)0x97);
  sectionNode.data.push_back((char)(0x0E));
  sectionNode.data.push_back((int)(0x00));
  sectionNode.data.push_back((char)(0x04));

  // mozda treba da se upisuje u cause 0 kad se vrati mozda?
  // zato sto int upsije 4 u cause registar

  sectionNode.size += 8;
  return ret;
}
int Assembler::process_call_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Call instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  string operand = match.str(2);
  process_literal_first(operand, current_section);
  location_counter += 4;
  assembler_help_file << "Call inst first " << operand << endl;
  return ret;
}

void Assembler::process_literal_first(string operand, string current_section)
{
  bool big = true;
  bool literal = false;
  int literalVal = getValue(&literal, &big, &operand);
  poolData &current = pool.at(current_section);
  if (literal)
  {
    LiteralPoolTable *literal_current = new LiteralPoolTable(literalVal, 0);
    current.insert({operand, *literal_current});
  }
  else
  {
    if (symbols.find(operand) != symbols.end())
    {
      SymbolTableNode &symbol = symbols.at(operand);
      if (!symbol.extern_sym or !symbol.defined)
      {
        LiteralPoolTable *literal = new LiteralPoolTable(symbol.value, symbol.value);
        symbol.used = true;
        literal->symbol = true;
        symbol.section_name = current_section;
        current.insert({operand, *literal});
      }
    }
    else
    {
      SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, 0, false, false, false);
      LiteralPoolTable *literal = new LiteralPoolTable(symbol->value, symbol->value);
      symbol->used = true;
      symbol->value = location_counter;
      symbol->section_name = current_section;
      literal->symbol = true;
      current.insert({operand, *literal});
    }
  }
}

int Assembler::call_inst_second(smatch match)
{
  int ret = 0;
  assembler_help_file << "Call inst second " << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  bool big = true;
  string operand = match.str(2);
  bool literal = false;
  long literalVal = getValue(&literal, &big, &operand);
  if (literal)
  {
    poolData &current_pool = pool.at(current_section);
    if (current_pool.find(operand) != current_pool.end())
    {
      LiteralPoolTable &literal_current = current_pool.at(operand);
      int disp = literal_current.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x21));
      sectionNode.data.push_back((char)0xF0);
      sectionNode.data.push_back((char)((0 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  else
  {
    if (symbols.find(operand) == symbols.end())
    {
      // Error?
    }
    else
    {
      SymbolTableNode symbol = symbols.at(operand);
      int val = symbol.value;
      poolData &current_pool = pool.at(current_section);
      LiteralPoolTable &literal = current_pool.at(operand);
      int disp = literal.offset - location_counter;// - 4;
      cout << "Za: " << operand << endl;
      cout << "Ugradio disp: " << hex << disp << " ? " << endl;
      sectionNode.data.push_back((char)(0x21));
      sectionNode.data.push_back((char)0xF0);
      sectionNode.data.push_back((char)((0 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
    // vrednost simbola?
    // ako ne znam?
    // ako znam jer on onda odmah u bazenu literala kad je definisan? nije? samo u tabeli?
    // proverim dal je veca od 12 ako nije onda odmah, a ako jeste onda bazen literala?
  }
  // ret = process_symbol_disp(CALL, 0, match.str(2), sectionNode);

  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_ret_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Ret instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }

  location_counter += 4;
  return ret;
}
int Assembler::ret_inst_second(smatch match)
{
  int ret = 0;
  if (sections.find(this->current_section) != sections.end())
  {
    SectionTableNode &current_section_node = sections.at(this->current_section);
    // current_section_node.size = current_section_node.data.size();
    assembler_help_file << "Ret in section: " << current_section_node.name << " with size: " << current_section_node.size << endl;
  }
  else
  {
    SectionTableNode *current_section_node = new SectionTableNode(this->current_section, this->location_counter, 0, this->current_section_id);
    sections.insert({this->current_section, *current_section_node});
    assembler_help_file << "Ret in section: " << current_section_node->name << " with size: " << current_section_node->size << endl;
  }
  SectionTableNode &sectionNode = sections.at(this->current_section);
  // ret = pop pc;
  sectionNode.data.push_back(POP);
  sectionNode.data.push_back((char)(0xE0));
  sectionNode.data.push_back((char)(0xF0));
  sectionNode.data.push_back((char)(0x04));
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_jmp_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Jmp instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  string operand = match.str(2);
  process_literal_first(operand, current_section);
  location_counter += 4;
  return ret;
}
int Assembler::jmp_inst_second(smatch match)
{
  int ret = 0;
  assembler_help_file << ".jmp with operand: " << match.str(2) << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  bool big = true;
  string operand = match.str(2);
  bool literal = false;
  long literalVal = getValue(&literal, &big, &operand);
  if (literal)
  {
    poolData &current_pool = pool.at(current_section);
    if (current_pool.find(operand) != current_pool.end())
    {
      LiteralPoolTable &literal_current = current_pool.at(operand);
      int disp = literal_current.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x38));
      sectionNode.data.push_back((char)0xF0);
      sectionNode.data.push_back((char)((0 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  else
  {
    if (symbols.find(operand) == symbols.end())
    {
      // Error?
    }
    else
    {
      int val = symbols.at(operand).value;
      poolData &current_pool = pool.at(current_section);
      LiteralPoolTable &literal = current_pool.at(operand);
      int disp = literal.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x38));
      sectionNode.data.push_back((char)0xF0);
      sectionNode.data.push_back((char)((0 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_beq_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Beq instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  string label = match.str(2);
  string r1;
  string r2;
  string operand;
  GetOperandBJmps(label, &r1, &r2, &operand);
  process_literal_first(operand, current_section);
  location_counter += 4;

  return ret;
}

void Assembler::GetOperandBJmps(string label, string *r1, string *r2, string *operand)
{

  string reg = label.substr(label.find(" ") + 1, label.size());
  *r1 = reg.substr(0, reg.find(","));
  *r2 = reg.substr(reg.find(",") + 1, reg.find(","));
  *operand = reg.substr(reg.find(",") + 1, reg.size());
  string help = *operand;
  *operand = help.substr(help.find(",") + 1, help.size());
}

int Assembler::beq_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string r1;
  string r2;
  string operand;
  GetOperandBJmps(int_label, &r1, &r2, &operand);
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "beq: " << val1 << " , " << val2 << " , " << operand << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  bool big = true;
  bool literal = false;
  long literalVal = getValue(&literal, &big, &operand);
  if (literal)
  {
    poolData &current_pool = pool.at(current_section);
    if (current_pool.find(operand) != current_pool.end())
    {
      LiteralPoolTable &literal_current = current_pool.at(operand);
      int disp = literal_current.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x39));
      sectionNode.data.push_back((char)(0xF << 4) | val1);
      sectionNode.data.push_back((char)((val2 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  else
  {
    if (symbols.find(operand) == symbols.end())
    {
      // Error?
    }
    else
    {
      int val = symbols.at(operand).value;
      poolData &current_pool = pool.at(current_section);
      LiteralPoolTable &literal = current_pool.at(operand);
      int disp = literal.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x39));
      sectionNode.data.push_back((char)(0xF << 4) | val1);
      sectionNode.data.push_back((char)((val2 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_bne_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Bne instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  string label = match.str(2);
  string r1;
  string r2;
  string operand;
  GetOperandBJmps(label, &r1, &r2, &operand);
  process_literal_first(operand, current_section);
  location_counter += 4;

  return ret;
}
int Assembler::bne_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string r1;
  string r2;
  string operand;
  GetOperandBJmps(int_label, &r1, &r2, &operand);
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "bne: " << val1 << " , " << val2 << " , " << operand << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  bool big = true;
  bool literal = false;
  long literalVal = getValue(&literal, &big, &operand);
  if (literal)
  {
    poolData &current_pool = pool.at(current_section);
    if (current_pool.find(operand) != current_pool.end())
    {
      LiteralPoolTable &literal_current = current_pool.at(operand);
      int disp = literal_current.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x3A));
      sectionNode.data.push_back((char)(0xF << 4) | val1);
      sectionNode.data.push_back((char)((val2 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  else
  {
    if (symbols.find(operand) == symbols.end())
    {
      // Error?
    }
    else
    {
      int val = symbols.at(operand).value;
      poolData &current_pool = pool.at(current_section);
      LiteralPoolTable &literal = current_pool.at(operand);
      int disp = literal.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x3A));
      sectionNode.data.push_back((char)(0xF << 4) | val1);
      sectionNode.data.push_back((char)((val2 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_bgt_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Bqt instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  string label = match.str(2);
  string r1;
  string r2;
  string operand;
  GetOperandBJmps(label, &r1, &r2, &operand);
  process_literal_first(operand, current_section);
  location_counter += 4;

  return ret;
}
int Assembler::bgt_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string r1;
  string r2;
  string operand;
  GetOperandBJmps(int_label, &r1, &r2, &operand);
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "bqt: " << val1 << " , " << val2 << " , " << operand << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  bool big = true;
  bool literal = false;
  long literalVal = getValue(&literal, &big, &operand);
  if (literal)
  {
    poolData &current_pool = pool.at(current_section);
    if (current_pool.find(operand) != current_pool.end())
    {
      LiteralPoolTable &literal_current = current_pool.at(operand);
      int disp = literal_current.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x3B));
      sectionNode.data.push_back((char)(0xF << 4) | val1);
      sectionNode.data.push_back((char)((val2 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  else
  {
    if (symbols.find(operand) == symbols.end())
    {
      // Error?
    }
    else
    {
      int val = symbols.at(operand).value;
      poolData &current_pool = pool.at(current_section);
      LiteralPoolTable &literal = current_pool.at(operand);
      int disp = literal.offset - location_counter;// - 4;
      sectionNode.data.push_back((char)(0x3B));
      sectionNode.data.push_back((char)(0xF << 4) | val1);
      sectionNode.data.push_back((char)((val2 << 4) | ((disp >> 8) & 0x0F)));
      sectionNode.data.push_back((char)(disp & 0xFF));
    }
  }
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_push_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Push instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::push_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  val = getRegNum(reg);
  assembler_help_file << "push: " << val << " location_counter: " << location_counter << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(PUSH);
  sectionNode.data.push_back((char)(0xE0));
  sectionNode.data.push_back((char)((val << 4) | 0xF));
  sectionNode.data.push_back((char)0xFC);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_pop_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Push instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::pop_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  val = getRegNum(reg);
  assembler_help_file << "pop: " << val << " location_counter: " << location_counter << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(POP);
  sectionNode.data.push_back((char)(0xE0));
  sectionNode.data.push_back((char)((val << 4)));
  sectionNode.data.push_back((char)0x04);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}

int Assembler::process_xchg_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Xchg instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::xchg_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "Xchg val1:  " << val1 << " val2: " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(XCHG);
  sectionNode.data.push_back((char)(val2));
  sectionNode.data.push_back((char)(val1 << 4));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_add_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Add instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::add_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "add: " << val1 << " + " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(ADD);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_sub_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Sub instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 2;
  return ret;
}
int Assembler::sub_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "sub: " << val1 << " - " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(SUB);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_mul_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Mul instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::mul_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "mul: " << val1 << " * " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(MUL);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_div_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Div instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::div_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "div: " << val1 << " / " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(DIV);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_not_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Not instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::not_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  val = getRegNum(reg);
  assembler_help_file << "not: " << val << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(NOT);
  sectionNode.data.push_back((char)((val << 4) | 0xF));
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_and_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "And instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::and_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(" ") - 1);
  string r2 = reg.substr(reg.find(" ") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "and: " << val1 << " & " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(AND);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_or_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Or instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::or_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "or: " << val1 << " | " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(OR);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_xor_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Xor instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::xor_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "xor: " << val1 << " xor " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(XOR);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_shl_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Shl instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::shl_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "shl: " << val1 << " + " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(SHL);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_shr_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Shr instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::shr_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getRegNum(r1);
  val2 = getRegNum(r2);
  assembler_help_file << "shr: " << val1 << " + " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(SHR);
  sectionNode.data.push_back((char)((val2 << 4) | val2));
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_ld_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Ld instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  string operands = match.str(2);
  string reg;
  string operand;
  mem_operand(operands, &operand, &reg, current_section, false);
  location_counter += 4;
  return ret;
}
int Assembler::ld_inst_second(smatch match)
{
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  string operands = match.str(2);
  string operand = operands.substr(0, operands.find(","));
  string reg = operands.substr(operands.find(",") + 1, operands.size());
  int val1 = getRegNum(reg);
  assembler_help_file << "ld sa operandom " << operand << endl;
  ret = process_operand(operand, val1, true);
  location_counter += 4;
  return ret;
}
int Assembler::process_st_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "St instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  string operands = match.str(2);
  string reg;
  string operand;
  mem_operand(operands, &operand, &reg, current_section, true);
  location_counter += 4;
  return ret;
}
int Assembler::st_inst_second(smatch match)
{
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  string operands = match.str(2);
  string reg = operands.substr(0, operands.find(","));
  string operand = operands.substr(operands.find(",") + 1, operands.size());
  assembler_help_file << "str sa operandom: " << operand << " and reg" << reg << endl;
  int val = getRegNum(reg);
  ret = process_operand(operand, val, false);
  location_counter += 4;
  return ret;
}
int Assembler::process_csrrd_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "Csrrd instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}

int Assembler::csrrd_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(1, reg.find(",") - 1);
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  val1 = getCssrRegNum(r1);
  if (val1 < 0)
  {
    cout << "Csrrd invalied control register " << r1 << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  val2 = getRegNum(r2);
  assembler_help_file << "csrrd: csr: " << val1 << " i reg:" << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(CSRRD);
  sectionNode.data.push_back((char)((val2 << 4) | val1));
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}
int Assembler::process_csrwr_inst(smatch match)
{
  int ret = 0;
  if (current_section == "UND")
  {
    cout << "CSRW instruction is not in a section" << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  location_counter += 4;
  return ret;
}
int Assembler::csrwr_inst_second(smatch match)
{
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 2, reg.size());
  val2 = getCssrRegNum(r2);
  if (val2 < 0)
  {
    cout << "Csrrd invalied control register " << r2 << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  val1 = getRegNum(r1);
  assembler_help_file << "csrwr: csr " << val1 << " i reg " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(CSRWR);
  sectionNode.data.push_back((char)((val1 << 4) | val2));
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  return ret;
}

long Assembler::getLiteralValue(string literal)
{
  smatch match;
  int value = 0;
  if (regex_search(literal, match, rx.hex))
  {
    literal = literal.substr(literal.find('x' | 'X') + 1, literal.size());
    value = stol(literal, 0, 16);
  }
  else
  {
    value = stol(literal);
  }
  return value;
}

int Assembler::process_operand(string operand, int reg, bool load_store)
{
  smatch match;
  int ret = 0;
  switch (load_store)
  {
  case false:
    // st
    if (regex_search(operand, match, rx.register_relative_operand_regex))
    {
      int r = 0;
      string operand = match.str(0);
      smatch match2;
      // izvucem iz [%r13 + operand] registar i operand
      //  broj registra odmah kodiram pa operand saljem na onaj process
      if (regex_search(operand, match2, rx.reg_regex))
      {
        r = getRegNum(match2.str(0));
      }
      operand = operand.substr(operand.find("+"), operand.size());
      operand = operand.substr(1, operand.find("]") - 1);
      bool big = false;
      bool literal = false;
      int literalVal = getValue(&literal, &big, &operand);
      SectionTableNode &sectionNode = sections.at(this->current_section);
      if (literal)
      {
        if (!big)
        {
          sectionNode.data.push_back((char)0x80);
          sectionNode.data.push_back((char)((r & 0x0F)));
          sectionNode.data.push_back((char)((reg << 4) | ((literalVal >> 8) & 0x0F)));
          sectionNode.data.push_back((char)(literalVal & 0xFF));
        }
        else
        {
          cout << "Error st + reg ind with disp biger then 12b" << endl;
          return -1;
        }
      }
      else
      {
        int val = symbols.at(operand).value;
        if (isBig(val))
        {
          cout << "Error st + reg ind with disp biger then 12b" << endl;
          return -1;
        }
        else
        {
          sectionNode.data.push_back((char)0x80);
          sectionNode.data.push_back((char)((r & 0x0F)));
          sectionNode.data.push_back((char)((reg << 4) | ((val >> 8) & 0x0F)));
          sectionNode.data.push_back((char)(val & 0xFF));
        }
      }
      sectionNode.size += 4;
      this->assembler_help_file << "Store registar indirect with disp: " << operand << endl;
    }
    else if (regex_search(operand, match, rx.register_absolute_operand_regex))
    {
      int r = 0; // to nemam sad
      string operand = match.str(0);
      operand = operand.substr(1, operand.size() - 1);
      r = getRegNum(operand);
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)0x80);
      sectionNode.data.push_back((char)r);
      sectionNode.data.push_back((char)(reg << 4));
      sectionNode.data.push_back((char)0);

      this->assembler_help_file << "Store registar indirect operand: " << operand << endl;
      sectionNode.size += 4;
    }
    else if (regex_search(operand, match, rx.reg_dir_regex))
    {
      int r = 0;
      r = getRegNum(match.str(0));
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)0x91);
      sectionNode.data.push_back((char)((r << 4) | reg));
      sectionNode.data.push_back((char)0);
      sectionNode.data.push_back((char)0);
      sectionNode.size += 4;
      this->assembler_help_file << "Store reg direct: " << reg << ", " << r << endl;
    }
    else if (regex_search(operand, match, rx.absolute_operand_regex))
    {
      cout << "Store + immediate error" << endl;
      ret = -1;
    }
    else if (regex_search(operand, match, rx.memory_direct_operand_regex))
    {
      smatch match2;
      bool big = false;
      bool literal = false;
      int literalVal = getValue(&literal, &big, &operand);
      SectionTableNode &sectionNode = sections.at(this->current_section);
      if (literal)
      {
        poolData &current_pool = pool.at(current_section);
        if (current_pool.find(operand) != current_pool.end())
        {
          LiteralPoolTable &literal_current = current_pool.at(operand);
          int disp = literal_current.offset - location_counter;// - 4;
          sectionNode.data.push_back((char)(0x82));
          sectionNode.data.push_back((char)0xF0);
          sectionNode.data.push_back((char)((reg << 4) | ((disp >> 8) & 0x0F)));
          sectionNode.data.push_back((char)(disp & 0xFF));
        }
      }
      else
      {
        if (symbols.find(operand) == symbols.end())
        {
          // Error?
        }
        else
        {
          int val = symbols.at(operand).value;
          poolData &current_pool = pool.at(current_section);
          LiteralPoolTable &literal = current_pool.at(operand);
          int disp = literal.offset - location_counter;// - 4;
          sectionNode.data.push_back((char)(0x82));
          sectionNode.data.push_back((char)0xF0);
          sectionNode.data.push_back((char)((reg << 4) | ((disp >> 8) & 0x0F)));
          sectionNode.data.push_back((char)(disp & 0xFF));
        }
      }
      sectionNode.size += 4;
      this->assembler_help_file << "Store memory direct: " << endl;
    }
    else
    {
      cout << "Store with error parsing" << endl;
      ret = -1;
    }
    break;

  case true:
    // ld
    if (regex_search(operand, match, rx.register_relative_operand_regex))
    {
      int r = 0;
      string operand = match.str(0);
      smatch match2;
      // izvucem iz [%r13 + operand] registar i operand
      //  broj registra odmah kodiram pa operand saljem na onaj process
      if (regex_search(operand, match2, rx.reg_regex))
      {
        r = getRegNum(match2.str(0));
      }
      operand = operand.substr(operand.find("+"), operand.size());
      operand = operand.substr(1, operand.find("]") - 1);
      bool big = false;
      bool literal = false;
      int literalVal = getValue(&literal, &big, &operand);
      SectionTableNode &sectionNode = sections.at(this->current_section);
      if (literal)
      {
        if (!big)
        {
          sectionNode.data.push_back((char)0x92);
          sectionNode.data.push_back((char)((reg << 4) | r));
          sectionNode.data.push_back((char)((literalVal >> 8) & 0xFF));
          sectionNode.data.push_back((char)(literalVal & 0xFF));
        }
        else
        {
          cout << "Error ld + reg ind with disp biger then 12b" << endl;
          return -1;
        }
      }
      else
      {
        int val = symbols.at(operand).value;
        if (isBig(val))
        {
          cout << "Error st + reg ind with disp biger then 12b" << endl;
          return -1;
        }
        else
        {
          sectionNode.data.push_back((char)0x92);
          sectionNode.data.push_back((char)((reg << 4) | r));
          sectionNode.data.push_back((char)((val >> 8) & 0xFF));
          sectionNode.data.push_back((char)(val & 0xFF));
        }
      }
      sectionNode.size += 4;
      this->assembler_help_file << "Load registar indirect with disp: " << operand << endl;
    }
    else if (regex_search(operand, match, rx.register_absolute_operand_regex))
    {
      int r = 0; // to nemam sad
      string operand = match.str(0);
      operand = operand.substr(1, operand.size() - 1);
      r = getRegNum(operand);
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)0x92);
      sectionNode.data.push_back((char)((reg << 4) | r));
      sectionNode.data.push_back((char)0);
      sectionNode.data.push_back((char)0);
      this->assembler_help_file << "Load registar indirect : " << operand << " , " << reg << endl;
      sectionNode.size += 4;
    }
    else if (regex_search(operand, match, rx.reg_dir_regex))
    {
      int r = 0;
      r = getRegNum(match.str(0));
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)0x91);
      sectionNode.data.push_back((char)((reg << 4) | r));
      sectionNode.data.push_back((char)0);
      sectionNode.data.push_back((char)0);
      sectionNode.size += 4;
      this->assembler_help_file << "Load reg direct operand: " << operand << endl;
    }
    else if (regex_search(operand, match, rx.absolute_operand_regex))
    {
      smatch match2;
      string operand = match.str(0);
      operand = operand.substr(1, operand.size());
      bool big = false;
      bool literal = false;
      int literalVal = getValue(&literal, &big, &operand);
      SectionTableNode &sectionNode = sections.at(this->current_section);
      if (literal)
      {

        poolData &current_pool = pool.at(current_section);
        if (current_pool.find(operand) != current_pool.end())
        {
          LiteralPoolTable &literal_current = current_pool.at(operand);
          int disp = literal_current.offset - location_counter;// - 4;
          sectionNode.data.push_back((char)(0x92));
          sectionNode.data.push_back((char)(reg << 4));
          sectionNode.data.push_back((char)(((0xF << 4) | ((disp >> 8) & 0xF))));
          sectionNode.data.push_back((char)(disp & 0xFF));
        }
      }
      else
      {
        if (symbols.find(operand) == symbols.end())
        {
          // Error?
        }
        else
        {
          int val = symbols.at(operand).value;
          poolData &current_pool = pool.at(current_section);
          LiteralPoolTable &literal = current_pool.at(operand);
          int disp = literal.offset - location_counter;// - 4;
          sectionNode.data.push_back((char)(0x92));
          sectionNode.data.push_back((char)(reg << 4));
          sectionNode.data.push_back((char)(((0xF << 4) | ((disp >> 8) & 0xF))));
          sectionNode.data.push_back((char)(disp & 0xFF));
        }
      }

      sectionNode.size += 4;
      this->assembler_help_file << "Load memory apsolut : " << operand << endl;
    }
    else if (regex_search(operand, match, rx.memory_direct_operand_regex))
    {
      smatch match2;
      string operand = match.str(0);
      bool big = false;
      bool literal = false;
      int literalVal = getValue(&literal, &big, &operand);
      SectionTableNode &sectionNode = sections.at(this->current_section);
      if (literal)
      {
        // bazen literala
        poolData &current_pool = pool.at(current_section);
        if (current_pool.find(operand) != current_pool.end())
        {
          LiteralPoolTable &literal_current = current_pool.at(operand);
          int disp = literal_current.offset - location_counter;// - 8;
          sectionNode.data.push_back((char)0x92);
          sectionNode.data.push_back((char)(reg << 4));
          sectionNode.data.push_back((char)(((0xF << 4) | ((disp >> 8) & 0xF))));
          sectionNode.data.push_back((char)(disp & 0xFF));
          // a u literalVal treba rastojanje do bazena literala
          sectionNode.data.push_back((char)0x92);
          sectionNode.data.push_back((char)(reg << 4));
          sectionNode.data.push_back((char)(0x00));
          sectionNode.data.push_back((char)(0x00));
          location_counter = location_counter + 4; // ali se onda ne poklapa sa prvim prolazom?
          sectionNode.size += 8;
        }
      }
      else
      {
        if (symbols.find(operand) == symbols.end())
        {
          // Error?
        }
        else
        {
          int val = symbols.at(operand).value;
          poolData &current_pool = pool.at(current_section);
          LiteralPoolTable &literal = current_pool.at(operand);
          int disp = literal.offset - location_counter;// - 8;
          sectionNode.data.push_back((char)0x92);
          sectionNode.data.push_back((char)(reg << 4));
          sectionNode.data.push_back((char)((((0xF << 4) | ((disp >> 8) & 0xF)))));
          sectionNode.data.push_back((char)(disp & 0xFF));
          // a u literalVal treba rastojanje do bazena literala
          sectionNode.data.push_back((char)0x92);
          sectionNode.data.push_back((char)(reg << 4));
          sectionNode.data.push_back((char)(0x00));
          sectionNode.data.push_back((char)(0x00));
          location_counter = location_counter + 4; // ali se onda ne poklapa sa prvim prolazom?
          sectionNode.size += 8;
        }
        this->assembler_help_file << "Load memory direct: " << operand << endl;
      }
    }
    else
    {
      cout << "Load with error parsing" << endl;
      ret = -1;
    }
    break;
  }
  return ret;
}

void Assembler::outputTables()
{

  unsigned dataInt = sections.size();
  binary_output->write((char *)(&dataInt), sizeof(unsigned));

  for (auto it = sections.begin(); it != sections.end(); ++it)
  {
    vector<char> data = it->second.data;
    convert_to_little_endian(&data, data.size());
    dataInt = (unsigned)it->second.data.size();
    binary_output->write((char *)(&dataInt), sizeof(unsigned)); // size
    unsigned len = (unsigned)it->second.name.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    // cout << "Velicina sekcije " << it->second.name << " : " << dataInt << "id: " << it->second.section_id << " address: " << it->second.address <<endl;
    binary_output->write((char *)(&it->second.section_id), sizeof(it->second.section_id));
    binary_output->write((char *)(&it->second.address), sizeof(it->second.address));
    // long data_size = it->second.data.size();
    // binary_output->write((char *)(&data_size), sizeof(data_size));
    // cout << "Duzina imena sekcije: " << len << " ime: " <<  it->second.name << endl;
    binary_output->write((char *)it->second.name.c_str(), len);
    // reverse(it->second.data.begin(), it->second.data.end()); // ne znam dal moze ovako da se prebaci na little-endian
    cout << "nece u fore? " << data.size() << endl;
    for (char c : data)
    {
      binary_output->write((char *)(&c), sizeof(c));
    }
  }

  dataInt = symbols.size();
  binary_output->write((char *)(&dataInt), sizeof(dataInt));

  for (auto it = symbols.cbegin(); it != symbols.end(); ++it)
  {
    binary_output->write((char *)(&it->second.symbol_id), sizeof(it->second.symbol_id));
    binary_output->write((char *)(&it->second.section_id), sizeof(it->second.section_id));
    char dataC = (char)((it->second.defined << 2) | (it->second.local << 1) | it->second.extern_sym);
    binary_output->write((char *)(&dataC), sizeof(dataC));
    unsigned len = (unsigned)it->second.name.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->second.name.c_str(), len);              // name
    binary_output->write((char *)(&it->second.value), sizeof(long)); // value
    len = (unsigned)it->second.section_name.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->second.section_name.c_str(), len); // section_name
    len = (unsigned)it->second.type.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->second.type.c_str(), len); // tip
  }

  dataInt = relocations.size();
  binary_output->write((char *)(&dataInt), sizeof(dataInt));

  for (auto it = relocations.cbegin(); it != relocations.end(); ++it)
  {
    unsigned len = (unsigned)it->name.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->name.c_str(), len); // name
    len = (unsigned)it->section_name.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->section_name.c_str(), len); // section_name
    // binary_output->write((char *)(&dataInt), sizeof(dataInt));
    binary_output->write((char *)(&it->relocation_id), sizeof(it->relocation_id));
    binary_output->write((char *)(&it->addend), sizeof(long));
    len = (unsigned)it->type.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->type.c_str(), len); // tip
    binary_output->write((char *)(&it->local), sizeof(bool));
    binary_output->write((char *)(&it->section), sizeof(bool));
    binary_output->write((char *)(&it->offset), sizeof(long));
    // cout << it->second.symbol_id << " " << it->first << " " << it->second.section_id << "  " << it->second.addend << "  " << it->second.value << endl;
  }
  this->assembler_help_file << "Posle upisa: " << endl;
  this->printRelocationTable();
}

void Assembler::convert_to_little_endian(vector<char> *data, int size)
{
  while (size % 4 != 0)
  {
    cout << "Ovde mi menja? " << endl;
    data->push_back((char)0x00);
    size++;
  }
  int i = 0;
  // cout << "Velcina: " << size << endl;
  auto begin = data->begin();
  while (i < size)
  {
    auto start = next(begin, i);
    auto end = next(begin, i + 4);
    reverse(start, end);
    i = i + 4;
  }
  // cout << "Izasao iz while " << size << endl;
  return;
}
