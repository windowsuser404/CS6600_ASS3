#include "../include/simulator.h"
#include <cstdlib>
#include <iostream>

using namespace std;

void Instruction::print_durs() {
  ullong IF = this->durations.IF;
  ullong ID = this->durations.ID;
  ullong EX = this->durations.EX;
  ullong IS = this->durations.IS;
  ullong WB = this->durations.WB;
  ////////////IF////////////////
  cout << "IF{" << ID - 1 << ",";
  cout << 1 << "} ";
  ////////////ID////////////////
  cout << "ID{" << ID << ",";
  cout << IS - ID << "} ";
  ////////////IS////////////////
  cout << "IS{" << IS << ",";
  cout << EX - IS << "} ";
  ////////////EX////////////////
  cout << "EX{" << EX << ",";
  cout << WB - EX << "} ";
  ////////////WB////////////////
  cout << "WB{" << WB << ",";
  cout << 1 << "} ";
}

void Instruction::print_info() {
  int src1 = this->src1;
  int src2 = this->src2;
  int dst = this->dst;
  if (src1 == REG_FILE_SIZE - 1) {
    src1 = -1;
  }
  if (src2 == REG_FILE_SIZE - 1) {
    src2 = -1;
  }
  if (dst == REG_FILE_SIZE - 1) {
    dst = -1;
  }
  cout << this->tag - REG_FILE_SIZE << " ";
  cout << "fu{" << this->op_type << "} ";
  cout << "src{" << src1 << ",";
  cout << src2 << "} ";
  cout << "dst{" << dst << "} ";

  print_durs();

  cout << endl;
}

void Instruction::decrease_latency() {
  if (latency_left == 0) {
    cout << "wrongly decreasing latency" << endl;
    exit(0);
  } else {
    latency_left--;
  }
}

void Instruction::put_state(INS_STATE state) { curr_state = state; }

Scheduling_queue::Scheduling_queue() {
  head = nullptr;
  tail = nullptr;
  size = 0;
}

Instruction::Instruction(uint op, int tag, uint S1, uint S2, uint DST,
                         bool HAS_SRC1, bool HAS_SRC2, bool HAS_DST)
    : op_type(op), tag(tag), src1(S1), src2(S2), dst(DST), has_src1(HAS_SRC1),
      has_src2(HAS_SRC2), has_dst(HAS_DST) {
  switch (op) {
  default:
    cout << "unknown types, sus" << endl;
    exit(1);
    break;
  case Type0:
    latency_left = Type0_lat;
    break;
  case Type1:
    latency_left = Type1_lat;
    break;
  case Type2:
    latency_left = Type2_lat;
    break;
  }
  // currently issue and then execute, so one "extra" cycle im adding to account
  // for this
  // latency_left++;
  durations.EX = -1;
  durations.ID = -1;
  durations.IF = -1;
  durations.IS = -1;
  durations.WB = -1;
}

void Scheduling_queue::ins_add(Scheduling_queue_entry *to_add) {
  // if head was also null, first entry
  if (head == nullptr) {
    to_add->prev_entry = nullptr;
    to_add->nxt_entry = nullptr;
    head = to_add;
    tail = to_add;
  } else {
    // head wasnt null, so non empty, add to tail
    tail->nxt_entry = to_add;
    to_add->prev_entry = tail;
    to_add->nxt_entry = nullptr;
    tail = to_add;
  }
  size++;
}

void Scheduling_queue::ins_remove(Scheduling_queue_entry *to_rem) {
  // if head==tail list will become empty now
  if (size == 0) {
    cout << "Removing from empty list, sus max" << endl;
    exit(1);
  }
  Scheduling_queue_entry *prev = to_rem->prev_entry;
  Scheduling_queue_entry *nxt = to_rem->nxt_entry;
  if (size == 1) {
    head = nullptr;
    tail = nullptr;
  } else {
    if (to_rem == head) {
      head = nxt;
      nxt->prev_entry = nullptr;
    } else if (to_rem == tail) {
      tail = prev;
      prev->nxt_entry = nullptr;
    } else {
      prev->nxt_entry = to_rem->nxt_entry;
      nxt->prev_entry = to_rem->prev_entry;
    }
  }
  size--;
}

#if DEBUG

void Scheduling_queue::print_queue() {
  cout << "ins src1 src1V src2 src2V" << endl;
  Scheduling_queue_entry *head = this->head;
  Instruction *temp;
  while (head != nullptr) {
    temp = head->curr_ins;
    cout << temp->tag - REG_FILE_SIZE + 1 << " " << head->src1.tag << " "
         << head->src1.valid << " " << head->src2.tag << " " << head->src2.valid
         << endl;
    head = head->nxt_entry;
  }
}

#endif
