#include "bcu.h"

void BcuUnit::tick() {
    auto trace = Input.front();
    for (auto& mem_addr : trace->mem_addrs) {
        uint64_t addr = mem_addr.at(0).addr;
        DT(1, "bcu-req: addr=" << std::hex << addr << ", " << *trace);
    }
}