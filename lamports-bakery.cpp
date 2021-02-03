#include <vuh/array.hpp>
#include <vuh/vuh.h>
#include <vector>
#include <set>
#include <string>

const int minWorkgroups = 36;
const int maxWorkgroups = 36;
const int minWorkgroupSize = 24;
const int maxWorkgroupSize = 24;

using Array = vuh::Array<uint32_t,vuh::mem::Host>;
class FenceProfiler {

public:
    void run() {
        // setup devices, memory, and parameters
        auto instance = vuh::Instance();
        auto device = instance.devices().at(0);
        auto entering = Array(device, maxWorkgroups);
	    auto tickets = Array(device, maxWorkgroups);
	    auto var = Array(device, 1);
        auto numWorkgroupsBuffer = Array(device, 1);
        using SpecConstants = vuh::typelist<uint32_t>;
	    std::string testFile("lamports-bakery.spv");
	    for (int i = 0; i < 10; i++) {
	        printf("\ntest iteration %i\n", i);
	        int numWorkgroups = setNumWorkgroups();
	        int workgroupSize = setWorkgroupSize();
	        printf("number of workgroups: %i\n", numWorkgroups);
	        printf("workgroup size: %i\n", workgroupSize);
            clearMemory(entering, maxWorkgroups);
	        clearMemory(tickets, maxWorkgroups);
	        clearMemory(var, 1);
	        numWorkgroupsBuffer[0] = numWorkgroups;
            auto program = vuh::Program<SpecConstants>(device, testFile.c_str());
            program.grid(numWorkgroups).spec(workgroupSize)(entering, tickets, var, numWorkgroupsBuffer);
	        printf("var: %u\n", var[0]);
	}
    }

    void clearMemory(Array &gpuMem, int size) {
	for (int i = 0; i < size; i++) {
		gpuMem[i] = 0;
	}
    }
    
    int setWorkgroupSize() {
	if (minWorkgroupSize == maxWorkgroupSize) {
	    return minWorkgroupSize;
	} else {
 	    int size = rand() % (maxWorkgroupSize - minWorkgroupSize);
            return minWorkgroupSize + size;
	}
    }

    int setNumWorkgroups() {
	if (minWorkgroups == maxWorkgroups) {
	    return minWorkgroups;
	} else {
	    int size = rand() % (maxWorkgroups - minWorkgroups);
            return minWorkgroups + size;
	}
    }
};

int main(int argc, char* argv[]) {
    srand (time(NULL));
    FenceProfiler app;
    try {
        app.run();
    }
    catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return 1;
    }
    return 0;
}

