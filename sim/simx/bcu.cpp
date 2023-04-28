#include "bcu.h"
#include "core.h"

using namespace vortex;

void BcuUnit::tick() {
    // handle rcache response
    auto& rcache_rsp_port = rcache_->BcuRspPort;
    if (!rcache_rsp_port.empty()) {
        auto& mem_rsp = rcache_rsp_port.front();
        auto& entry = pending_reqs_.at(mem_rsp.tag);          
        auto trace = entry.first;
        DT(1, "rcache-rsp: tag=" << mem_rsp.tag << ", type=" << trace->lsu.type 
            << ", " << *trace);  

        auto access_addr = mem_rsp.req_addr;
        auto rbt_entry_base_addr = mem_rsp.rbt_entry->base_addr;
        auto rbt_entry_size = mem_rsp.rbt_entry->size;
        auto evaluation = (rbt_entry_base_addr <= access_addr) && (access_addr < rbt_entry_base_addr + rbt_entry_size);
        if (rbt_entry_base_addr) {
            DT(1, "bcu-evaluation: access_addr=0x" << std::hex << access_addr
                    << " rbt_entry_base_addr=0x" << rbt_entry_base_addr
                    << " rbt_entry_size=0x" << rbt_entry_size
                    << " evaluation=" << (evaluation ? "valid" : "invalid"));
        }
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
    
        
    auto mem_addr = trace->mem_addrs.at(0).at(0);

    if (buffer_id_map_.count(mem_addr.addr)){
        // FIXME: need to properly initialize memory and request for loading RBT entry.
        DT(1, "bcu: valid buffer id found");
        auto tag = pending_reqs_.allocate({trace, 1});
        DT(1, "bcu-req: addr=0x" << std::hex << mem_addr.addr << ", " << tag << ", " << *trace);

        RbtEntryReq mem_req;
        mem_req.addr  = mem_addr.addr;
        mem_req.buffer_id = buffer_id_map_.at(mem_addr.addr);
        mem_req.tag   = tag;
        mem_req.uuid = trace->uuid;
            
        rcache_->BcuReqPort.send(mem_req, 1);
    } else {
        DT(1, "bcu: no buffer id for addr " << std::hex << mem_addr.addr << ", " << *trace);
    }
        
    Input.pop();
}

void RbtCache::reset() {
    cache_set_ = std::deque<RbtEntry*>();
    perf_stats_ = PerfStats();
    // mshr.clear();
    pending_read_reqs_ = 0;
    pending_fill_reqs_ = 0;
}

void RbtCache::tick() {
    // handle mem response
    auto& rbt_mem_rsp_port = core_->rbt_mem_->RcacheRspPort;
    if (!rbt_mem_rsp_port.empty()) {
        auto& mem_rsp = rbt_mem_rsp_port.front();
        DT(1, mem_rsp);  
        
        // eviction if necessary
        if (cache_set_.size() == config_.size) {
            cache_set_.pop_front();
            ++perf_stats_.evictions;
        }

        cache_set_.push_back(mem_rsp.rbt_entry);

        BcuRspPort.send(mem_rsp, config_.latency);
        DT(1, "rcache-sending-rsp: " << mem_rsp);
        rbt_mem_rsp_port.pop();  
    }


    // calculate memory latency
    // perf_stats_.mem_latency += pending_fill_reqs_; 

    // handle incoming BCU requests
    if (BcuReqPort.empty())
        return;

    auto& bcu_req = BcuReqPort.front();
    
    DT(1, "rcache-receiving-req: addr=0x" << std::hex << bcu_req.addr << ", " << bcu_req.tag);

    auto it = cache_set_.begin();
    for (; it != cache_set_.end(); it++) {
        if ((*it)->buffer_id == bcu_req.buffer_id) {
            DT(1, "RBT entry found in rcache");
            RbtEntryRsp bcu_rsp{bcu_req.tag, *it, bcu_req.uuid, bcu_req.addr};
            BcuRspPort.send(bcu_rsp, config_.latency);
            DT(1, "rcache-sending-rsp: addr=0x" << std::hex << bcu_req.addr << ", " << bcu_req.tag);
            break;
        }
    }

    if (it == cache_set_.end()) {
        // miss
        DT(1, "RBT entry does not exist in rcache");
        // miss repair
        core_->rbt_mem_->RcacheReqPort.send(bcu_req, core_->rbt_mem_->latency_);
        ++perf_stats_.read_misses;
    }

    ++perf_stats_.reads;

    // remove request
    auto time = BcuReqPort.pop();
    perf_stats_.pipeline_stalls += (SimPlatform::instance().cycles() - time);
} 

const RbtCache::PerfStats& RbtCache::perf_stats() const {
    return perf_stats_;
}

void RbtMem::tick() {
    // handle incoming BCU requests
    while (!RcacheReqPort.empty()) {
        auto& req = RcacheReqPort.front();
        
        DT(1, "rbt-mem-receiving-req: addr=0x" << std::hex << req.addr << ", " << req.tag << ", buffer_id=" << req.buffer_id);

        if (rbt_.count(req.buffer_id)) {
            RbtEntryRsp bcu_rsp{req.tag, &rbt_.at(req.buffer_id), req.uuid, req.addr};
            RcacheRspPort.send(bcu_rsp, latency_);
            DT(1, "rbt-mem-sending-rsp: addr=0x" << std::hex << req.addr << ", " << req.tag);
        } else {
            DT(1, "rbt-mem: invalid buffer id!");
            RbtEntryRsp bcu_rsp{req.tag, &rbt_.at(0), req.uuid, req.addr};
            RcacheRspPort.send(bcu_rsp, latency_);
        }


        // remove request
        RcacheReqPort.pop();
    }


}