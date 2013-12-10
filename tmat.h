#ifndef CHDL_TMAT_H
#define CHDL_TMAT_H
#include <iostream>
#include <string>

#include "tickable.h"
#include "egress.h"

namespace chdl {
  class tmat : public tickable {
  public:
    tmat(const std::string &type, const std::vector<node> &inputs);
    virtual ~tmat() {}

    void dump(std::ostream &out);

    void tick();
    void tock();

  private:
    unsigned prev_egint, egint, n;
    std::vector<unsigned long long> tcount;
  };

  void add_tmat(const std::string &type, const std::vector<node> &inputs);
  void tmat_report(std::ostream &out);
}
#endif
