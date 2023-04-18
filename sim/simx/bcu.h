#pragma once

#include <simobject.h>
#include "pipeline.h"
#include "cache.h"

namespace vortex { 

class Core;

class BcuUnit : public SimObject<BcuUnit> {
public:
    SimPort<pipeline_trace_t*> Input;
    SimPort<pipeline_trace_t*> Output;

    BcuUnit(const SimContext& ctx, Core* core, const char* name) 
        : SimObject<BcuUnit>(ctx, name) 
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