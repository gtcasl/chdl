// Node implementation.
#ifndef __NODEIMPL_H
#define __NODEIMPL_H

#include <vector>
#include <iostream>

#include "node.h"
#include "hierarchy.h"
#include "cdomain.h"

namespace chdl {
  struct nodeimpl;

  extern std::vector<nodeimpl*> nodes;

  struct nodeimpl {
    nodeimpl() { id = nodes.size(); nodes.push_back(this); path = get_hpath(); }
    virtual ~nodeimpl() {}

    virtual bool eval(cdomain_handle_t) = 0;
    virtual void print(std::ostream &) = 0;
    virtual void print_vl(std::ostream &) = 0;

    // The node keeps a copy of its ID. This makes mapping from nodeimpl
    // pointers to node objects straightforward.
    nodeid_t id;

    // Where is this node in the hierarchy?
    hpath_t path;

    std::vector<node> src;
  };
};

#endif
