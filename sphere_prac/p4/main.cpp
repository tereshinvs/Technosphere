/*
OS: Ubuntu 14.04 x64
 
Warning: a lot of hacks in sources.
Shell doesn't work properly on test #6.
However the output of this program is equal to
bash output on my computer for this script.
*/

#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <functional>

#include "Instruction.hpp"

std::vector<Instruction*> active;

void sighandler(int signum) {
	if (signum == SIGINT)
		for (auto p: active) {
			p->create_active_copy(); // Hack
			kill(p->get_pid(), SIGINT);
		}
	active.clear();	
	signal(SIGINT, sighandler);
}

int main() {
	signal(SIGINT, sighandler);

	std::string s;
	while (std::getline(std::cin, s)) {
		Instruction instruction(s);
		//instruction.output_instruction(std::cerr);
		int pid = instruction.run();
		if (instruction.is_background()) {
			std::cerr << "Spawned child process " << pid << std::endl;
			active.push_back(&instruction);
		} else {
			int result = instruction.wait();
			std::cerr << "Process " << pid << " exited: " << result << std::endl;
		}
	}

	for (auto p: active) {
		int result = p->wait();
		std::cerr << "Process " << p->get_pid() << " exited: " << result << std::endl;		
	}
	return 0;
}
