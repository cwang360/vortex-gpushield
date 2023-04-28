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
        uint8_t C;              // log2 cache size
        uint8_t B;              // log2 block size
        uint8_t W;              // log2 word size
        uint8_t A;              // log2 associativity
        uint8_t addr_width;     // word address bits
        uint8_t num_banks;      // number of banks
        uint8_t ports_per_bank; // number of ports per bank
        uint8_t num_inputs;     // number of inputs
        bool    write_through;  // is write-through
        bool    write_reponse;  // enable write response
        uint16_t victim_size;   // victim cache size
        uint16_t mshr_size;     // MSHR buffer size
        uint8_t latency;        // pipeline latency
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
        , mem_req_ports_(config.num_banks, this)
        , mem_rsp_ports_(config.num_banks, this)
        // , mshr(config.mshr_size)
    {}

    void reset();
    
    void tick();

    const PerfStats& perf_stats() const;
    
private:
    Config config_;
    std::queue<RbtCacheEntry> cache_set_;
    Switch<MemReq, MemRsp>::Ptr mem_switch_;    
    Switch<MemReq, MemRsp>::Ptr bypass_switch_;
    std::vector<SimPort<MemReq>> mem_req_ports_;
    std::vector<SimPort<MemRsp>>  mem_rsp_ports_;
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
            log2ceil(RCACHE_SIZE),  // C
            log2ceil(L1_BLOCK_SIZE),// B
            2,                      // W
            0,                      // A
            32,                     // address bits    
            1,                      // number of banks
            1,                      // number of ports
            1,                      // request size   
            true,                   // write-through
            false,                  // write response
            0,                      // victim size
            RCACHE_MSHR_SIZE,       // mshr
            2,                      // pipeline latency
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


