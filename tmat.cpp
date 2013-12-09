#include <vector>

#include "tmat.h"

using namespace std;
using namespace chdl;

static vector<dumpable *> dumpables;

chdl::dumpable::dumpable(): tickable() {
  dumpables.push_back(this);
}

void chdl::tmat_report(ostream &out) {
  for (auto d : dumpables) d->dump(out);
}
