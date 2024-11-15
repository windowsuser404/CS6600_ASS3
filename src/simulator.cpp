#include "../include/simulator.h"

void OOOE::retire() {
  // execute till non WB is found
  while (!ROB_table.empty()) {
    Instruction *rob_top = ROB_table.front();
#if DEBUG
    cout << "ROB TOP has ins" << rob_top->tag - REG_FILE_SIZE + 1 << endl;
#endif
    if (rob_top->get_state() == WB) {
// delete the entry since it has finished
// delete rob_top;
#if DEBUG
      cout << "Retiring ins" << rob_top->tag - REG_FILE_SIZE + 1 << endl;
#endif
      ROB_table.pop();
    } else {
      // non WB entry found, stop retiring more
      break;
    }
  }
}

void OOOE::execute() {
  Executing_queue_entry *curr_entry = execute_list.head;
  while (curr_entry != nullptr) {
    curr_entry->curr_ins->decrease_latency();
    if (curr_entry->curr_ins->is_finished()) {
      finished_exec_list.push_back(curr_entry);
    }
    curr_entry = curr_entry->nxt_entry;
  }
  execute_commit();
}

void OOOE::execute_commit() {
#if DEBUG
  cout << "commiting executed stuff, size=" << finished_exec_list.size()
       << endl;
#endif
  Executing_queue_entry *finished_ins;
  for (uint i = 0; i < finished_exec_list.size(); i++) {
    finished_ins = finished_exec_list[i];

#if DEBUG
    cout << "ins" << finished_ins->curr_ins->tag - REG_FILE_SIZE + 1 << " jover"
         << endl;
#endif

    mark_ready(finished_ins->curr_ins->tag);
    finished_ins->curr_ins->put_state(WB);
    finished_ins->curr_ins->durations.WB = cycle;
    // new curr entry made, remove the prev one from list
    execute_list.ins_remove(finished_ins);
    delete finished_ins;
  }
}

void OOOE::issue() {
  Scheduling_queue_entry *curr_entry = issue_list.head;
  uint count = 0;
  while ((curr_entry != nullptr) && (count < Dispatch_size)) {
    if (is_ready(curr_entry)) {
      to_execute_list.push_back(curr_entry);
      count++;
    } else {
#if DEBUG
      cout << "ins " << curr_entry->curr_ins->tag - REG_FILE_SIZE + 1
           << " is not issued" << endl;
      cout << "src1=" << curr_entry->src1.tag
           << " and has_src1=" << curr_entry->has_src1
           << " is valid=" << curr_entry->src1.valid << endl;
      cout << "src2=" << curr_entry->src2.tag
           << " and has_src2=" << curr_entry->has_src2
           << " is valid=" << curr_entry->src2.valid << endl;
#endif
    }
    curr_entry = curr_entry->nxt_entry;
  }
  issue_commit();
}

void OOOE::issue_commit() {
  Scheduling_queue_entry *to_exec;
  for (uint i = 0; i < to_execute_list.size(); i++) {
    to_exec = to_execute_list[i];
    Instruction *the_ins = to_exec->curr_ins;

#if DEBUG
    cout << "issuing ins" << the_ins->tag - REG_FILE_SIZE + 1 << endl;
#endif

    the_ins->put_state(EX);
    the_ins->durations.EX = cycle;
    // adding to executing queueA
    Register src1 = to_exec->src1;
    Register src2 = to_exec->src2;
    bool has_src1 = to_exec->has_src1;
    bool has_src2 = to_exec->has_src2;
    Executing_queue_entry *to_add =
        new Executing_queue_entry(the_ins, src1.valid, src1.tag, src2.valid,
                                  src2.tag, has_src1, has_src2);
    execute_list.ins_add(to_add);
    issue_list.ins_remove(to_exec);
    delete to_exec;
  }
}

void OOOE::dispatch() {
  uint count = 0;
  // hopefully dispatch_list never empty, still adding it here
  // since comiiting later, add count also to check the size
  while ((issue_list.size + count < Schedule_size) && (count < Dispatch_size) &&
         !((dispatch_list.size() - count) == 0)) {
    Instruction *ins = dispatch_list[count];
    to_issue_list.push_back(ins);
    count++;
    ins_disped++;
  }

#if DEBUG
  if (count == 0) {
    cout << "Nothing dispatched" << endl;
    cout << "Scheduling_queue = " << issue_list.size << endl;
  }
#endif
  dispatch_commit();
}

void OOOE::dispatch_commit() {
  Instruction *to_issue;

#if DEBUG
  cout << "scheduling " << to_issue_list.size() << " instructions" << endl;
#endif

  for (uint i = 0; i < to_issue_list.size(); i++) {
    to_issue = to_issue_list[i];

#if DEBUG
    cout << "Dispatching ins" << to_issue->tag - REG_FILE_SIZE + 1 << endl;
#endif

    to_issue->put_state(IS);
    to_issue->durations.IS = cycle;

    ullong src1 = to_issue->src1;
    ullong src2 = to_issue->src2;
    bool has_src1 = to_issue->has_src1;
    bool has_src2 = to_issue->has_src2;

    dispatch_list.pop_front();
    Scheduling_queue_entry *to_add = new Scheduling_queue_entry(
        to_issue, register_array[src1].valid, register_array[src1].tag,
        register_array[src2].valid, register_array[src2].tag, has_src1,
        has_src2);

    if (to_issue->has_dst) {
#if DEBUG
      cout << "marking reg " << to_issue->dst << " as " << to_issue->tag
           << endl;
#endif
      register_array[to_issue->dst].valid = 0;
      register_array[to_issue->dst].tag = to_issue->tag;
    }
    issue_list.ins_add(to_add);
  }
}

void OOOE::fetch() {
  uint count = 0;
  // same, add count and check if size overflowing
  while ((dispatch_list.size() + count + ins_disped < 2 * Dispatch_size) &&
         (count < Dispatch_size)) {
    Instruction *to_dispatch;
    if (get_the_ins(to_dispatch)) {
      to_dispatch_list.push_back(to_dispatch);
      count++;
    } else {
      break;
    }
  }
  fetch_commit();
}

void OOOE::fetch_commit() {
  Instruction *to_dispatch;
  for (uint i = 0; i < to_dispatch_list.size(); i++) {
    to_dispatch = to_dispatch_list[i];
    to_dispatch->put_state(ID);
    to_dispatch->durations.ID = cycle;
    dispatch_list.push_back(to_dispatch);
  }
}

bool OOOE::advance_cycle() {
  ins_disped = 0;
  // execute_commit();
  // issue_commit();
  // dispatch_commit();
  // fetch_commit();

#if DEBUG
  cout << "Printing the ROB" << endl;
  print_rob();
  cout << endl << endl;

  cout << "Printing the schedule list" << endl;
  issue_list.print_queue();
  cout << endl << endl;

  cout << "Printing the executing list" << endl;
  execute_list.print_queue();
  cout << endl << endl;
#endif

  cycle++;
  to_dispatch_list.clear();
  to_issue_list.clear();
  to_execute_list.clear();
  finished_exec_list.clear();
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
  return ((!ROB_table.empty() || !dispatch_list.empty() ||
           (ins_pointer != ALL_ins.size())) &&
          (true));
}

bool OOOE::is_ready(Scheduling_queue_entry *&ins) {
  return ((ins->src2.valid || !(ins->has_src2)) &&
          (ins->src1.valid || !(ins->has_src1)));
}

void OOOE::mark_ready(ullong &tag) {
// marking the needed registers as ready
#if DEBUG
  cout << "marking tag=" << tag << endl;
#endif
  for (uint i = 0; i < REG_FILE_SIZE; i++) {
    if (register_array[i].tag == tag) {
      register_array[i].valid = 1;
    }
  }

  Scheduling_queue_entry *head = issue_list.head;
  while (head != nullptr) {
    if (head->src1.tag == tag || !(head->has_src1)) {
      head->src1.valid = true;
    }
    if (head->src2.tag == tag || !(head->has_src2)) {
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
    bool has_src1, has_src2, has_dst;
    has_src1 = 1;
    has_src2 = 1;
    has_dst = 1;
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
      src1_reg = REG_FILE_SIZE - 1;
      has_src1 = false;
    }
    if (src2_reg == -1) {
#if DEBUG
      // cout << "Encountered a -1" << " in" << ins_count << endl;
#endif
      src2_reg = REG_FILE_SIZE - 1;
      has_src2 = false;
    }
    if (dest_reg == -1) {
#if DEBUG
      // cout << "Encountered a -1" << " in" << ins_count << endl;
#endif
      dest_reg = REG_FILE_SIZE - 1;
      has_dst = false;
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
                        src2_reg, dest_reg, has_src1, has_src2, has_dst);
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

#if DEBUG
void OOOE::print_rob() {
  //
}
#endif

void OOOE::print_output() {
  for (uint i = 0; i < ALL_ins.size(); i++) {
    ALL_ins[i]->print_info();
  }
}
