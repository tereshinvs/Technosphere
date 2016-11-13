#ifndef SHELL_CALL_HPP
#define SHELL_CALL_HPP

#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>

class Call {
	private:
		std::string command;
		std::vector<std::string> args;
		std::string input_file, output_file;
		int infd, outfd;
		int pid = 0;

		std::vector<const char *> get_old_style_args() const;

	public:
		Call(const std::string &_s,
			int _infd = 0,
			int _outfd = 1);

		Call(const std::string &_command,
			const std::vector<std::string> &_args,
			const std::string &_input_file = "",
			const std::string &_output_file = "",
			int _infd = 0,
			int _outfd = 1);

		void set_in_out_fd(int _infd = 0, int _outfd = 1);
		void set_in_fd(int _infd = 0);
		void set_out_fd(int _outfd = 1);

		int get_pid() const;

		void output_call(std::ostream &out) const;

		int run();
		int wait();
};

#endif
