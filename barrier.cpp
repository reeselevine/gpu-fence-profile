#include <vuh/array.hpp>
#include <vuh/vuh.h>
#include <vector>
#include <set>
#include <string>

const int minWorkgroups = 4;
const int maxWorkgroups = 4;
const int minWorkgroupSize = 24;
const int maxWorkgroupSize = 24;
const int numIterations = 2;

using Array = vuh::Array<uint32_t,vuh::mem::Host>;
class FenceProfiler {

public:
    void run() {
        // setup devices, memory, and parameters
        auto instance = vuh::Instance();
        auto device = instance.devices().at(0);
        auto barrier = Array(device, 1);
	auto flag = Array(device, 1);
	auto data = Array(device, 1);
	auto results = Array(device, maxWorkgroups);
	auto paramsBuffer = Array(device, 2);
        using SpecConstants = vuh::typelist<uint32_t>;
	std::string testFile("barrier.spv");
	for (int i = 0; i < 10; i++) {
	    printf("\ntest iteration %i\n", i);
	    int numWorkgroups = setNumWorkgroups();
	    int workgroupSize = setWorkgroupSize();
            clearMemory(barrier, 1);
	    clearMemory(flag, 1);
	    clearMemory(data, 1);
	    clearMemory(results, maxWorkgroupSize);
	    paramsBuffer[0] = numWorkgroups;
	    paramsBuffer[1] = numIterations;
            auto program = vuh::Program<SpecConstants>(device, testFile.c_str());
            program.grid(numWorkgroups).spec(workgroupSize)(barrier, flag, data, results, paramsBuffer);
	    for (int i = 0; i < numWorkgroups - 1; i++) {
		printf("%ith workgroup result: %i\n", i, results[i]);
	    }
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


