#ifndef SHELL_INSTRUCTION_HPP
#define SHELL_INSTRUCTION_HPP

#include <cstdlib>
#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <array>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "Call.hpp"

class Instruction {
	private:
		std::string s;
		std::vector<std::tuple<Call, int, int>> calls;
		bool background = false, parse_error = false;
		int pid = 0;

		std::vector<Call*> active;
		std::vector<std::array<int, 2>> fds;

		enum {
			FIRST, AND, OR, SIMULTANEOUSLY, LAST
		};

		void close_descriptors();
		int wait_active_calls();
		int execute();

	public:
		Instruction(const std::string &_s);

		bool is_background() const;
		bool is_parse_error() const;

		int get_pid() const;
		void create_active_copy();

		void output_instruction(std::ostream &out) const;

		int run();
		int wait();
};

#endif
