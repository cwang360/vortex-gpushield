#include "bcu.h"
#include "core.h"

using namespace vortex;

void BcuUnit::tick() {
    // handle rcache response
    auto& rcache_rsp_port = core_->rcache_switch_->RspOut.at(0);
    if (!rcache_rsp_port.empty()) {
        auto& mem_rsp = rcache_rsp_port.front();
        auto& entry = pending_reqs_.at(mem_rsp.tag);          
        auto trace = entry.first;
        DT(1, "rcache-rsp: tag=" << mem_rsp.tag << ", type=" << trace->lsu.type 
            << ", " << *trace);  
        assert(entry.second);
        --entry.second; // track remaining blocks 
        if (0 == entry.second) {
            Output.send(trace, 1);
            pending_reqs_.release(mem_rsp.tag);
        } 
        rcache_rsp_port.pop();  
    }


    if (Input.empty())
        return;

    auto trace = Input.front();
    if (trace->mem_addrs.size() == 0)
        return;
    
        
    auto& rcache_req_port = core_->rcache_switch_->ReqIn.at(0);

    auto mem_addr = trace->mem_addrs.at(0).at(0);
    auto tag = pending_reqs_.allocate({trace, 1});


    DT(1, "bcu-req: " << std::hex << mem_addr.addr << ", " << tag << ", " << *trace);


    // FIXME: need to properly initialize memory and request for loading RBT entry.
    MemReq mem_req;
    mem_req.addr  = mem_addr.addr;
    mem_req.write = false;
    mem_req.non_cacheable = false; 
    mem_req.tag   = tag;
    mem_req.core_id = trace->cid;
    mem_req.uuid = trace->uuid;
         
    // temporarily use this, may have to make a custom cache later.
    rcache_req_port.send(mem_req, 1);
        
    auto time = Input.pop();
}

void RbtCache::reset() {
    cache_set_ = std::queue<RbtCacheEntry>();
    perf_stats_ = PerfStats();
    // mshr.clear();
    pending_read_reqs_ = 0;
    pending_write_reqs_ = 0;
    pending_fill_reqs_ = 0;
}

void RbtCache::tick() {

    // calculate memory latency
    perf_stats_.mem_latency += pending_fill_reqs_; 

    // handle incoming core requests
    for (uint32_t req_id = 0, n = config_.num_inputs; req_id < n; ++req_id) {
        auto& core_req_port = CoreReqPorts.at(req_id);            
        if (core_req_port.empty())
            continue;

        auto& core_req = core_req_port.front();
        
        DT(1, "rcache-receiving-req: addr=" << std::hex << core_req.addr << ", " << core_req.tag);

        MemRsp core_rsp{core_req.tag, core_req.core_id, core_req.uuid};
        CoreRspPorts.at(req_id).send(core_rsp, config_.latency);
        DT(1, "rcache-sending-rsp: addr=" << std::hex << core_req.addr << ", " << core_req.tag);

        ++perf_stats_.reads;

        // remove request
        auto time = core_req_port.pop();
        perf_stats_.pipeline_stalls += (SimPlatform::instance().cycles() - time);
    }

    // process active request        
    // this->processBankRequest(pipeline_reqs);
} 

const RbtCache::PerfStats& RbtCache::perf_stats() const {
    return perf_stats_;
}
