#include <vuh/array.hpp>
#include <vuh/vuh.h>
#include <vector>
#include <set>
#include <string>
#include <chrono>
#include <iostream>
#include <string>

const int minWorkgroups = 128;
const int maxWorkgroups = 128;
const int minWorkgroupSize = 1;
const int maxWorkgroupSize = 1;
const int expectedCount = 20480;
const int numIterations = 100;

using Array = vuh::Array<uint32_t,vuh::mem::Host>;
using SpecConstants = vuh::typelist<uint32_t>;

class FenceProfiler {

public:
    void run(std::string testName) {
	std::cout << "Running test " << testName << "\n";
        // setup devices, memory, and parameters
        auto instance = vuh::Instance();
        auto device = instance.devices().at(0);
	std::vector<Array> buffers;
	initializeBuffers(device, buffers, testName);
	auto var = Array(device, 1);
        auto paramsBuffer = Array(device, 2);
	std::string testFile(testName + ".spv");
	std::chrono::duration<double> results[numIterations];
	double sum = 0;
	std::chrono::time_point<std::chrono::system_clock> start, end;
	for (int numWorkgroups = minWorkgroups; numWorkgroups <= maxWorkgroups; numWorkgroups*=2) {
	    std::cout << "\nTest workgroups " << numWorkgroups << "\n"; 
	    int iterationsPerTest = expectedCount/numWorkgroups;
	    paramsBuffer[0] = numWorkgroups;
	    paramsBuffer[1] = iterationsPerTest;
	    for (int i = 0; i < numIterations + 1; i++) {
	        std::cout << "\ntest iteration " << i << "\n";
	    	int workgroupSize = setWorkgroupSize();
	    	clearMemory(var, 1);
            	auto program = vuh::Program<SpecConstants>(device, testFile.c_str());
	    	program.grid(numWorkgroups);
	    	program.spec(workgroupSize);
	    	bindBuffers(program, buffers, var, paramsBuffer, testName);
	    	start = std::chrono::system_clock::now();
		try {
		    program.run();
		} 
		catch (const std::runtime_error& e) {
        	    printf("%s\n", e.what());
	    	    end = std::chrono::system_clock::now();
	            std::chrono::duration<double> result = end - start;
	    	    std::cout << "iteration time: " << result.count() << "s\n";
	    	    std::cout << "expected: " << expectedCount << ", actual: " << var[0] << "\n";
		    return;
    		}
	    	end = std::chrono::system_clock::now();
	    	std::chrono::duration<double> result = end - start;
		if (i > 0) sum += result.count();
	    	std::cout << "iteration time: " << result.count() << "s\n";
	    	std::cout << "expected: " << expectedCount << ", actual: " << var[0] << "\n";
	    	if (expectedCount != var[0]) {
		    std::cout << "Expected not equal to actual!\n";
	    	}
	    }
	    std::cout << "Average test iteration time: " << sum / numIterations << "s\n";
	    sum = 0;
	}
    }

    void initializeBuffers(vuh::Device &device, std::vector<Array> &buffers, std::string testName) {
	if (testName == "lamports-bakery") {
	    buffers.push_back(Array(device, maxWorkgroups));
            buffers.push_back(Array(device, maxWorkgroups));
	} else if (testName == "petersons") {
	    buffers.push_back(Array(device, maxWorkgroups));
	    buffers.push_back(Array(device, maxWorkgroups - 1));
	} else if (testName == "spin-lock") {
	    buffers.push_back(Array(device, 1));
	} else if (testName == "dekker-fences") {
	    buffers.push_back(Array(device, maxWorkgroups));
	    buffers.push_back(Array(device, 1));
	}
    }

    void bindBuffers(vuh::Program<SpecConstants> &program, std::vector<Array> &buffers, Array &var, Array &paramsBuffer, std::string testName) {
	if (testName == "lamports-bakery") {
	    clearMemory(buffers[0], maxWorkgroups);
	    clearMemory(buffers[1], maxWorkgroups);
    	    program.bind(buffers[0], buffers[1], var, paramsBuffer); 
	} else if (testName == "petersons") {
	    clearMemory(buffers[0], maxWorkgroups);
	    clearMemory(buffers[1], maxWorkgroups - 1);
    	    program.bind(buffers[0], buffers[1], var, paramsBuffer); 
	} else if (testName == "spin-lock") {
	    clearMemory(buffers[0], 1);
	    program.bind(buffers[0], var, paramsBuffer);
	} else if (testName == "dekker-fences") {
	    clearMemory(buffers[0], maxWorkgroups);
	    clearMemory(buffers[1], 1);
    	    program.bind(buffers[0], buffers[1], var, paramsBuffer); 
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
    if (argc != 2) {
	std::cout << "Test name must be specified\n";
	return 1;
    }
    std::string testName(argv[1]);
    srand (time(NULL));
    FenceProfiler app;
    try {
        app.run(testName);
    }
    catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return 1;
    }
    return 0;
}


