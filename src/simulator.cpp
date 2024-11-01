#include "../include/simulator.h"
#include <algorithm>
#include <cstddef>

void OOOE::retire() {
  if (!ROB_table.empty()) {
    // execute till non WB is found
    while (true) {
      Instruction *rob_top = ROB_table.front();
      if (rob_top->get_state() == WB) {
        // delete the entry since it has finished
        delete rob_top;
        ROB_table.pop();
      } else {
        // non WB entry found, stop retiring more
        break;
      }
    }
  }
}

void OOOE::execute() {
  Scheduling_queue_entry *curr_entry = execute_list.head;
  while (curr_entry != NULL) {
    curr_entry->curr_ins->decrease_latency();
    if (curr_entry->curr_ins->is_finished()) {
      mark_ready(curr_entry->curr_ins->dst);
      curr_entry->curr_ins->put_state(WB);
      curr_entry = curr_entry->nxt_entry;
      // new curr entry made, remove the prev one from list
      execute_list.ins_remove(curr_entry->prev_entry);
    } else {
      curr_entry = curr_entry->nxt_entry;
    }
  }
}

void OOOE::issue() {
  Scheduling_queue_entry *curr_entry = issue_list.head;
  uint count = 0;
  while ((curr_entry != NULL) && (count < Dispatch_size)) {
    Instruction *the_ins = curr_entry->curr_ins;
    if (is_ready(the_ins)) {
      register_array[the_ins->dst].valid = 0;
      register_array[the_ins->dst].tag = curr_entry->curr_ins->tag;
      count++;
      // adding to executing queueA
      Executing_queue_entry *to_add =
          new Executing_queue_entry(the_ins, the_ins->src1, the_ins->src2);
      execute_list.ins_add(to_add);
      issue_list.ins_remove(curr_entry);
      delete curr_entry;
    }
    curr_entry = curr_entry->nxt_entry;
  }
}

void OOOE::dispatch() {
  uint count = 0;
  // hopefully dispatch_list never empty, still adding it here
  while ((issue_list.size < Schedule_size) && (count < Dispatch_size) &&
         (!dispatch_list.empty())) {
    Instruction *ins = dispatch_list.front();
    dispatch_list.pop();
    Scheduling_queue_entry *to_add =
        new Scheduling_queue_entry(ins, ins->src1, ins->src2);
    issue_list.ins_add(to_add);
    count++;
  }
}

void OOOE::fetch() {
  uint count = 0;
  while ((dispatch_list.size() < 2 * Dispatch_size) &&
         (count < Dispatch_size)) {
    Instruction *to_dispatch;
    if (get_the_ins(to_dispatch)) {
      dispatch_list.push(to_dispatch);
      count++;
    } else {
      break;
    }
  }
}

bool OOOE::advance_cycle() {
  return (!ROB_table.empty() || !dispatch_list.empty());
}

OOOE::OOOE(uint N, uint S) : Dispatch_size(N), Schedule_size(S) {
  ins_pointer = 0;
  for (uint i = 0; i < REG_FILE_SIZE; i++) {
    register_array[i].tag = 0;
    register_array[i].valid = true;
  }
}
