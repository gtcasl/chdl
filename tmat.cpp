#include <vector>

#include "tmat.h"
#include "egress.h"

using namespace std;
using namespace chdl;

static vector<tmat *> tmats;

chdl::tmat::tmat(const vector<node> &inputs):
  prev_egint(0), egint(0), n(inputs.size()), tcount(1<<(2*n)), chdl::tickable()
{
  using namespace chdl;

  EgressInt(egint, inputs);
  for (unsigned i = 0; i < (1<<n); ++i)
    for (unsigned j = 0; j < (1<<n); ++j)
      tcount[(i<<n) | j] = 0;

  tmats.push_back(this);
}

void chdl::tmat_report(ostream &out) {
  for (auto d : tmats) d->dump(out);
}

void chdl::add_tmat(const vector<node> &inputs) {
  new chdl::tmat(inputs);
}

void chdl::tmat::tick() {}

void chdl::tmat::tock() {
  ++tcount[(prev_egint<<n) | egint];
  prev_egint = egint;
}

void chdl::tmat::dump(std::ostream &out) {
  using namespace std;

  for (unsigned i = 0; i < (1<<n); ++i)
   for (unsigned j = 0; j < (1<<n); ++j)
     out << ' ' << tcount[(i<<n) | j];

  out << endl; 
}
