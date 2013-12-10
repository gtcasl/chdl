#ifndef __CHDL_EGRESS_H
#define __CHDL_EGRESS_H
#include <chdl/chdl.h>
#include <chdl/nodeimpl.h>
#include <chdl/tickable.h>

#include <vector>
#include <functional>

namespace chdl {
  template <typename T> void EgressFunc(const T& f, node n);
  static void Egress(bool &b, node n);
  template <typename T> void EgressInt(T &i, const std::vector<node> &v);
  template <typename T, unsigned N> void EgressInt(T &i, bvec<N> bv);

  template <typename T> class egress : public tickable {
  public:
    egress(const T& x, node n): n(n), x(x) { gtap(n); }

    void tick() { x(nodes[n]->eval()); }
    void tock() {}
  private:
    node n;
    T x;
  };

  void print_tmat(std::ostream &out);
}

template <typename T> void chdl::EgressFunc(const T &f, chdl::node n) {
  new chdl::egress<T>(f, n);
}

static void chdl::Egress(bool &b, chdl::node n) {
  chdl::EgressFunc([&b](bool val){ b = val; }, n);
}

template <typename T, unsigned N>
  void chdl::EgressInt(T &x, chdl::bvec<N> bv)
{
  using namespace std;
  using namespace chdl;

  vector<node> v;
  for (unsigned i = 0; i < N; ++i) v.push_back(bv[i]);
  EgressInt(x, v);
}

template <typename T>
  void chdl::EgressInt(T &x, const std::vector<node> &v)
{
  using namespace chdl;
  using namespace std;

  x = 0;

  for (unsigned i = 0; i < v.size(); ++i)
    chdl::EgressFunc(
      [i, &x](bool val){
        if (val) x |= (1ull<<i); else x &= ~(1ull<<i);
      },
      v[i]
    );
}
#endif
