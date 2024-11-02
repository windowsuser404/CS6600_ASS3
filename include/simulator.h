#pragma once
#ifndef SIM_H
#define SIM_H

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using uint = unsigned int;
using ullong = unsigned long long;

void parser();

class OOOE;
class Scheduling_queue_entry;
class Scheduling_queue;

// pre define latency cycle
#define Type0 0
#define Type1 1
#define Type2 2
#define Type0_lat 1
#define Type1_lat 2
#define Type2_lat 10

#define REG_FILE_SIZE 128

enum INS_STATE { IF, ID, IS, EX, WB };

typedef ullong Reg_number;

class Register {
  friend OOOE;

private:
  bool valid;
  ullong tag;

public:
  Register(bool valid, ullong tag) {
    this->valid = valid;
    this->tag = tag;
  }
};

class Instruction {
  friend OOOE;

private:
  INS_STATE curr_state;
  uint latency_left;
  uint op_type;
  int tag;
  Reg_number dst;
  Reg_number src1;
  Reg_number src2;

public:
  Instruction(uint op, int tag, uint S1, uint S2, uint DST);
  void decrease_latency();
  bool is_finished() { return (latency_left == 0); }
  void put_state(INS_STATE state);
  INS_STATE get_state() { return curr_state; }
};

class Scheduling_queue_entry {
  friend OOOE;
  friend Scheduling_queue;

private:
  Scheduling_queue_entry *nxt_entry;
  Scheduling_queue_entry *prev_entry;
  Instruction *curr_ins;
  Register src1;
  Register src2;

public:
  Scheduling_queue_entry(Instruction *&ins, bool src1_stat, ullong src1_tag,
                         bool src2_stat, ullong src2_tag)
      : curr_ins(ins), src1(src1_stat, src1_tag), src2(src2_stat, src2_tag) {
    nxt_entry = nullptr;
    prev_entry = nullptr;
  }
};

class Scheduling_queue {
  friend OOOE;

private:
  Scheduling_queue_entry *head;
  Scheduling_queue_entry *tail;
  ullong size;

public:
  void ins_add(
      Scheduling_queue_entry *to_add); // dont forget to check if list was empty
  void ins_remove(
      Scheduling_queue_entry *to_rem); // dont forget to check if removing head
  ullong get_size() { return size; }
  Scheduling_queue();
};

// Copying same code as Scheduling, the needed functionality seems same,
// actually, doesnt need source registers, so can actually remove those later
using Executing_queue_entry = Scheduling_queue_entry;
using Executing_queue = Scheduling_queue;

// out of order executer
class OOOE {

private:
  vector<Instruction *> ALL_ins;
  queue<Instruction *> dispatch_list;
  Scheduling_queue issue_list;
  Executing_queue execute_list;
  queue<Instruction *> ROB_table;
  // register file, stores whether valid, the tag
  vector<Register> register_array = vector<Register>(REG_FILE_SIZE);
  uint Dispatch_size; // N
  uint Schedule_size; // S
  // which instruction is being pointed to
  uint ins_pointer;

  // functions
  // checks if given instruction is ready
  bool is_ready(Scheduling_queue_entry *&ins);
  // marks the relevant registers and stations as ready
  void mark_ready(ullong &tag);
  // get the next instruction, return false if no more to dispatch
  bool get_the_ins(Instruction *&to_dispatch);
  // load all ins
  void read_ins(string filepath);

public:
  OOOE(uint N, uint S, string filepath);
  void retire();
  void execute();
  void issue();
  void dispatch();
  void fetch();
  bool advance_cycle();
};

#endif
