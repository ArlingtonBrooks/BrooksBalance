#ifndef COMPUTER_HPP_
#define COMPUTER_HPP_

#include <vector>

struct Device {
	long unsigned Latency;
	long unsigned Factor;
	long unsigned ID_Start = 0;
	long unsigned ID_End = 0;
	char type = 0;
};

struct Computer {
	std::vector<Device> Devices;
	long unsigned Latency;
};

#endif
