#include <vuh/array.hpp>
#include <vuh/vuh.h>
#include <vector>
#include <set>
#include <string>

const int minWorkgroups = 36;
const int maxWorkgroups = 36;
const int minWorkgroupSize = 8;
const int maxWorkgroupSize = 8;
const int numIterations = 10;

using Array = vuh::Array<uint32_t,vuh::mem::Host>;
class FenceProfiler {

public:
    void run() {
        // setup devices, memory, and parameters
        auto instance = vuh::Instance();
        auto device = instance.devices().at(0);
        auto flag = Array(device, 1);
	auto var = Array(device, 1);
        auto paramsBuffer = Array(device, 1);
        using SpecConstants = vuh::typelist<uint32_t>;
	std::string testFile("spin-lock.spv");
	for (int i = 0; i < 10; i++) {
	    printf("\ntest iteration %i\n", i);
	    int numWorkgroups = setNumWorkgroups();
	    int workgroupSize = setWorkgroupSize();
            clearMemory(flag, 1);
	    clearMemory(var, 1);
	    paramsBuffer[0] = numIterations;
            auto program = vuh::Program<SpecConstants>(device, testFile.c_str());
            program.grid(numWorkgroups).spec(workgroupSize)(flag, var, paramsBuffer);
	    int expectedCount = numIterations * numWorkgroups;
	    printf("expected: %i, actual: %u\n", expectedCount, var[0]);
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


