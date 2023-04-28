#pragma once

#include <stdint.h>
#include <bitset>
#include <queue>
#include <unordered_map>
#include <util.h>
#include <VX_config.h>
#include <simobject.h>

namespace vortex {

typedef uint8_t Byte;
#if XLEN == 32
typedef uint32_t Word;
typedef int32_t  WordI;
typedef uint64_t DWord;
typedef int64_t  DWordI;
#elif XLEN == 64
typedef uint64_t Word;
typedef int64_t  WordI;
typedef __uint128_t DWord;
typedef __int128_t DWordI;
#else
#error unsupported XLEN
#endif

typedef uint64_t FWord;

typedef std::bitset<32> RegMask;
typedef std::bitset<32> ThreadMask;
typedef std::bitset<32> WarpMask;

///////////////////////////////////////////////////////////////////////////////

enum class RegType {
  None,
  Integer,
  Float,
  Vector
};

inline std::ostream &operator<<(std::ostream &os, const RegType& clss) {
  switch (clss) {
  case RegType::None: break;
  case RegType::Integer: os << "x"; break;  
  case RegType::Float:   os << "f"; break;
  case RegType::Vector:  os << "v"; break;
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class ExeType {
  NOP,
  ALU,
  LSU,
  CSR,
  FPU,
  GPU,
  MAX,
};

inline std::ostream &operator<<(std::ostream &os, const ExeType& type) {
  switch (type) {
  case ExeType::NOP: os << "NOP"; break;
  case ExeType::ALU: os << "ALU"; break;
  case ExeType::LSU: os << "LSU"; break;
  case ExeType::CSR: os << "CSR"; break;
  case ExeType::FPU: os << "FPU"; break;
  case ExeType::GPU: os << "GPU"; break;
  case ExeType::MAX: break;
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class AluType {
  ARITH,
  BRANCH,
  SYSCALL,
  IMUL,
  IDIV,    
  CMOV,
};

inline std::ostream &operator<<(std::ostream &os, const AluType& type) {
  switch (type) {
  case AluType::ARITH:   os << "ARITH"; break;
  case AluType::BRANCH:  os << "BRANCH"; break;
  case AluType::SYSCALL: os << "SYSCALL"; break;
  case AluType::IMUL:    os << "IMUL"; break;
  case AluType::IDIV:    os << "IDIV"; break;
  case AluType::CMOV:    os << "CMOV"; break;
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class LsuType {
  LOAD,
  STORE,
  FENCE,
  PREFETCH,    
};

inline std::ostream &operator<<(std::ostream &os, const LsuType& type) {
  switch (type) {
  case LsuType::LOAD:     os << "LOAD"; break;
  case LsuType::STORE:    os << "STORE"; break;
  case LsuType::FENCE:    os << "FENCE"; break;
  case LsuType::PREFETCH: os << "PREFETCH"; break;
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class AddrType {
  Global,
  Shared,
  IO,
};

inline std::ostream &operator<<(std::ostream &os, const AddrType& type) {
  switch (type) {
  case AddrType::Global: os << "Global"; break;
  case AddrType::Shared: os << "Shared"; break;
  case AddrType::IO:     os << "IO"; break;
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

struct mem_addr_size_t {
  uint64_t addr;
  uint32_t size;
};

inline AddrType get_addr_type(Word addr, uint32_t size) {
  __unused (size);
  if (SM_ENABLE) {
    if (addr >= (SMEM_BASE_ADDR - SMEM_SIZE)
    &&  addr < SMEM_BASE_ADDR) {      
      assert((addr + size) <= SMEM_BASE_ADDR);
      return AddrType::Shared;
    }
  }
  if (addr >= IO_BASE_ADDR) {
     return AddrType::IO;
  }
  return AddrType::Global;
}

///////////////////////////////////////////////////////////////////////////////

enum class FpuType {
  FNCP,
  FMA,
  FDIV,
  FSQRT,
  FCVT,
};

inline std::ostream &operator<<(std::ostream &os, const FpuType& type) {
  switch (type) {
  case FpuType::FNCP:  os << "FNCP"; break;
  case FpuType::FMA:   os << "FMA"; break;
  case FpuType::FDIV:  os << "FDIV"; break;
  case FpuType::FSQRT: os << "FSQRT"; break;
  case FpuType::FCVT:  os << "FCVT"; break;
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class GpuType {
  TMC,
  WSPAWN,
  SPLIT,
  JOIN,
  BAR,
  TEX,
};

inline std::ostream &operator<<(std::ostream &os, const GpuType& type) {
  switch (type) {
  case GpuType::TMC:    os << "TMC"; break;
  case GpuType::WSPAWN: os << "WSPAWN"; break;
  case GpuType::SPLIT:  os << "SPLIT"; break;
  case GpuType::JOIN:   os << "JOIN"; break;
  case GpuType::BAR:    os << "BAR"; break;
  case GpuType::TEX:    os << "TEX"; break;
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

enum class ArbiterType {
  Priority,
  RoundRobin
};

inline std::ostream &operator<<(std::ostream &os, const ArbiterType& type) {
  switch (type) {
  case ArbiterType::Priority:   os << "Priority"; break;
  case ArbiterType::RoundRobin: os << "RoundRobin"; break;
  }
  return os;
}

///////////////////////////////////////////////////////////////////////////////

struct MemReq {
    uint64_t addr;
    bool write;
    bool non_cacheable;
    uint32_t tag;
    uint32_t core_id;    
    uint64_t uuid;

    MemReq(uint64_t _addr = 0, 
           bool _write = false,
           bool _non_cacheable = false,
           uint64_t _tag = 0, 
           uint32_t _core_id = 0,
           uint64_t _uuid = 0
    )   : addr(_addr)
        , write(_write)
        , non_cacheable(_non_cacheable)
        , tag(_tag)
        , core_id(_core_id)
        , uuid(_uuid)
    {}
};

inline std::ostream &operator<<(std::ostream &os, const MemReq& req) {
  os << "mem-" << (req.write ? "wr" : "rd") << ": ";
  os << "addr=" << std::hex << req.addr << std::dec << ", tag=" << req.tag << ", core_id=" << req.core_id;
  os << " (#" << std::dec << req.uuid << ")";
  return os;
}

///////////////////////////////////////////////////////////////////////////////

struct MemRsp {
    uint64_t tag;    
    uint32_t core_id;
    uint64_t uuid;
    MemRsp(uint64_t _tag = 0, uint32_t _core_id = 0, uint64_t _uuid = 0)
      : tag (_tag) 
      , core_id(_core_id)
      , uuid(_uuid)
    {}
};

inline std::ostream &operator<<(std::ostream &os, const MemRsp& rsp) {
  os << "mem-rsp: tag=" << rsp.tag << ", core_id=" << rsp.core_id;
  os << " (#" << std::dec << rsp.uuid << ")";
  return os;
}

///////////////////////////////////////////////////////////////////////////////

struct RbtEntry {
    uint16_t buffer_id;
    uint64_t base_addr;
    uint64_t size;
    bool read_only;
    uint64_t kernel_id;

    RbtEntry(
      uint16_t _buffer_id = 0,
      uint64_t _base_addr = 0,
      uint64_t _size = 0,
      bool _read_only = 0,
      uint64_t _kernel_id = 0
    ) : buffer_id(_buffer_id)
      , base_addr(_base_addr)
      , size(_size)
      , read_only(_read_only)
      , kernel_id(_kernel_id)
    {}
};

inline std::ostream &operator<<(std::ostream &os, const RbtEntry& entry) {
  os << "(buffer_id=" << std::hex << entry.buffer_id << ", base_addr=0x" << entry.base_addr;
  os << ", size=0x" << entry.size << ", read_only=" << entry.read_only << ", kernel_id=" << entry.kernel_id;
  os << ")";
  return os;
}

///////////////////////////////////////////////////////////////////////////////

struct RbtEntryReq {
    uint64_t addr;
    uint16_t buffer_id;
    uint32_t tag;
    uint64_t uuid;

    RbtEntryReq(uint64_t _addr = 0, 
           uint16_t _buffer_id = 0,
           uint64_t _tag = 0,
           uint64_t _uuid = 0
    )   : addr(_addr)
        , buffer_id(_buffer_id)
        , tag(_tag)
        , uuid(_uuid)
    {}
};

inline std::ostream &operator<<(std::ostream &os, const RbtEntryReq& req) {
  os << "rbt-entry-req: ";
  os << "addr=0x" << std::hex << req.addr << std::dec << ", tag=" << req.tag << ", buffer_id=" << req.buffer_id;
  os << " (#" << std::dec << req.uuid << ")";
  return os;
}

///////////////////////////////////////////////////////////////////////////////

struct RbtEntryRsp {
    uint64_t tag;    
    RbtEntry* rbt_entry;
    uint64_t uuid;
    uint64_t req_addr;
    RbtEntryRsp(uint64_t _tag = 0, RbtEntry* _rbt_entry = NULL, uint64_t _uuid = 0, uint64_t _req_addr = 0)
      : tag (_tag) 
      , rbt_entry(_rbt_entry)
      , uuid(_uuid)
      , req_addr(_req_addr)
    {}
};

inline std::ostream &operator<<(std::ostream &os, const RbtEntryRsp& rsp) {
  os << "rbt-mem-rsp: tag=" << rsp.tag << ", rbt_entry=" << *rsp.rbt_entry;
  os << " (#" << std::dec << rsp.uuid << ")";
  return os;
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class HashTable {
private:
  std::vector<std::pair<bool, T>> entries_;
  uint32_t size_;

public:    
  HashTable(uint32_t capacity)
    : entries_(capacity)
    , size_(0) 
  {}

  bool empty() const {
    return (0 == size_);
  }
  
  bool full() const {
    return (size_ == entries_.size());
  }

  uint32_t size() const {
    return size_;
  }

  bool contains(uint32_t index) const {
    return entries_.at(index).first;
  }

  const T& at(uint32_t index) const {
    auto& entry = entries_.at(index);
    assert(entry.first);
    return entry.second;
  }

  T& at(uint32_t index) {
    auto& entry = entries_.at(index);
    assert(entry.first);
    return entry.second;
  }

  uint32_t allocate(const T& value) {
    for (uint32_t i = 0, n = entries_.size(); i < n; ++i) {
      auto& entry = entries_.at(i);
      if (!entry.first) {
        entry.first = true;
        entry.second = value;
        ++size_;              
        return i;
      }
    }
    assert(false);
    return -1;
  }

  void release(uint32_t index) {
    auto& entry = entries_.at(index);
    assert(entry.first);
    entry.first = false;
    --size_;
  }

  void clear() {
    for (uint32_t i = 0, n = entries_.size(); i < n; ++i) {
      auto& entry = entries_.at(i);
      entry.first = false;
    }
    size_ = 0;
  }
};

///////////////////////////////////////////////////////////////////////////////

template <typename Req, typename Rsp, uint32_t MaxInputs = 32>
class Switch : public SimObject<Switch<Req, Rsp>> {
private:
  ArbiterType type_;
  uint32_t delay_;  
  uint32_t cursor_;
  uint32_t tag_shift_;

public:
  Switch(
    const SimContext& ctx, 
    const char* name, 
    ArbiterType type, 
    uint32_t num_inputs, 
    uint32_t delay = 1
  ) 
    : SimObject<Switch<Req, Rsp, MaxInputs>>(ctx, name)    
    , type_(type)
    , delay_(delay)
    , cursor_(0)
    , tag_shift_(log2ceil(num_inputs))
    , ReqIn(num_inputs, this)
    , ReqOut(this)
    , RspIn(this)    
    , RspOut(num_inputs, this)
  {
    assert(delay_ != 0);
    assert(num_inputs <= MaxInputs);
    if (num_inputs == 1) {
      // bypass
      ReqIn.at(0).bind(&ReqOut);
      RspIn.bind(&RspOut.at(0));
    }
  }

  void reset() {
    cursor_ = 0;
  }

  void tick() {  
    if (ReqIn.size() == 1)
      return;
        
    // process incomming requests    
    for (uint32_t i = 0, n = ReqIn.size(); i < n; ++i) {      
      uint32_t j = (cursor_ + i) % n;
      auto& req_in = ReqIn.at(j);      
      if (!req_in.empty()) {
        auto& req = req_in.front();
        if (tag_shift_) {
          req.tag = (req.tag << tag_shift_) | j;
        }
        ReqOut.send(req, delay_);                
        req_in.pop();
        this->update_cursor(j);
        break;
      }
    } 

    // process incoming reponses
    if (!RspIn.empty()) {
      auto& rsp = RspIn.front();    
      uint32_t port_id = 0;
      if (tag_shift_) {
        port_id = rsp.tag & ((1 << tag_shift_)-1);
        rsp.tag >>= tag_shift_;
      }      
      RspOut.at(port_id).send(rsp, 1);
      RspIn.pop();
    }
  }

  void update_cursor(uint32_t grant) {
    if (type_ == ArbiterType::RoundRobin) {
      cursor_ = grant + 1;
    }
  }

  std::vector<SimPort<Req>>  ReqIn;
  SimPort<Req>              ReqOut;
  SimPort<Rsp>               RspIn;    
  std::vector<SimPort<Rsp>> RspOut;
};

}