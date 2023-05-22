#include "../../inc/assembler.h"

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
    current_line = regex_replace(current_line, rx.tab, " ");
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

  // for (string s: lines){
  //   cout << s << endl;
  // }

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
  return this->process_second_pass();
}

int Assembler::process_second_pass(){
  int ret = 0;
  for (string line : lines) {
    if (this->end)
      break;
    this->current_line++;
    smatch match;
    if (!regex_search(line, match, rx.only_label) and !regex_search(line, match, rx.label_with_data)){
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
      symbol.defined = true;
      symbol.value = location_counter;
      symbol.section_id = this->_section_id;
      symbol.section_name = this->current_section;
      symbol.name = label;
    }
  }
  return 0;
}

int Assembler::process_command(string command, bool first)
{
  // cout << "Zove proces command:" << command << endl;
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
    if(first)
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
      ret = call_inst_second(match);
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
  this->assembler_help_file << "Symbol_id Symbol_name  Section_id Section_name defined local extern" << endl;
  for (auto it = symbols.cbegin(); it != symbols.end(); ++it)
  {
    this->assembler_help_file << it->second.symbol_id << " " << it->first << " " << it->second.section_id << "  " << it->second.section_name << "  " << it->second.defined << " " << it->second.local << "  " << it->second.extern_sym << endl;
  }
}

void Assembler::printRelocationTable()
{
  this->assembler_help_file << endl
                            << "Relocation table" << endl;
  this->assembler_help_file << "Symbol_id Symbol_name  Section_id type addend value" << endl;
  for (auto it = relocations.cbegin(); it != relocations.end(); ++it)
  {
    this->assembler_help_file << it->second.symbol_id << " " << it->first << " " << it->second.section_id << "  " << it->second.addend << "  " << it->second.value << endl;
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

int Assembler::process_extern_dir(smatch match)
{
  int ret = 0;
  string extern_label = ((string)match[0]);
  list<string> symbol_list = this->split(extern_label.substr(extern_label.find(" ") + 1, extern_label.size()), ",");
  extern_label = extern_label.substr(0, extern_label.find(" "));
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
  for (string s : symbol_list)
  {
    if (symbols.find(s) == symbols.end())
    {
      // not found

      SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, 0, false, false, false);
      symbol->name = s;
      symbol->type = "NOTYP";
      symbol->section_name = current_section; // ili UND?
      symbols.insert({s, *symbol});
      assembler_help_file << ".global "
                          << "symbol_name: " << s << " location counter: " << location_counter << " section: " << symbol->section_name << endl;
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
  string section_name = section_label.substr(section_label.find(" "), section_label.size());
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
      // current_section_node.size = location_counter;
      assembler_help_file << "End of section: " << current_section_node.name << " with size: " << current_section_node.size << endl;
    }
    else
    {
      SectionTableNode *current_section_node = new SectionTableNode(this->current_section, this->location_counter, 0, this->current_section_id);
      // current_section_node.size = location_counter;
      sections.insert({this->current_section, *current_section_node});
      assembler_help_file << "End of section: " << current_section_node->name << " with size: " << current_section_node->size << endl;
    }
  }
  SectionTableNode *newSection = new SectionTableNode(section_name, this->location_counter, 0, this->_section_id);
  sections.insert({section_name, *newSection});

  this->current_section = section_name;
  this->location_counter = 0;
  this->current_section_id = _section_id;

  return ret;
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
  this->location_counter = this->location_counter + symbol_list.size()*4; // uvec za alociran prostor sa word
  return ret;
}

int Assembler::word_dir_second(smatch match){
  SectionTableNode &sectionNode = sections.at(this->current_section);
  string word_label = ((string)match[0]);
  list<string> symbol_list = this->split(word_label.substr(word_label.find(" ") + 1, word_label.size()), ",");
  word_label = word_label.substr(0, word_label.find(" "));
  for (string s : symbol_list)
  {
    // ako je broj bacim allocate za toliki prostor samo
    if (regex_match(s, rx.integer))
    {
      int dataInt = stoi(s) & 0xFFFFFFFF;
      sectionNode.data.push_back((int)dataInt);
      dataInt = (stoi(s) >> 16) & 0xFFFF0000;
      sectionNode.data.push_back((int)dataInt);
      assembler_help_file << "Word directive with number: " << s << "location_counter"
                          << " : " << location_counter << endl;
    }
    else if (regex_match(s, rx.hex))
    {
      int dataInt = stoi(s, 0, 32) & 0xFFFFFFFF;
      sectionNode.data.push_back((int)dataInt);
      dataInt = (stoi(s, 0, 32) >> 16) & 0xFFFF0000;
      sectionNode.data.push_back((int)dataInt);
      assembler_help_file << "Word directive with hex number: " << s << "location_counter"
                          << " : " << location_counter << endl;
    }
    else if (regex_match(s, rx.sym_regex))
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
          sectionNode.data.push_back((int)(symbol.value & 0xFFFF));
          sectionNode.data.push_back((int)((symbol.value >> 16) & 0xFFFF));
          //
          RelocationTableNode *relocation_data = new RelocationTableNode(symbol.symbol_id, symbol.section_id, symbol.name);
          relocation_data->value = current_line;
          relocation_data->type = "R_X86_64_32";
          relocation_data->addend = 0;
          relocations.insert({symbol.name, *relocation_data});
        }
        else
        {
          char temp = (int)(0 & 0xFFFF);
          sectionNode.data.push_back(temp);
          temp = (int)((0 >> 16) & 0xFFFF);
          sectionNode.data.push_back(temp);

          symbol.value = 0;
          symbol.offset = current_line;
          symbol.section_id = current_section_id;
          symbol.section_name = current_section;
          symbol.section_name = current_section;
          RelocationTableNode *relocation_data = new RelocationTableNode(symbol.symbol_id, symbol.section_id, symbol.name);
          relocation_data->value = current_line;
          relocation_data->type = "R_X86_64_32";
          relocation_data->addend = 0;
          relocations.insert({symbol.name, *relocation_data});
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
      cout << "Wrong word argument format " << endl
           << "Error at line: " << this->current_line << "za symbol:" << s << endl;
      return -1;
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
  if (regex_match(skip_val, rx.integer))
  {
    location_counter += stoi(skip_val, 0, 32);
    assembler_help_file << "first pass .skip  with value: " << stoi(skip_val) << endl;
  }
  else if (regex_match(skip_val, rx.hex))
  {
    location_counter += stoi(skip_val, 0, 32);
    assembler_help_file << "first pass .skip  with value: " << stoi(skip_val, 0, 32) << endl;
  }

  return ret;
}

int Assembler::skip_dir_second(smatch match) {
  int ret = 0;
  string skip_label = ((string)match[0]);
  string skip_val = skip_label.substr(skip_label.find(" "), skip_label.size());
  skip_label = skip_label.substr(0, skip_label.find(" "));
   SectionTableNode &sectionNode = sections.at(this->current_section);
  if (regex_match(skip_val, rx.integer))
  {
    location_counter += stoi(skip_val, 0, 32);
    sectionNode.size += stoi(skip_val, 0, 32);
    assembler_help_file << "second pass .skip  with value: " << stoi(skip_val) << endl;
    for (int i = 0; i <stoi(skip_val, 0, 32); i++)
    {
      int temp = 0 & 0xFFFF;
      sectionNode.data.push_back(temp);
    }
  }
  else if (regex_match(skip_val, rx.hex))
  {
    location_counter += stoi(skip_val, 0, 32);
    sectionNode.size += stoi(skip_val, 0, 32);
    assembler_help_file << "second pass .skip  with value: " << stoi(skip_val, 0, 32) << endl;
    for (int i = 0; i < stoi(skip_val, 0, 32); i++)
    {
      int temp = 0 & 0xFFFF;
      sectionNode.data.push_back(temp);
    }
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

int Assembler::ascii_dir_second(smatch match){
  int ret = 0;
  string ascii = match.str(0);
  SectionTableNode &sectionNode = sections.at(this->current_section);
  for (int i = 0; i < ascii.size(); i++)
  {
    char temp = ascii[i];
    sectionNode.data.push_back(temp);
    sectionNode.size++;
    location_counter++;
  }
  return ret;
}
int Assembler::process_end_dir(smatch match)
{
  int ret = 0;
  assembler_help_file << ".end " << endl;
  this->end = true;
  return ret;
}
int Assembler::end_dir_second(smatch match) {
  // da proverim dal su svi definisani simboli
  int ret = 0;
  if (current_section != "")
  {
    if (sections.find(this->current_section) == sections.end())
    {
      SectionTableNode *current_section_node = new SectionTableNode(this->current_section, this->location_counter, 0, this->current_section_id);
      sections.insert({this->current_section, *current_section_node});
      assembler_help_file << ".end of section: " << current_section_node->name << " with size: " << current_section_node->size << endl;
    }
    else
    {
      SectionTableNode &current_section_node = sections.at(this->current_section);
      // current_section_node.size = location_counter;
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
  this->end = true;
  return ret;
}

list<string> Assembler::split(string s, string delimeter)
{
  list<string> ret;
  int pos = 0;
  // cout << "U split : ";
  while ((pos = s.find(delimeter)) != string::npos)
  {
    ret.push_back(s.substr(0, pos));
    // cout << s.substr(0, pos) << endl;
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
  location_counter+= 4;

  return ret;
}

int  Assembler::halt_inst_second(smatch match){
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
  char temp = HALT;
  sectionNode.data.push_back(temp);
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((int)0);
  sectionNode.size+= 4;
  location_counter+= 4;
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

  // push status; push pc; cause<=4; status<=status&(~0x1); pc<=handle;
  // push r0;
  // push r15;
  // ld r2, #4
  // ???
  // ld r15, r1
  // status mi je r0, cause mi je r2 handler mi je r1
  // da li ovo znaci da ja treba da maskiram prekide da bi se to izvrsilo atomicno
  location_counter+= 4;
  return ret;
}

int Assembler::int_inst_second(smatch match) {
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  string int_label = ((string)match[0]);

  assembler_help_file << "int "<< endl;
  sectionNode.data.push_back(INT);
  sectionNode.data.push_back((char)0);
  sectionNode.data.push_back((int)0);
  sectionNode.size+= 4;
  location_counter+= 4;
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
int Assembler::iret_inst_second(smatch match){
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
  
  sectionNode.size+= 8;
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
  location_counter += 4;


  return ret;
}
int Assembler::call_inst_second(smatch match){
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back((char)CALL);
  sectionNode.data.push_back((char)0);

  ret = process_operand(match.str(2), false); 
  //ovde treba da obradim da mi operand stavlja u bazen literala
  // i onda dips u najniza 12 bita do toga u bazenu literala
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

  location_counter+= 4;
  return ret;
}
int Assembler::ret_inst_second(smatch match) {
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
  sectionNode.size+=4;
  location_counter+=4;
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
  location_counter+= 4; 
  return ret;
}
int Assembler::jmp_inst_second(smatch match) {
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(JMP);
  sectionNode.data.push_back((char)0);
  assembler_help_file << ".jmp with operand: " << match.str(2) << endl;
  ret = process_operand(match.str(2), false); 
  //ovde treba da obradim da mi operand stavlja u bazen literala
  // i onda dips u najniza 12 bita do toga u bazenu literala
  sectionNode.size+=4;
  location_counter+=4;
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
  location_counter+= 4;
  
  return ret;
}
int Assembler::beq_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.find(","));
  string operand = reg.substr(reg.find(",") + 1, reg.size());
  operand = operand.substr(operand.find(",") + 1, operand.size());

  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2); //%r
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
  assembler_help_file << "beq: " << val1 << " , " << val2 << " , " << operand << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(BEQ);
  sectionNode.data.push_back((char)(val1));
  sectionNode.data.push_back((char)(val1 << 4));

  ret = process_operand(operand, false); 
  // sad ovde treba u najnizih 12b da spakujem operand tj adresu, ili odstojanje od
  // bazena literala gde se nalazi tacna adresa 
  sectionNode.size+= 4;
  location_counter+= 4;
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
  location_counter+=4;
  
  return ret;
}
int Assembler::bne_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.find(","));
  string operand = reg.substr(reg.find(",") + 1, reg.size());
  operand = operand.substr(operand.find(",") + 1, operand.size());
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2); //%r
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
  assembler_help_file << "bne: " << val1 << " , " << val2 << " , " << operand << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(BNE);
  sectionNode.data.push_back((char)(val1));
  sectionNode.data.push_back((char)(val1 << 4));

  ret = process_operand(operand, false);
  // sad ovde treba u najnizih 12b da spakujem operand tj adresu, ili odstojanje od
  // bazena literala gde se nalazi tacna adresa 
  sectionNode.size+=4;
  location_counter+=4;
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
  location_counter+=4;

  return ret;
}
int Assembler::bgt_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.find(","));
  string operand = reg.substr(reg.find(",") + 1, reg.size());
  operand = operand.substr(operand.find(",") + 1, operand.size());
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2); //%r
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
  assembler_help_file << "bgt: " << val1 << " , " << val2 << " , " << operand << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(BGT);
  sectionNode.data.push_back((char)(val1));
  sectionNode.data.push_back((char)(val1 << 4));
  
  ret = process_operand(operand, false);
  // sad ovde treba u najnizih 12b da spakujem operand tj adresu, ili odstojanje od
  // bazena literala gde se nalazi tacna adresa 
  sectionNode.size+= 4;
  location_counter+= 4;
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
  location_counter+= 4;
  return ret;
}
int Assembler::push_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  if (reg == "pc")
  {
    val = 15;
  } else if (reg == "sp") {
    val = 14;
  }
  else
  {
    reg.erase(0, 2);
    val = stoi(reg);
  }
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
int Assembler::pop_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  if (reg == "pc")
  {
    val = 15;
  } else if (reg == "sp") {
    val = 14;
  }
  else
  {
    reg.erase(0, 2);
    val = stoi(reg);
  }
  assembler_help_file << "pop: " << val << " location_counter: " << location_counter << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(POP);
  //ovaj ostatak je prepisan od prosle godine???
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
int Assembler::xchg_inst_second(smatch match){
  int ret = 0;
   string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
int Assembler::add_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  //moram da odvojim unapred slucaj kad je pc i sp
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2); //%r
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
  assembler_help_file << "add: " << val1 << " + " << val2 << endl;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(ADD);
  sectionNode.data.push_back((char)((val2 << 4) | val2));   
  sectionNode.data.push_back((char)((val1 << 4)));
  sectionNode.data.push_back((char)0);
  sectionNode.size += 4;
  location_counter += 4;
  // cout << "Processing add " << val1 << " + " << val2 << endl;
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
  location_counter+= 2;
  return ret;
}
int Assembler::sub_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  // cout << "Registri su sub:  " << r1 << " " << r2 << endl;
    if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
  location_counter+= 4;
  return ret;
}
int Assembler::mul_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  // cout << "Registri su mul: " << r1 << " " << r2 << endl;
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
int Assembler::div_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  // cout << "Registri su div: " << r1 << " " << r2 << endl;
    if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
int Assembler::not_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  if (reg == "pc")
  {
    val = 15;
  }else if (reg == "sp"){
    val = 14;
  }
  else
  {
    reg.erase(0, 2);
    val = stoi(reg);
  }
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
int Assembler::and_inst_second(smatch match){
  int ret = 0;
   string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(" ") - 1);
  string r2 = reg.substr(reg.find(" ") + 1, reg.size());
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
int Assembler::or_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  // cout << "Registri su or: " << r1 << " " << r2 << endl;
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
int Assembler::xor_inst_second(smatch match){
  int ret = 0;
   string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  // cout << "Registri su xor: " << r1 << " " << r2 << endl;
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
int Assembler::shl_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  // cout << "Registri su shl: " << r1 << " " << r2 << endl;
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
int Assembler::shr_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  // cout << "Registri su shr: " << r1 << " " << r2 << endl;
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
  location_counter += 4;
  return ret;
}
int Assembler::ld_inst_second(smatch match){
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(0x8);
  sectionNode.size++;
  string operand = match.str(1);
  assembler_help_file << "ld sa operandom "  << operand << endl;
  ret = process_operand(operand, false);
  // cout << endl
  //      << operand << " ldr operand, ldr reg: " << endl
  //      << operand << endl
  //      << reg << endl;
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
  location_counter += 4;
  return ret;
}
int Assembler::st_inst_second(smatch match){
  int ret = 0;
  SectionTableNode &sectionNode = sections.at(this->current_section);
  sectionNode.data.push_back(0x9);
  string operand = match.str(2);
  assembler_help_file << "str sa operandom: " << operand << endl;
  ret = process_operand(operand, true);
  // cout << endl
  //      << operand << " str operand, str reg: " << reg << endl;
  location_counter += 4;
  return ret;
}
int Assembler::process_csrrd_inst(smatch match){
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
int Assembler::csrrd_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(1, reg.find(",")-1);
  string r2 = reg.substr(reg.find(",") + 1, reg.size());
  if (r1 == "status")
  {
    val1 = 0;
  } else if (r1 == "handler") {
    val1 = 1;
  }
  else if (r1 == "cause")
  {
    val1 = 2;
  } else {
    cout << "Csrrd invalied control register " << r1 << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  if (r2 == "pc")
  {
    val2 = 15;
  } else if (r2 == "sp") {
    val2 = 14;
  }
  else
  {
    r2.erase(0, 2);
    val2 = stoi(r2);
  }
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
int Assembler::process_csrwr_inst(smatch match){
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
int Assembler::csrwr_inst_second(smatch match){
  int ret = 0;
  string int_label = ((string)match[0]);
  int val1 = 0;
  int val2 = 0;
  string reg = int_label.substr(int_label.find(" ") + 1, int_label.size());
  string r1 = reg.substr(0, reg.find(","));
  string r2 = reg.substr(reg.find(",") + 2, reg.size());
  if (r2 == "status")
  {
    val2 = 0;
  } else if (r2 == "handler") {
    val2 = 1;
  }
  else if (r2 == "cause")
  {
    val2 = 2;
  } else {
    cout << "Csrrw invalied control register " << r2 << endl
         << "Error at line: " << this->current_line << endl;
    return -1;
  }
  if (r1 == "pc")
  {
    val1 = 15;
  } else if (r1 == "sp") {
    val1 = 14;
  }
  else
  {
    r1.erase(0, 2);
    val1 = stoi(r1);
  }
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
int Assembler::process_operand(string operand, bool load_store)
{
  smatch match;
  int ret = 0;
  switch (load_store)
  {
  case false:
    if (regex_search(operand, match, rx.jump_absolute_operand_regex))
    {
      char dataInt;
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back(0xff);
      sectionNode.data.push_back(0x00);
      operand = operand.substr(1, operand.size());
      if (regex_match(operand, rx.integer))
      {
        this->assembler_help_file << "Absolute operand: " << operand << endl;
        dataInt = stoi(operand);
        SectionTableNode &sectionNode = sections.at(this->current_section);
        sectionNode.data.push_back((char)(dataInt >> 8));
        sectionNode.data.push_back((char)dataInt);
      }
      else if (regex_match(operand, rx.hex))
      {
        this->assembler_help_file << "Absolute operand hex: " << operand << endl;
        dataInt = stoi(operand, 0, 16);
        SectionTableNode &sectionNode = sections.at(this->current_section);
        sectionNode.data.push_back((char)(dataInt >> 8));
        sectionNode.data.push_back((char)dataInt);
      }
      else if (regex_match(operand, rx.sym_regex))
      {
        // ako je simbol i nemam radim sta?
        SectionTableNode &sectionNode = sections.at(this->current_section);
        sectionNode.data.push_back(0x00);
        sectionNode.data.push_back(0x00);
        if (symbols.find(operand) == symbols.end())
        {
          // not defined symbol
          SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, 0, true, false, false);
          symbol->name = operand;
          symbol->value = 0;
          symbol->section_name = "UND";
          symbol->type = "NOTYP";
          symbol->offset = current_line;
          symbols.insert({operand, *symbol});
          // i treba da ga dodam u relokacionu tabelu?
          RelocationTableNode *relocation_data = new RelocationTableNode(symbol->symbol_id, 0, symbol->name);
          relocation_data->value = current_line;
          relocation_data->type = "TYPE_16";
          relocation_data->addend = 0;
          relocation_data->offset = location_counter + 2;
          relocations.insert({symbol->name, *relocation_data});
        }
      }
      sectionNode.size += 4;
      location_counter += 4;
      this->assembler_help_file << "Jump absolute operand: " << operand << endl;
    }
    else
    {
      cout << "Jump with error parsing" << endl;
      ret = -1;
    }
    break;

  case true:
    if (regex_search(operand, match, rx.memory_direct_operand_regex))
    {
      int r = 0; //to nemam sad    
      char dataInt;
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)((r << 4) | 0xf));
      sectionNode.data.push_back(0x04);
      operand = operand.substr(0, operand.size());
      if (regex_match(operand, rx.integer))
      {
        this->assembler_help_file << "Memmory operand: " << operand << endl;
        dataInt = stoi(operand);
        sectionNode.data.push_back((char)(dataInt >> 8));
        sectionNode.data.push_back((char)dataInt);
      }
      else if (regex_match(operand, rx.hex))
      {
        this->assembler_help_file << "Memmory operand hex: " << operand << endl;
        dataInt = stoi(operand, 0, 16);
        sectionNode.data.push_back((char)(dataInt >> 8));
        sectionNode.data.push_back((char)dataInt);
      }
      else if (regex_match(operand, rx.sym_regex))
      {
        // ako je simbol i nemam radim sta?
        sectionNode.data.push_back(0x00);
        sectionNode.data.push_back(0x00);
        if (symbols.find(operand) == symbols.end())
        {
          // not defined symbol
          SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, this->_section_id, true, false, false);
          symbol->name = operand;
          symbol->value = 0;
          symbol->section_name = this->current_section;
          symbol->offset = current_line;
          symbols.insert({operand, *symbol});
          // i treba da ga dodam u relokacionu tabelu?
          RelocationTableNode *relocation_data = new RelocationTableNode(symbol->symbol_id, symbol->section_id, symbol->name);
          relocation_data->value = current_line;
          relocation_data->addend = 0;
          relocations.insert({symbol->name, *relocation_data});
        }
      }
      sectionNode.size += 4;
      location_counter += 4;
      this->assembler_help_file << "Load/store memory direct operand: " << operand << endl;
    }
    else if (regex_search(operand, match, rx.absolute_operand_regex))
    {
      int r = 0; //to nemam sad
      char dataInt;
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)((r << 4) | 0xf));
      sectionNode.data.push_back(0x00);
      operand = operand.substr(1, operand.size());
      if (regex_match(operand, rx.integer))
      {
        this->assembler_help_file << "Absolute operand: " << operand << endl;
        dataInt = stoi(operand);
        sectionNode.data.push_back((char)(dataInt >> 8));
        sectionNode.data.push_back((char)dataInt);
      }
      else if (regex_match(operand, rx.hex))
      {
        this->assembler_help_file << "Absolute operand hex: " << operand << endl;
        dataInt = stoi(operand, 0, 16);
        sectionNode.data.push_back((char)(dataInt >> 8));
        sectionNode.data.push_back((char)dataInt);
      }
      else if (regex_match(operand, rx.sym_regex))
      {
        // ako je simbol i nemam radim sta?
        sectionNode.data.push_back(0x00);
        sectionNode.data.push_back(0x00);
        if (symbols.find(operand) == symbols.end())
        {
          // not defined symbol
          SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, this->_section_id, true, false, false);
          symbol->name = operand;
          symbol->value = 0;
          symbol->section_name = this->current_section;
          symbol->offset = current_line;
          symbols.insert({operand, *symbol});
          // i treba da ga dodam u relokacionu tabelu?
          RelocationTableNode *relocation_data = new RelocationTableNode(symbol->symbol_id, symbol->section_id, symbol->name);
          relocation_data->value = current_line;
          relocation_data->addend = 0;
          relocations.insert({symbol->name, *relocation_data});
        }
      }
      this->assembler_help_file << "Load/store absolute operand: " << operand << endl;
      location_counter += 4;
      sectionNode.size += 4;
    }
    else if (regex_search(operand, match, rx.reg_dir_regex))
    {
      int r = 0; //to nemam sad
      int r2 = 0; //nemam 
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)((r2 << 4) | r));
      sectionNode.data.push_back(0x01);
      sectionNode.size += 2;

      location_counter += 2;
      this->assembler_help_file << "Load/store reg direct operand: " << operand << endl;
    }
    else if (regex_search(operand, match, rx.register_relative_operand_regex))
    {
      // location_counter += 2;
      int reg = 0;
      smatch match2;
      string reg2;
      if (regex_search(operand, match2, rx.reg_regex))
      {
        reg2 = match2.str(1);
      }
      string displacment = operand.substr(operand.find("+") + 2, operand.size());
      displacment = displacment.substr(0, displacment.size() - 1);

      int r = 0;//nemam
      int r2 = 0; //nemam
      char dataInt;
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)((r << 4) | r2));
      sectionNode.data.push_back(0x03);
      // Sad mi treba sta je displaysment
      if (regex_match(displacment, rx.integer))
      {
        this->assembler_help_file << "Displaycment je int: " << displacment << endl;
        dataInt = stoi(displacment);
        sectionNode.data.push_back((char)(dataInt >> 8));
        sectionNode.data.push_back((char)dataInt);
      }
      else if (regex_match(displacment, rx.hex))
      {
        this->assembler_help_file << "Displaycment je hex: " << displacment << endl;
        dataInt = stoi(displacment, 0, 16);
        sectionNode.data.push_back((char)(dataInt >> 8));
        sectionNode.data.push_back((char)dataInt);
      }
      else if (regex_match(displacment, rx.sym_regex))
      {
        this->assembler_help_file << "Displaycment je symbol: " << displacment << endl;
        sectionNode.data.push_back(0x00);
        sectionNode.data.push_back(0x00);
        if (symbols.find(operand) == symbols.end())
        {
          // not defined symbol
          SymbolTableNode *symbol = new SymbolTableNode(++this->_symbol_id, this->_section_id, true, false, false);
          symbol->name = displacment;
          symbol->value = 0;
          symbol->section_name = this->current_section;
          symbol->offset = current_line;
          symbols.insert({displacment, *symbol});
          // i treba da ga dodam u relokacionu tabelu?
          RelocationTableNode *relocation_data = new RelocationTableNode(symbol->symbol_id, symbol->section_id, symbol->name);
          relocation_data->value = current_line;
          relocation_data->addend = 0;
          relocations.insert({symbol->name, *relocation_data});
        }
      }
      sectionNode.size += 4;
      location_counter += 4;
      this->assembler_help_file << "Load/store reg indirect with displaysment operand: " << operand << "with reg: " << reg << " and reg2: " << reg2 << " and displaycment: " << displacment << endl;
    }
    else if (regex_search(operand, match, rx.register_absolute_operand_regex))
    {
      //[r0]
      int r = 0;
      int r2 = 0;
      //nemaju ova 2 poslata odmah
      SectionTableNode &sectionNode = sections.at(this->current_section);
      sectionNode.data.push_back((char)((r << 4) | r2));
      sectionNode.data.push_back(0x02);
      sectionNode.size += 2;
      location_counter += 2;
      this->assembler_help_file << "Load/store reg indirect operand: " << endl;
    }
    else
    {
      cout << "Load/store with error parsing" << endl;
      ret = -1;
    }
    break;
  }
  return ret;
}

void Assembler::outputTables()
{

  int dataInt = sections.size();
  binary_output->write((char *)(&dataInt), sizeof(dataInt));

  for (auto it = sections.cbegin(); it != sections.end(); ++it)
  {
    dataInt = it->second.data.size();
    binary_output->write((char *)(&it->second.section_id), sizeof(it->second.section_id));
    binary_output->write((char *)(&it->second.address), sizeof(it->second.address));
    // binary_output->write((char *)(&it->second.size), sizeof(it->second.size));
    binary_output->write((char *)(&dataInt), sizeof(dataInt));
    unsigned len = (unsigned)it->second.name.size();
    // cout << "Duzina imena sekcije: " << len << " ime: " <<  it->second.name << endl;
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->second.name.c_str(), len);
    for (char c : it->second.data)
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
    binary_output->write(it->second.name.c_str(), len);                          // name
    binary_output->write((char *)(&it->second.value), sizeof(it->second.value)); // value
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
    unsigned len = (unsigned)it->second.name.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->second.name.c_str(), len); // name
    // binary_output->write((char *)(&dataInt), sizeof(dataInt));
    binary_output->write((char *)(&it->second.symbol_id), sizeof(it->second.symbol_id));
    binary_output->write((char *)(&it->second.section_id), sizeof(it->second.section_id));
    binary_output->write((char *)(&it->second.addend), sizeof(it->second.addend));
    binary_output->write((char *)(&it->second.value), sizeof(it->second.value));
    len = (unsigned)it->second.type.size();
    binary_output->write((char *)(&len), sizeof(unsigned));
    binary_output->write(it->second.type.c_str(), len); // tip
    // cout << it->second.symbol_id << " " << it->first << " " << it->second.section_id << "  " << it->second.addend << "  " << it->second.value << endl;
  }
  this->assembler_help_file << "Posle upisa: " << endl;
  this->printRelocationTable();
}