#include <BGrid.h>

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

static void assertNear(double actual, double expected, double eps, const char* msg) {
    if (std::fabs(actual - expected) > eps) {
        std::cerr << msg << " expected=" << expected << " actual=" << actual << "\n";
        std::abort();
    }
}

static void assertEq(const std::string& actual, const std::string& expected, const char* msg) {
    if (actual != expected) {
        std::cerr << msg << " expected=\"" << expected << "\" actual=\"" << actual << "\"\n";
        std::abort();
    }
}

int main() {
    BGrid bgrid;

    // Vectors computed from bgrid-js (coordsToBGrid + bgridToCell).
    {
        const std::string idx = bgrid.ddToBGrid(40.7128, -74.0060, 3);
        assertEq(idx, "531,1563,1600", "NYC ddToBGrid");
        auto dd = bgrid.bGridToDD(idx);
        assertNear(dd.first, 40.713958740234375, 1e-6, "NYC lat");
        assertNear(dd.second, -74.00527954101562, 1e-6, "NYC lon");
    }

    {
        const std::string idx = bgrid.ddToBGrid(0.0, 0.0, 2);
        assertEq(idx, "1057,1", "Origin ddToBGrid");
        auto dd = bgrid.bGridToDD(idx);
        assertNear(dd.first, -0.0439453125, 1e-6, "Origin lat");
        assertNear(dd.second, 0.087890625, 1e-6, "Origin lon");
    }

    {
        const std::string idx = bgrid.ddToBGrid(-33.8688, 151.2093, 4);
        assertEq(idx, "1467,61,718,533", "Sydney ddToBGrid");
        auto dd = bgrid.bGridToDD(idx);
        assertNear(dd.first, -33.86881113052368, 1e-6, "Sydney lat");
        assertNear(dd.second, 151.20934009552002, 1e-6, "Sydney lon");
    }

    std::cout << "bgrid_test: OK\n";
    return 0;
}
