#pragma once

#include <simobject.h>
#include "pipeline.h"
#include "cache.h"

namespace vortex { 

#define BCUQ_SIZE 16

class Core;

class RbtCache : public SimObject<RbtCache> {
public:
    struct Config {
        uint64_t size;              // log2 cache size
        uint8_t latency;
    };
    
    struct PerfStats {
        uint64_t reads;
        uint64_t read_misses;
        uint64_t evictions;
        uint64_t pipeline_stalls;
        uint64_t mshr_stalls;
        uint64_t mem_latency;

        PerfStats() 
            : reads(0)
            , read_misses(0)
            , evictions(0)
            , pipeline_stalls(0)
            , mshr_stalls(0)
            , mem_latency(0)
        {}
    };

    struct RbtCacheEntry {
        uint32_t buffer_id;
        uint64_t base_addr;
        uint64_t size;
        bool read_only;
        uint64_t kernel_id;
    };

    SimPort<MemReq>     BcuReqPort;
    SimPort<MemRsp>     BcuRspPort;
    SimPort<MemReq>     MemReqPort;
    SimPort<MemRsp>     MemRspPort;
    // MSHR                         mshr;

    RbtCache(const SimContext& ctx, const char* name, const Config& config) 
        : SimObject<RbtCache>(ctx, name)    
        , BcuReqPort(this)
        , BcuRspPort(this)
        , MemReqPort(this)
        , MemRspPort(this)
        , config_(config)
        // , mshr(config.mshr_size)
    {}

    void reset();
    
    void tick();

    const PerfStats& perf_stats() const;
    
private:
    Config config_;
    std::deque<RbtCacheEntry> cache_set_;
    PerfStats perf_stats_;
    uint64_t pending_read_reqs_;
    uint64_t pending_write_reqs_;
    uint64_t pending_fill_reqs_;    

};

class BcuUnit : public SimObject<BcuUnit> {
private:
    HashTable<std::pair<pipeline_trace_t*, uint32_t>> pending_reqs_;
    RbtCache::Ptr rcache_;

public:
    SimPort<pipeline_trace_t*> Input;
    SimPort<pipeline_trace_t*> Output;

    BcuUnit(const SimContext& ctx, Core* core, const char* name) 
        : SimObject<BcuUnit>(ctx, name) 
        , pending_reqs_(BCUQ_SIZE)
        , rcache_(RbtCache::Create("rcache", RbtCache::Config{
            RCACHE_SIZE,
            2
        }))
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


