#include "bcu.h"
#include "core.h"

using namespace vortex;

void BcuUnit::tick() {
    if (Input.empty())
        return;

    auto trace = Input.front();
    if (trace->mem_addrs.size() == 0)
        return;
    
        
    auto& rcache_req_port = core_->rcache_switch_->ReqIn.at(0);

    auto mem_addr = trace->mem_addrs.at(0).at(0);

    DT(1, "bcu-req: " << std::hex << mem_addr.addr << ", " << *trace);

    // FIXME: need to properly initialize memory and request for loading RBT entry.
    MemReq mem_req;
    mem_req.addr  = mem_addr.addr;
    mem_req.write = false;
    mem_req.non_cacheable = false; 
    mem_req.tag   = 0;
    mem_req.core_id = trace->cid;
    mem_req.uuid = trace->uuid;
         
    // temporarily use this, may have to make a custom cache later.
    rcache_req_port.send(mem_req, 1);
        
    //for (auto& mem_addr : trace->mem_addrs) {
    //    uint64_t addr = mem_addr.at(0).addr;
    //    DT(1, "bcu-req: addr=" << std::hex << addr << ", " << *trace);
    //} 
    auto time = Input.pop();
}
