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
        auto barrier = Array(device, 1);
	auto data = Array(device, 1);
	auto results = Array(device, maxWorkgroups);
	auto numWorkgroupsBuffer = Array(device, 1);
        using SpecConstants = vuh::typelist<uint32_t>;
	std::string testFile("barrier.spv");
	for (int i = 0; i < 10; i++) {
	    printf("\ntest iteration %i\n", i);
	    int numWorkgroups = setNumWorkgroups();
	    int workgroupSize = setWorkgroupSize();
	    printf("number of workgroups: %i\n", numWorkgroups);
	    printf("workgroup size: %i\n", workgroupSize);
            clearMemory(barrier, 1);
	    clearMemory(data, 1);
	    clearMemory(results, maxWorkgroupSize);
	    numWorkgroupsBuffer[0] = numWorkgroups;
            auto program = vuh::Program<SpecConstants>(device, testFile.c_str());
            program.grid(numWorkgroups).spec(workgroupSize)(barrier, data, results, numWorkgroupsBuffer);
	    for (int i = 0; i < numWorkgroups - 1; i++) {
		    if (results[i] != 1) {
			    printf("%ith memory location is %i, which is not equal to 1\n", i, results[i]);
			    break;
		    }
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


