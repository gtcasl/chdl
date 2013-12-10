#include <vector>
#include <map>
#include <string>

#include "tmat.h"
#include "egress.h"

using namespace std;
using namespace chdl;

static multimap<string, tmat *> tmats;

chdl::tmat::tmat(const string &type, const vector<node> &inputs):
  prev_egint(0), egint(0), n(inputs.size()), tcount(1<<(2*n)), chdl::tickable()
{
  using namespace chdl;

  EgressInt(egint, inputs);
  for (unsigned i = 0; i < (1<<n); ++i)
    for (unsigned j = 0; j < (1<<n); ++j)
      tcount[(i<<n) | j] = 0;

  tmats.insert(make_pair(type, this));
}

void chdl::tmat_report(ostream &out) {
  for (auto &d : tmats) {
    out << d.first << endl;
    d.second->dump(out);
  }
}

void chdl::add_tmat(const string &type, const vector<node> &inputs) {
  new chdl::tmat(type, inputs);
}

void chdl::tmat::tick() {}

void chdl::tmat::tock() {
  ++tcount[(prev_egint<<n) | egint];
  prev_egint = egint;
}

void chdl::tmat::dump(std::ostream &out) {
  using namespace std;

  for (unsigned i = 0; i < (1<<n); ++i) {
   for (unsigned j = 0; j < (1<<n); ++j)
     out << ' ' << tcount[(i<<n) | j];
   out << endl;
  }

  out << endl; 
}

template <typename A, typename B>
  double dotprod(const vector<A> &a, const vector<B> &b)
{
  double sum(0);
  for (unsigned i = 0; i < a.size(); ++i)
    sum += a[i] * b[i];
  return sum;
}

void chdl::tmat_compute(const string &name, const vector<double> &emat) {
  cout << name << ":";
  bool first(true);
  for (auto it = tmats.find(name); it != tmats.end() && it->first == name; ++it)
  {
    if (!first) cout << ',';
    else first = false;
    cout << dotprod(emat, it->second->tcount);
  }
  cout << endl;
}
