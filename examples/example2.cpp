#include <iostream>
#include <fstream>

#include <gateops.h>
#include <bvec-basic-op.h>
#include <netlist.h>

#include <adder.h>
#include <divider.h>
#include <mult.h>

#include <tap.h>
#include <sim.h>
#include <opt.h>
#include <vis.h>

#include <techmap.h>

#include <analysis.h>

#include <tmat.h>

#include "report.h"

using namespace std;
using namespace chdl;

int main(int argc, char **argv) {
  #if 0
  // The design
  bvec<4> a, b, c, d, ax;
  bvec<8> v, v2;
  bvec<4> w, x, y, z;
  bvec<16> o;

  a = Reg(a + Lit<4>(1), 4);
  b = Reg(b + Lit<4>(3), 8);
  c = Reg(c + Lit<4>(5), 7);
  d = Reg(d + Lit<4>(7), 5);

  o[range< 0, 3>()] = a;
  o[range< 4, 7>()] = b;
  o[range< 8,11>()] = c;
  o[range<12,15>()] = d;


  v = Zext<8>(a) * Zext<8>(b);
  w = a & b & c & d;
  x = a ^ b;
  y = a | b | c | d;
  z = a + b;
  ax = Zext<4>(divider(v, Zext<8>(b)));

  node a_b_eq(a == b);

  TAP(a); TAP(b); TAP(c); TAP(d);
  TAP(ax);
  TAP(v); TAP(w); TAP(x); TAP(y); TAP(z);
  TAP(o);
  TAP(a_b_eq);
  #endif

  bvec<32> next_a, a(Reg(next_a)), next_b, b(Reg(next_b));
  next_a = a + Lit<32>(0xfedcba98 + 7);
  next_b = b + Lit<32>(0x76543211 + 5);

  bvec<32> p(a * b);

  TAP(p); TAP(a); TAP(b);

  // The simulation (generate .vcd file)
  optimize();

  ofstream netl_file("example2.netl");
  techmap(netl_file);
  netl_file.close();

  ofstream wave_file("example2.vcd");
  run(wave_file, 100000);

  ofstream netlist_file("example2.nand");
  print_netlist(netlist_file);
  netlist_file.close();

  ofstream dot_file("example2.dot");
  print_dot(dot_file);
  dot_file.close();

  cout << "Critical path length: " << critpath() << endl;

  // Print transition matrices:
  ofstream mat("example2.tmat");
  tmat_report(mat);

  ifstream elib("ELIB");
  while (!!elib) {
    string type;
    elib >> type;
    vector<double> emat;
    //cout << "Reading for " << type << endl;
    do {
      double d;
      elib >> d;
      emat.push_back(d);
    } while (!!elib && elib.peek() != '\n');
    if (!!elib) tmat_compute(type, emat);
  }

  report();

  return 0;
}
