#include <queue>
#include <utility>
#include <vector>

using namespace std;

using uint = unsigned int;
using ullong = unsigned long long;

void parser();

class OOOE;

// pre define latency cycle
#define Type0 0
#define Type1 1
#define Type2 2
#define Type0_lat 1
#define Type1_lat 2
#define Type2_lat 10

#define REG_FILE_SIZE 128

enum INS_STATE { IF, ID, IS, EX, WB };

typedef uint Reg_number;

class Register {
  friend OOOE;

private:
  bool valid;
  ullong tag;
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
  void initiate();
  void decrease_latency();
  bool is_finished() { return (latency_left == 0); }
  void put_state(INS_STATE state);
  INS_STATE get_state() { return curr_state; }
};

// class ROB_entry {
// private:
//   Instruction *instruction;
//   bool src1_stat;
//   bool src2_stat;
//   int tag;
//
// public:
// };

// a linked list for scheduling, could be a struct tbh, but keep it as a class
// for now
class Scheduling_queue_entry {
  friend OOOE;

private:
  Scheduling_queue_entry *nxt_entry;
  Scheduling_queue_entry *prev_entry;
  Instruction *curr_ins;
  Register src1;
  Register src2;

public:
  Scheduling_queue_entry(Instruction *&ins, Reg_number &S1, Reg_number &S2);
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
};

// Copying same code as Scheduling, the needed functionality seems same
// class Executing_queue_entry {
//   Executing_queue_entry *nxt_entry;
//   Executing_queue_entry *prev_entry;
//   Instruction *curr_ins;
// };
//
// class Executing_queue {
//   Executing_queue_entry *head;
//   ullong size;
// };

// Copying same code as Scheduling, the needed functionality seems same
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
  bool is_ready(Instruction *&ins);
  // marks the relevant registers and stations as ready
  void mark_ready(uint &tag);
  // get the next instruction, return false if no more to dispatch
  bool get_the_ins(Instruction *&to_dispatch);
  // load all ins
  void read_ins();

public:
  OOOE(uint Dispatch_size, uint Schedule_size);
  void retire();
  void execute();
  void issue();
  void dispatch();
  void fetch();
  bool advance_cycle();
};
