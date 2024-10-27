#include <vector>

using namespace std;

void parser();

enum INS_STATE { IF, ID, IS, EX, WB };

typedef int Reg_number;

class Instruction {
private:
  INS_STATE curr_state;
  int latency_left;
  int op_type;
  Reg_number dst;
  Reg_number src1;
  Reg_number src2;

public:
  void initiate();
  void decrease_lat();
};

class ROB_entry {
private:
  Instruction *instruction;
  bool src1_stat;
  bool src2_stat;
  int tag;

public:
};

// out of order executer
class OOOE {

private:
  vector<Instruction> dispatch_list;
  vector<Instruction> issue_list;
  vector<Instruction> execute_list;
  vector<ROB_entry> ROB_table;

public:
  void retire();
  void execute();
  void issue();
  void dispatch();
  void fetch();
  void advance_cycle();
};
