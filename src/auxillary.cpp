#include "../include/simulator.h"
#include <cstdlib>
#include <iostream>

using namespace std;

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
    cout << temp->tag - REG_FILE_SIZE << " " << head->src1.tag << " "
         << head->src1.valid << " " << head->src2.tag << " " << head->src2.valid
         << endl;
    head = head->nxt_entry;
  }
}

#endif
