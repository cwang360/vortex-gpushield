#pragma once

#include <simobject.h>
#include <map>
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
        uint64_t lower_level_latency;
        bool lru; // lru if true, fifo if false
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


    SimPort<RbtEntryReq>     BcuReqPort;
    SimPort<RbtEntryRsp>     BcuRspPort;
    SimPort<RbtEntryReq>*     MemReqPort;
    SimPort<RbtEntryRsp>*     MemRspPort;
    // MSHR                         mshr;

    RbtCache(const SimContext& ctx, Core* core, const char* name, std::string print_name, const Config& config) 
        : SimObject<RbtCache>(ctx, name)    
        , BcuReqPort(this)
        , BcuRspPort(this)
        // , MemReqPort(this)
        // , MemRspPort(this)
        , config_(config)
        , core_(core)
        , print_name(print_name)
        // , mshr(config.mshr_size)
    {}

    void reset();
    
    void tick();

    const PerfStats& perf_stats() const;
    
private:
    Config config_;
    Core* core_;
    std::string print_name;
    std::list<RbtEntry*> cache_set_;
    PerfStats perf_stats_;
    uint64_t pending_read_reqs_;
    uint64_t pending_fill_reqs_;  
};

class BcuUnit : public SimObject<BcuUnit> {
private:
    HashTable<std::pair<pipeline_trace_t*, uint32_t>> pending_reqs_;
    RbtCache::Ptr l1_rcache_;
    RbtCache::Ptr l2_rcache_;

public:
    SimPort<pipeline_trace_t*> Input;
    SimPort<pipeline_trace_t*> Output;

    BcuUnit(const SimContext& ctx, Core* core, const char* name);
    
    virtual ~BcuUnit() {}

    void reset() {}

    void tick();

private:
    Core* core_;

    // map of address to buffer id for testing
    std::map<uint64_t, uint16_t> buffer_id_map_ = {
        {0xfeffeffc, 0},
        {0xfeffdffc, 0},

        {0xFEFFFFC8, 1},
        {0xFEFFFFBC, 2},
        {0x8000A820, 3},
        {0x8000BA84, 4},
    };
};

class RbtMem : public SimObject<RbtMem> {
private:
    // map of buffer id to RBT entry  
    
    // RBT layout using program output
    // The base addresses might not be deterministic, run make to double check them
    // Buffer ID (14 bits) | Base Address (48 bits) | Size (32 bits) | Read-only (1 bit) | Kernel ID (12 bit)
    std::map<uint64_t, RbtEntry> rbt_ = {
        {0x0, {0x0, 0x00000000, 0x00, 0x0, 0x0}}, // invalid
        {0x1, {0x1, 0xFEFFFFC8, 0x28, 0x0, 0x0}},
        {0x2, {0x2, 0xFEFFFFBC, 0x0A, 0x0, 0x0}},
        {0x3, {0x3, 0x8000A820, 0x0D, 0x1, 0x0}},
        {0x4, {0x4, 0x8000BA84, 0xC8, 0x0, 0x0}}
    };

public:
    SimPort<RbtEntryReq>     RcacheReqPort;
    SimPort<RbtEntryRsp>     RcacheRspPort;

    RbtMem(const SimContext& ctx, const char* name, uint64_t latency) 
        : SimObject<RbtMem>(ctx, name) 
        , RcacheReqPort(this)
        , RcacheRspPort(this)
        , latency_(latency)
    {}
    
    void tick();

    void reset(){}

private:
    uint64_t latency_;

friend class RbtCache;
friend class BcuUnit;
};


}


