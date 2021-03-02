#include <vuh/array.hpp>
#include <vuh/vuh.h>
#include <vector>
#include <set>
#include <string>
#include <chrono>
#include <iostream>

const int minWorkgroups = 2;
const int maxWorkgroups = 256;
const int minWorkgroupSize = 1;
const int maxWorkgroupSize = 1;
const int numIterations = 100;
const int iterationsPerTest = 200000;

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
	std::chrono::time_point<std::chrono::system_clock> start, end;
	int expectedCount = 0;
	for (int i = 0; i < iterationsPerTest; i++) {
	    expectedCount += i + 1;
	}
	for (int numWorkgroups = minWorkgroups; numWorkgroups <= maxWorkgroups; numWorkgroups *= 2) {
	std::cout << "\nNumber of workgroups: " << numWorkgroups << "\n";
	double sum = 0;
	for (int i = 0; i < numIterations + 1; i++) {
	    printf("\ntest iteration %i\n", i);
	    int workgroupSize = setWorkgroupSize();
            clearMemory(barrier, 1);
	    clearMemory(flag, 1);
	    clearMemory(data, 1);
	    clearMemory(results, maxWorkgroups);
	    paramsBuffer[0] = numWorkgroups;
	    paramsBuffer[1] = iterationsPerTest;
            auto program = vuh::Program<SpecConstants>(device, testFile.c_str());
            program.grid(numWorkgroups);
	    program.spec(workgroupSize);
	    program.bind(barrier, flag, data, results, paramsBuffer);
	    start = std::chrono::system_clock::now();
	    program.run();
	    end = std::chrono::system_clock::now();
	    std::chrono::duration<double> result = end - start;
	    if (i > 0) sum += result.count();
	    std::cout << "iteration time: " << result.count() << "s\n";
	    for (int i = 0; i < numWorkgroups - 1; i++) {
		if (results[i] != expectedCount) {
		    printf("%ith workgroup result: %i\n", i, results[i]);
		}
	    }
	}
	std::cout << "Average test iteration time: " << sum / numIterations << "s\n";
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


