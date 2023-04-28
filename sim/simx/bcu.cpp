#include "bcu.h"

using namespace vortex;

void BcuUnit::tick() {
    if (Input.empty())
        return;

    auto trace = Input.front();
    if (trace->mem_addrs.size() == 0)
        return;
    auto mem_addr = trace->mem_addrs.at(0).at(0);
    DT(1, "bcu-req: " << std::hex << mem_addr.addr << ", " << *trace);
    Input.pop();
}