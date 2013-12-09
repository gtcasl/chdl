#ifndef CHDL_TMAT_H
#define CHDL_TMAT_H
#include <iostream>

#include "tickable.h"
#include "egress.h"

namespace chdl {
  class dumpable : public tickable {
    dumpable();
    virtual ~dumpable() {}
  public:
    virtual void dump(std::ostream &os) = 0;
  };

  template<unsigned N> class tmat : public dumpable {
  public:
    tmat(const bvec<N> &inputs);
    virtual ~tmat() {}

    void dump(std::ostream &out);

    void tick();
    void tock();

  private:
    unsigned prev_egint, egint;
    unsigned long long tcount[1<<N][1<<N];
  };

  template <unsigned N> void add_tmat(const bvec<N> &inputs);
  void tmat_report(std::ostream &out);
}

template <unsigned N> void chdl::add_tmat(const chdl::bvec<N> &inputs) {
  new chdl::tmat<N>(inputs);
}

template <unsigned N> chdl::tmat<N>::tmat(const chdl::bvec<N> &inputs):
  prev_egint(0), egint(0), chdl::dumpable()
{
  using namespace chdl;

  egressInt(egint, inputs);
  for (unsigned i = 0; i < (1<<N); ++i)
    for (unsigned j = 0; j < (1<<N); ++j)
      tcount[i][j] = 0;
}

template <unsigned N> void chdl::tmat<N>::tick() {}

template <unsigned N> void chdl::tmat<N>::tock() {
  ++tcount[prev_egint][egint];
  prev_egint = egint;
}


template <unsigned N> void chdl::tmat<N>::dump(std::ostream &out) {
  using namespace std;

  for (unsigned i = 0; i < (1<<N); ++i)
   for (unsigned j = 0; j < (1<<N); ++j)
     out << ' ' << tcount[i][j];

  out << endl; 
}



#endif
