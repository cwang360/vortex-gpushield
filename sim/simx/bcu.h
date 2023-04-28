#pragma once

#include <simobject.h>
#include "pipeline.h"
#include "cache.h"

namespace vortex { 

#define BCUQ_SIZE 16

class Core;

class BcuUnit : public SimObject<BcuUnit> {
private:
    HashTable<std::pair<pipeline_trace_t*, uint32_t>> pending_reqs_;

public:
    SimPort<pipeline_trace_t*> Input;
    SimPort<pipeline_trace_t*> Output;

    BcuUnit(const SimContext& ctx, Core* core, const char* name) 
        : SimObject<BcuUnit>(ctx, name) 
        , pending_reqs_(BCUQ_SIZE)
        , Input(this)
        , Output(this)
        , core_(core)
    {}
    
    virtual ~BcuUnit() {}

    void reset() {}

    void tick();

private:
    Core* core_;
};

}