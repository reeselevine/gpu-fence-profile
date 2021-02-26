#include <vuh/array.hpp>
#include <vuh/vuh.h>
#include <vector>
#include <set>
#include <string>
#include <chrono>
# include <iostream>

const int minWorkgroups = 36;
const int maxWorkgroups = 36;
const int minWorkgroupSize = 24;
const int maxWorkgroupSize = 24;
const int iterationsPerTest = 1000;
const int numIterations = 10;

using Array = vuh::Array<uint32_t,vuh::mem::Host>;
using SpecConstants = vuh::typelist<uint32_t>;

class FenceProfiler {

public:
    void run(char* testName) {
	std::cout << "Running test " << testName << "\n";
        // setup devices, memory, and parameters
        auto instance = vuh::Instance();
        auto device = instance.devices().at(0);
	std::vector<Array> buffers;
	if (testName == "lamports-bakery") {
	    initializeLamportsBakery(&device, buffers);
	}
        auto entering = Array(device, maxWorkgroups);
	auto tickets = Array(device, maxWorkgroups);
	auto var = Array(device, 1);
        auto paramsBuffer = Array(device, 2);
	std::string testFile("lamports-bakery.spv");
	std::chrono::duration<double> results[numIterations];
	double sum = 0;
	for (int i = 0; i < numIterations; i++) {
	    std::chrono::time_point<std::chrono::system_clock> start, end;
	    std::cout << "\n test iteration " << i << "\n";
	    int numWorkgroups = setNumWorkgroups();
	    int workgroupSize = setWorkgroupSize();
            clearMemory(entering, maxWorkgroups);
	    clearMemory(tickets, maxWorkgroups);
	    clearMemory(var, 1);
	    paramsBuffer[0] = numWorkgroups;
	    paramsBuffer[1] = iterationsPerTest;
            auto program = vuh::Program<SpecConstants>(device, testFile.c_str());
	    program.grid(numWorkgroups);
	    program.spec(workgroupSize);
	    start = std::chrono::system_clock::now();
            program.grid(numWorkgroups).spec(workgroupSize)(entering, tickets, var, paramsBuffer);
	    end = std::chrono::system_clock::now();
	    int expectedCount = iterationsPerTest * numWorkgroups;
	    std::chrono::duration<double> result = end - start;
	    sum += result.count();
	    std::cout << "iteration time: " << result.count() << "s\n";
	    std::cout << "expected: " << expectedCount << ", actual: " << var[0] << "\n";
	}
	std::cout << "Average test iteration time: " << sum / numIterations << "s\n";
    }

    void initializeLamportsBakery(vuh::Device* device, std::vector<Array> &buffers) {
   	buffers.push_back(Array(*device, maxWorkgroups));
        buffers.push_back(Array(*device, maxWorkgroups));
        buffers.push_back(Array(*device, 1));
    }

    void bindLamportsBakery(vuh::Program<SpecConstants> *program, std::vector<Array> &buffers, Array *paramsBuffer) {
	clearMemory(buffers[0], maxWorkgroups);
	clearMemory(buffers[1], maxWorkgroups);
	clearMemory(buffers[2], 1);
    	program->bind(buffers[0], buffers[1], buffers[2], *paramsBuffer); 
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
    if (argc != 2) {
	std::cout << "Test name must be specified\n";
	return 1;
    }
    srand (time(NULL));
    FenceProfiler app;
    try {
        app.run(argv[1]);
    }
    catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return 1;
    }
    return 0;
}


