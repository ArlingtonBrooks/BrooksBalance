#include "Block.hpp"
#include "Computer.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

//Make a "computer" which contains CPUs and GPUs
//N_GPUs: number of GPUs
//N_CPUs: number of CPUs
//Latency_GPU: latency of sending data to GPU (unused)
//Latency_CPU: latency of sending data to CPU (unused)
//Latency_Node: communications latency (unused)
//Factor_GPU: compute performance factor for the GPU (this could be double?)
//Factor_CPU: compute performance factor of the CPU (this could be double?)
Computer MakeComputer(long unsigned N_GPUs, long unsigned N_CPUs, long unsigned Latency_GPU, long unsigned Latency_CPU, long unsigned Latency_Node, long unsigned Factor_GPU, long unsigned Factor_CPU)
{
	Computer C;
	for (long unsigned i = 0; i != N_GPUs; i++) {
		Device D;
		D.Latency = Latency_GPU;
		D.Factor = Factor_GPU;
		D.type = 'G';
		C.Devices.push_back(D);
	}
	for (long unsigned i = 0; i != N_CPUs; i++) {
		Device D;
		D.Latency = Latency_CPU;
		D.Factor = Factor_CPU;
		D.type = 'C';
		C.Devices.push_back(D);
	}
	return C;
}

//Balance the computational load of a bunch of blocks over a bunch of computers
void BalanceLoad(std::vector<Computer> &Computers, std::vector<Block> &Blocks)
{
	long unsigned Time_Total = 0;
	unsigned TotalDevs = 0;
	for (auto i : Computers) {
		for (auto j : i.Devices) {
			for (long unsigned k = j.ID_Start; k != j.ID_End; k++) { //I hate nesting.
				Time_Total += Blocks[k].t_compute_ns * j.Factor;
				Blocks[k].t_compute_ns *= j.Factor; //bring compute time into global reference;
			}
			TotalDevs += j.Factor;
		}
	}
	long unsigned Time_Average = Time_Total / TotalDevs; //in terms of reference time;
	std::cout << "AV: " << Time_Average << "\n"; //debuginfo
	long unsigned ID = 0;
	unsigned Comp_ID = 0;
	unsigned Dev_ID = 0;
	long unsigned time_tmp = 0;
	while (ID != Blocks.size()) {
		Computers[Comp_ID].Devices[Dev_ID].ID_Start = ID;
std::cout << "Processing device type " << Computers[Comp_ID].Devices[Dev_ID].type << "\n"; //debuginfo
		while (time_tmp < Time_Average && ID < Blocks.size()) { //could stagger this by computer ID (avoid under-subscribing)
			time_tmp += Blocks[ID].t_compute_ns / Computers[Comp_ID].Devices[Dev_ID].Factor;
			ID += 1;
		}
std::cout << "Last block: " << Blocks[ID-1].t_compute_ns / Computers[Comp_ID].Devices[Dev_ID].Factor << "\n"; //debuginfo
std::cout << time_tmp << "\n"; //debuginfo
		Computers[Comp_ID].Devices[Dev_ID].ID_End = ID;
		Dev_ID += 1;
		time_tmp = 0;
		if (Dev_ID == Computers[Comp_ID].Devices.size()) {
			Dev_ID = 0;
			Comp_ID += 1;
		}
		if (Comp_ID == Computers.size())
			break;
	}
	Computers.back().Devices.back().ID_End = Blocks.size()-1;
}

int main(int, char**)
{
	std::vector<Block> Blocks;
	std::default_random_engine generator;
	std::lognormal_distribution<double> distribution(16.0,0.5); //random times (not representative of a real load)

	for (long unsigned i = 0; i != 1000000; i++) {
		Block V;
		V.t_compute_ns = distribution(generator) + 100070;
		Blocks.push_back(V);
	}

	//Create a bunch of computers connected together
	std::vector<Computer> Computers;
	Computers.push_back(MakeComputer(2, 16, 33420, 2321, 9022, 32,   1));
	Computers.push_back(MakeComputer(2, 32, 33420, 2321, 9022, 32,   4));
	Computers.push_back(MakeComputer(2, 32, 33420, 2321, 9022, 120,  5));
	Computers.push_back(MakeComputer(2, 8,  33420, 2321, 9022, 221,  5));
	Computers.push_back(MakeComputer(2, 8,  33420, 2321, 9022, 211,  1));
	Computers.push_back(MakeComputer(2, 4,  33420, 2321, 9022, 104,  1));
	Computers.push_back(MakeComputer(2, 4,  33420, 2321, 9022, 45,   1));
	Computers.push_back(MakeComputer(2, 4,  33420, 2321, 9022, 45,   1));
	unsigned N_Devices = 0;
	for (auto const &i : Computers) {
		N_Devices += i.Devices.size();
	}
	
	unsigned Naive_N_Per_Device = Blocks.size() / N_Devices;
	unsigned ID = 0;
	for (auto &i : Computers) {
		for (auto &j : i.Devices) {
			j.ID_Start = ID;
			ID += Naive_N_Per_Device;
			if (ID > Blocks.size()) ID = Blocks.size();
			j.ID_End = ID;
			for (unsigned k = j.ID_Start; k != j.ID_End; k++) {
				Blocks[k].t_compute_ns /= j.Factor;
			}
		}
	}
	Computers.back().Devices.back().ID_End = Blocks.size();

	//NOTE: WE DO NOT COMPUTE THE TOTAL TIME SINCE WE HAVEN'T ACCOUNTED FOR LATENCY YET!
	for (unsigned i = 0; i != Computers.size(); i++) {
		std::cout << "Computer (" << i << ")\n";
		for (unsigned j = 0; j != Computers[i].Devices.size(); j++) {
			std::cout << "\t" << "(" << Computers[i].Devices[j].type << ")" << Computers[i].Devices[j].ID_Start << "--" << Computers[i].Devices[j].ID_End << "\n";
		}
	}
	BalanceLoad(Computers, Blocks);
	for (unsigned i = 0; i != Computers.size(); i++) {
		std::cout << "Computer (" << i << ")\n";
		for (unsigned j = 0; j != Computers[i].Devices.size(); j++) {
			std::cout << "\t" << "(" << Computers[i].Devices[j].type << ")" << Computers[i].Devices[j].ID_Start << "--" << Computers[i].Devices[j].ID_End << "\n";
		}
	}
	//It would be nice to print the longest compute time for the un-sorted case and compare, but there's cake afoot.
}
