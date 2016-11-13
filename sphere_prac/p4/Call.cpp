#include "Call.hpp"

#include <cctype>

std::vector<const char *> Call::get_old_style_args() const {
	std::vector<const char*> res(args.size() + 2);
	for (int i = 1; i <= args.size(); i++)
		res[i] = args[i - 1].data();
	res[0] = command.data();
	res.back() = nullptr;
	return res;
}

Call::Call(const std::string &_s,
	int _infd,
	int _outfd):
	command(),
	args(),
	input_file(),
	output_file(),
	infd(_infd),
	outfd(_outfd) {

	int l = 0, r = _s.size() - 1;
	for ( ; l < _s.size() && std::isspace(_s[l]); ++l);
	for ( ; r >= 0 && std::isspace(_s[r]); --r);
	std::string s = _s.substr(l, r - l + 1) + " ";

	std::size_t command_end = s.find_first_of(" <>");
	if (s[0] == '\'')
		command_end = s.find_first_of('\'', 1);
	if (s[0] == '\"')
		command_end = s.find_first_of('\"', 1);
	std::size_t last = command_end + 1;
	if (s[0] != '\'' && s[0] != '\"') {
		command = s.substr(0, command_end);
		last = command_end;
	} else {
		command = s.substr(0, command_end + 1);
		last = command_end + 1;
	}

	bool input_file_need = false, output_file_need = false;
	for (std::size_t i = s.find_first_of(" <>\'\"", last);
		i < s.size() && i != std::string::npos;
		last = i + 1, i = s.find_first_of(" <>\'\"", i + 1)) {

		std::string token = "";
		if (s[i] == '\'') {
			std::size_t tmp = s.find_first_of("\'", i + 1);
			token = s.substr(i, tmp - i + 1);
			i = tmp;
		} else if (s[i] == '\"') {
			std::size_t tmp = s.find_first_of("\"", i + 1);
			token = s.substr(i, tmp - i + 1);
			i = tmp;
		} else
			token = s.substr(last, i - last);

		if (token != "") {
			if (input_file_need) {
				input_file = token;
				input_file_need = false;
			} else if (output_file_need) {
				output_file = token;
				output_file_need = false;
			} else
				args.push_back(token);
		}

		if (s[i] == '<') {
			input_file_need = true;
			continue;
		}
		if (s[i] == '>') {
			output_file_need = true;
			continue;
		}
	}
}

Call::Call(const std::string &_command,
	const std::vector<std::string> &_args,
	const std::string &_input_file,
	const std::string &_output_file,
	int _infd,
	int _outfd):
	command(_command),
	args(_args),
	input_file(_input_file),
	output_file(_output_file),
	infd(_infd),
	outfd(_outfd) {
}

void Call::set_in_out_fd(int _infd, int _outfd) {
	infd = _infd;
	outfd = _outfd;
}

void Call::set_in_fd(int _infd) {
	infd = _infd;
}

void Call::set_out_fd(int _outfd) {
	outfd = _outfd;
}

int Call::get_pid() const {
	return pid;
}

void Call::output_call(std::ostream &out) const {
	out << "[" << command << "] ";
	if (input_file != "")
		out << "<[" << input_file << "] ";
	if (output_file != "")
		out << ">[" << output_file << "] ";
	for (const auto &arg: args)
		out << "[" << arg << "] ";
	out << std::endl;
}

int Call::run() {
	int cur_infd = infd, cur_outfd = outfd;
	bool infd_rwd = false, outfd_rwd = false;
	if (input_file != "") {
		cur_infd = open(input_file.data(), O_RDONLY | O_CLOEXEC);
		infd_rwd = true;
	}
	if (output_file != "") {
		cur_outfd = open(output_file.data(),
			O_WRONLY | O_CREAT | O_TRUNC | O_APPEND | O_CLOEXEC, 0666);
		outfd_rwd = true;
	}

	if ((pid = fork()) == 0) {
		dup2(cur_infd, 0);
		dup2(cur_outfd, 1);
		execvp(command.data(),
			const_cast<char * const *>(get_old_style_args().data()));
	}

	if (infd_rwd)
		close(cur_infd);
	if (outfd_rwd)
		close(cur_outfd);
	return pid;
}

int Call::wait() {
	int status = 0;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}
