#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <set>

#include "memory.h"
#include "tickable.h"
#include "nodeimpl.h"
#include "regimpl.h"
#include "analysis.h"

using namespace chdl;
using namespace std;

unsigned long toUint(vector<node> &v) {
  unsigned long r(0);
  for (unsigned i = 0; i < v.size(); ++i)
    if (nodes[v[i]]->eval()) r |= 1<<i;
  return r;
}

struct memory;

// Set of all currently-existing memory objects.
vector<memory *> memories;

struct memory : public tickable {
  memory(vector<node> &qa, vector<node> &d, vector<node> &da, node w,
         string filename, bool sync, size_t &id);
  ~memory() { cout << "Destructing a memory." << endl; }

  vector<node> add_read_port(vector<node> &qa);

  void tick();
  void tock();

  void print(ostream &out);
  void print_vl(ostream &out);

  void print_c_decl(ostream &out);
  void print_c_impl(ostream &out);

  node w;
  vector<node> da, d;
  vector<vector<node>> qa, q;

  bool do_write;
  vector<bool> wrdata;

  size_t waddr;
  vector<size_t> raddr;
  vector<bool> contents;
  vector<vector<bool>> rdval;
  string filename;

  bool sync;
};

void memory::tick() {
  do_write = nodes[w]->eval();
  if (sync) for (unsigned i = 0; i < qa.size(); ++i) raddr[i] = toUint(qa[i]);
  waddr = toUint(da);

  if (do_write)
    for (unsigned i = 0; i < d.size(); ++i)
      wrdata[i] = nodes[d[i]]->eval();
}

void memory::tock() {
  if (sync)
    for (unsigned i = 0; i < q.size(); ++i)
      for (unsigned j = 0; j < d.size(); ++j)
        rdval[i][j] = contents[raddr[i]*d.size() + j];

  if (do_write)
    for (unsigned i = 0; i < d.size(); ++i)
      contents[waddr*d.size() + i] = wrdata[i];
}

void memory::print(ostream &out) {
  out << "  " << (sync?"sync":"") << "ram <" << qa[0].size() << ' ' << d.size();
  if (filename != "") out << " \"" << filename << '"';
  out << '>';
  for (unsigned i = 0; i < da.size(); ++i) out << ' ' << da[i];
  for (unsigned i = 0; i < d.size(); ++i) out << ' ' << d[i];
  out << ' ' << w;
  for (unsigned j = 0; j < qa.size(); ++j) {
    for (unsigned i = 0; i < qa[0].size(); ++i) out << ' ' << qa[j][i];
    for (unsigned i = 0; i < q[0].size(); ++i) out << ' ' << q[j][i];
  }
  out << endl;
}

void memory::print_vl(ostream &out) {
  nodeid_t id(q[0][0]);

  if (!sync) {
    cerr << "Async RAM not currently supported in verilog. Use llmem." << endl;
    exit(1);
  }

  size_t words(1ul<<da.size()), bits(d.size());
  for (unsigned i = 0; i < qa.size(); ++i) {
    out << "  wire [" << qa[0].size()-1 << ":0] __mem_qa" << id << '_' << i
        << ';' << endl
        << "  reg [" << bits-1 << ":0] __mem_q" << id << '_' << i <<  ';'
        << endl;
  }

  out << "  wire [" << da.size()-1 << ":0] __mem_da" << id << ';' << endl
      << "  wire [" << bits-1 << ":0] __mem_d" << id << ';' << endl
      << "  wire __mem_w" << id << ';' << endl
      << "  reg [" << bits-1 << ":0] __mem_array" << id
      << '[' << words-1 << ":0];" << endl
      << "  always @(posedge phi)" << endl
      << "    begin" << endl;
  for (unsigned i = 0; i < q.size(); ++i) {
    out << "      __mem_q" << id << '_' << i
        << " <= __mem_array" << id << "[__mem_qa" << id << '_' << i << "];"
        << endl;
  }

  out << "      if (__mem_w" << id << ") __mem_array" << id
      << "[__mem_da" << id << "] <= __mem_d" << id << ';' << endl
      << "  end" << endl;
  
  out << "  assign __mem_w" << id << " = __x" << w << ';' << endl;
  for (unsigned j = 0; j < qa.size(); ++j) {
    for (unsigned i = 0; i < qa[0].size(); ++i)
      out << "  assign __mem_qa" << id << '_' << j << '[' << i << "] = __x"
          << qa[j][i] << ';' << endl;
  }

  for (unsigned i = 0; i < da.size(); ++i)
    out << "  assign __mem_da" << id << '[' << i << "] = __x" << da[i] << ';'
        << endl;
  for (unsigned j = 0; j < q.size(); ++j) {
    for (unsigned i = 0; i < q[j].size(); ++i) {
      out << "  assign __x" << q[j][i] << " = __mem_q" << id << '_' << j
          << '[' << i << "];" << endl;
    }
  }
  for (unsigned i = 0; i < d.size(); ++i)
    out << "  assign __mem_d" << id << '[' << i << "] = __x" << d[i] << ';'
        << endl;
}

void memory::print_c_decl(ostream &out) {
  size_t sz(1ul<<qa.size());
  nodeid_t id(q[0][0]);

  for (unsigned i = 0; i < d.size(); ++i) {
    out << "  char mem" << id << '_' << i << '[' << sz << "];\n";
    out << "  { size_t i; for (i = 0; i < " << sz << "; i++) "
           "mem" << id << '_' << i << "[i] = 0; }\n";
  }
}

void memory::print_c_impl(ostream &out) {
  nodeid_t id(q[0][0]);

  out << "  size_t mem" << id << "_da(";
  for (unsigned i = 0; i < da.size(); ++i) {
    out << "((";
    nodes[da[i]]->print_c_val(out);
    out << ")<<" << i << ')';
    if (i != da.size()-1) out << '|';
  }
  out << ')';

  out << "  if (";
  nodes[w]->print_c_val(out);
  out << ") {";
  for (unsigned i = 0; i < d.size(); ++i) {
    out << "    mem" << id << '_' << i << "[mem" << id << "_da] = ";
    nodes[d[i]]->print_c_val(out);
    out << ";\n";
  }
}

struct qnodeimpl : public nodeimpl {
  qnodeimpl(memory *mem, unsigned port, unsigned idx):
    mem(mem), port(port), idx(idx) {}

  bool eval() {
    if (mem->sync)
      return mem->rdval[port][idx];
    else
      return mem->contents[toUint(mem->qa[port])*mem->d.size() + idx];
  }

  void print(ostream &out) { if (port == 0 && idx == 0) mem->print(out); }
  void print_vl(ostream &out) { if (port == 0 && idx == 0) mem->print_vl(out); }
  void print_c_decl(ostream &out) {
    if (port == 0 && idx == 0) mem->print_c_decl(out);
  }
  void print_c_val(ostream &out);

  unsigned port, idx;

  memory *mem;
};

void qnodeimpl::print_c_val(ostream &out) {
  out << "mem" << id << '_' << idx << '[';
  for (unsigned i = 0; i < mem->qa[port].size(); ++i) {
    out << "((";
    nodes[mem->qa[port][i]]->print_c_val(out);
    out << ")<<" << idx << ')';
    if (i != mem->qa[port].size()-1) out << '|';
  }
  out << ']';
}

// Load a hex file.
void load_contents(unsigned n, vector<bool> &contents, string filename) {
  ifstream in(filename.c_str());
  size_t i = 0;
  while (in) {
    unsigned long long val;
    in >> hex >> val;
    for (unsigned j = 0; j < n; ++j) {
      if (i*n + j < contents.size()) contents[i*n + j] = val & 1;
      val >>= 1;
    }
    ++i;
  }
 
  for (size_t j = i*n; j < contents.size(); ++j) contents[j] = i%2;
}

memory::memory(
  vector<node> &qai, vector<node> &di, vector<node> &dai, node w,
  string filename, bool sync, size_t &id
) :
  contents(di.size()<<(qai.size())), wrdata(di.size()-1), filename(filename),
  w(w), d(di.size()), da(qai.size()), raddr(0), waddr(0),
  sync(sync)
{
  // Load contents from file
  if (filename != "") load_contents(d.size(), contents, filename);

  // Add the read port
  add_read_port(qai);

  // Populate the write address array.
  rdval.push_back(vector<bool>(di.size()));
  for (unsigned i = 0; i < qai.size(); ++i)
    da[i] = dai[i];

  // Create the q bits.
  for (unsigned i = 0; i < d.size(); ++i)
    d[i] = di[i];

  id = memories.size();
  memories.push_back(this);
}

// The function used by memory.h, and the simplest of all. Just create a new
// memory with the given inputs and return the outputs.
namespace chdl {
  vector<node> memory_internal(
    vector<node> &qa, vector<node> &d, vector<node> &da, node w,
    string filename, bool sync, size_t &id
  )
  {
    memory *m = new memory(qa, d, da, w, filename, sync, id);
    return m->q[0];
  }

  vector<node> memory_add_read_port(size_t id, vector<node> &qa) {
    return memories[id]->add_read_port(qa);
  }
};

vector<node> memory::add_read_port(vector<node> &qai) {
  size_t idx(q.size());

  qa.push_back(qai);
  raddr.push_back(0);
  rdval.push_back(vector<bool>(d.size()));

  q.push_back(vector<node>());
  for (unsigned i = 0; i < d.size(); ++i)
    q[idx].push_back((new qnodeimpl(this, idx, i))->id);

  return q[idx];
}

void chdl::get_mem_nodes(set<nodeid_t> &s) {
  for (auto mp : memories) {
    memory &m(*mp);
    for (unsigned j = 0; j < m.qa.size(); ++j)
      for (unsigned i = 0; i < m.qa[j].size(); ++i) s.insert(m.qa[j][i]);
    for (unsigned i = 0; i < m.da.size(); ++i) s.insert(m.da[i]);
    for (unsigned i = 0; i < m.d.size(); ++i)  s.insert(m.d[i]);
    s.insert(m.w);
  }
}

size_t chdl::num_sram_bits() {
  size_t count(0);
  for (auto m : memories)
    count += m->contents.size();
  return count;
}
