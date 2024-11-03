#include "../include/simulator.h"

#define DEBUG 1

void OOOE::retire() {
  if (!ROB_table.empty()) {
    // execute till non WB is found
    while (true) {
      Instruction *rob_top = ROB_table.front();
      if (rob_top->get_state() == WB) {
        // delete the entry since it has finished
        // delete rob_top;
        ROB_table.pop();
      } else {
        // non WB entry found, stop retiring more
        break;
      }
    }
  }
}

void OOOE::execute() {
  Executing_queue_entry *curr_entry = execute_list.head;
  while (curr_entry != nullptr) {
    curr_entry->curr_ins->decrease_latency();
    if (curr_entry->curr_ins->is_finished()) {

#if DEBUG
      cout << "ins" << curr_entry->curr_ins->tag - REG_FILE_SIZE << " jover"
           << endl;
#endif

      mark_ready(curr_entry->curr_ins->dst);
      curr_entry->curr_ins->put_state(WB);
      // new curr entry made, remove the prev one from list
      execute_list.ins_remove(curr_entry);
      Executing_queue_entry *to_delete = curr_entry;
      curr_entry = curr_entry->nxt_entry;
      delete to_delete;
    } else {
      curr_entry = curr_entry->nxt_entry;
    }
  }
}

void OOOE::issue() {
  Scheduling_queue_entry *curr_entry = issue_list.head;
  uint count = 0;
  while ((curr_entry != nullptr) && (count < Dispatch_size)) {
    if (is_ready(curr_entry)) {
      Instruction *the_ins = curr_entry->curr_ins;

#if DEBUG
      cout << "issuing ins" << the_ins->tag - REG_FILE_SIZE << endl;
#endif

      the_ins->put_state(EX);
      register_array[the_ins->dst].valid = 0;
      register_array[the_ins->dst].tag = curr_entry->curr_ins->tag;
      count++;
      // adding to executing queueA
      ullong src1 = the_ins->src1;
      ullong src2 = the_ins->src2;
      Executing_queue_entry *to_add = new Executing_queue_entry(
          the_ins, register_array[src1].valid, register_array[src1].tag,
          register_array[src2].valid, register_array[src2].tag);
      execute_list.ins_add(to_add);
      issue_list.ins_remove(curr_entry);
      Scheduling_queue_entry *to_delete = curr_entry;
      curr_entry = curr_entry->nxt_entry;
      delete to_delete;
    } else {
      curr_entry = curr_entry->nxt_entry;
    }
    count++;
  }
}

void OOOE::dispatch() {
  uint count = 0;
  // hopefully dispatch_list never empty, still adding it here
  while ((issue_list.size < Schedule_size) && (count < Dispatch_size) &&
         (!dispatch_list.empty())) {
    Instruction *ins = dispatch_list.front();

#if DEBUG
    cout << "Dispatching ins" << ins->tag - REG_FILE_SIZE << endl;
#endif

    ins->put_state(IS);
    ullong src1 = ins->src1;
    ullong src2 = ins->src2;

#if DEBUG
    // cout << "src1 " << src1 << " src2 " << src2 << endl;
    // if (src1 == 4294967295 || src2 == 4294967295) {
    //   cout << "hmm -1 still lingers in " << ins->tag << endl;
    // }
#endif

    dispatch_list.pop();
    Scheduling_queue_entry *to_add = new Scheduling_queue_entry(
        ins, register_array[src1].valid, register_array[src1].tag,
        register_array[src2].valid, register_array[src2].tag);
    issue_list.ins_add(to_add);
    count++;
  }

#if DEBUG
  if (count == 0) {
    cout << "Nothing dispatched" << endl;
    cout << "Scheduling_queue = " << issue_list.size << endl;
  }
#endif
}

void OOOE::fetch() {
  uint count = 0;
  while ((dispatch_list.size() < 2 * Dispatch_size) &&
         (count < Dispatch_size)) {
    Instruction *to_dispatch;
    if (get_the_ins(to_dispatch)) {
      to_dispatch->put_state(ID);
      dispatch_list.push(to_dispatch);
      count++;
    } else {
      break;
    }
  }
}

bool OOOE::advance_cycle() {
  cycle++;
#if DEBUG
  cout << endl << "Doing cycle " << cycle << endl;
  if (ins_pointer == ALL_ins.size() - 1) {
    cout << "All instructions dispatched" << endl;
  }
  if (ROB_table.empty()) {
    cout << "Rob is empty" << endl;
  }
  if (dispatch_list.empty()) {
    cout << "Nothing to dispatch" << endl;
  }
#endif
  return (!ROB_table.empty() || !dispatch_list.empty() ||
          (ins_pointer != ALL_ins.size() - 1));
}

bool OOOE::is_ready(Scheduling_queue_entry *&ins) {
  return (ins->src2.valid && ins->src1.valid);
}

void OOOE::mark_ready(ullong &tag) {
  // marking the needed registers as ready
  for (uint i = 0; i < REG_FILE_SIZE; i++) {
    if (register_array[i].tag == tag) {
      register_array[i].valid = 1;
    }
  }

  Scheduling_queue_entry *head = issue_list.head;
  while (head != nullptr) {
    if (head->src1.tag == tag) {
      head->src1.valid = true;
    }
    if (head->src2.tag == tag) {
      head->src2.valid = true;
    }
    head = head->nxt_entry;
  }
}

bool OOOE::get_the_ins(Instruction *&to_dispatch) {
  if (ins_pointer < ALL_ins.size()) {
    to_dispatch = ALL_ins[ins_pointer];
    ins_pointer++;
    return true;
  } else {
    return false;
  }
}

void OOOE::read_ins(string filePath) {
#if DEBUG
  cout << "Reading the file for instruction" << endl;
#endif
  ifstream file(filePath);

  ullong ins_count = 0;

  if (!file.is_open()) {
    cerr << "Error opening file: " << filePath << endl;
    exit(1);
  }

  string line;
  while (getline(file, line)) {
    istringstream iss(line);
    string pc_hex;
    uint operation_type;
    int dest_reg, src1_reg, src2_reg;
    // Read each field from the line
    if (!(iss >> pc_hex >> operation_type >> dest_reg >> src1_reg >>
          src2_reg)) {
      cerr << "Error parsing line: " << line << endl;
      exit(1);
      continue;
    }
    if (src1_reg == -1) {
#if DEBUG
      // cout << "Encountered a -1" << " in" << ins_count << endl;
#endif
      src1_reg = REG_FILE_SIZE;
    }
    if (src2_reg == -1) {
#if DEBUG
      // cout << "Encountered a -1" << " in" << ins_count << endl;
#endif
      src2_reg = REG_FILE_SIZE;
    }
    if (dest_reg == -1) {
#if DEBUG
      // cout << "Encountered a -1" << " in" << ins_count << endl;
#endif
      dest_reg = REG_FILE_SIZE;
    }
#if DEBUG
    // cout << "src1 src2 dst" << src1_reg << src2_reg << dest_reg << endl;
#endif
    ////ignore for now
    int pc = stoi(pc_hex, nullptr, 16);
    /////////
    // tag is made by adding register, so as to not collide with existing
    // register tag
    Instruction *ins =
        new Instruction(operation_type, ins_count + REG_FILE_SIZE, src1_reg,
                        src2_reg, dest_reg);
    ALL_ins.push_back(ins);
    ROB_table.push(ins);
    ins_count++;
  }

  file.close();
}

OOOE::OOOE(uint N, uint S, string filepath)
    : Dispatch_size(N), Schedule_size(S) {
  ins_pointer = 0;
  cycle = 0;
  for (uint i = 0; i < REG_FILE_SIZE + 1; i++) {
    register_array.emplace_back(true, i);
  }
  read_ins(filepath);

#if DEBUG
  cout << "Total ins = " << ALL_ins.size() << endl;
#endif
}
