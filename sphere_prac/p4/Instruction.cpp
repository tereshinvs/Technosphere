#include "Instruction.hpp"

#include <functional>

std::vector<Call*> *active_copy;

void instruction_sighandler(int signum) {
	std::vector<Call*> &active_copy_tmp = *active_copy;
	if (signum == SIGINT)
		for (auto p: active_copy_tmp)
			kill(p->get_pid(), SIGINT);
	active_copy_tmp.clear();	
	signal(SIGINT, instruction_sighandler);
}

void Instruction::close_descriptors() {
	for (int i = 0; i < fds.size(); ++i) {
		close(fds[i][0]);
		close(fds[i][1]);
	}
	fds.clear();
}

int Instruction::wait_active_calls() {
	int last_exit_code = 0, i = 0;
	for (auto &call: active) {
		last_exit_code = call->wait();
	}
	active.clear();
	return last_exit_code;
}

int Instruction::execute() {
	int last_exit_code = 0;

	for (int i = 0; i < calls.size(); ++i) {
		Call &call = std::get<0>(calls[i]);
		int last_action = std::get<1>(calls[i]);
		int action = std::get<2>(calls[i]);

		if (last_action == SIMULTANEOUSLY)
			call.set_in_fd(fds.back()[0]);

		if (action == SIMULTANEOUSLY) {
			std::array<int, 2> fd;
			pipe2(fd.data(), O_CLOEXEC);
			call.set_out_fd(fd[1]);
			fds.push_back(fd);
		}

		bool started = false;
		switch (last_action) {
			case FIRST: {
				started = true;
				call.run();
				break;
			}
			case AND: {
				if (!last_exit_code) {
					started = true;
					call.run();
				}
				break;
			}
			case OR: {
				if (last_exit_code) {
					started = true;
					call.run();
				}
				break;
			}
			case SIMULTANEOUSLY: {
				started = true;
				call.run();
				break;
			}
			case LAST: {
				break;
			}
			default: {
				break;
			}
		}

		if (started) {
			active.push_back(&call);
			if (action != SIMULTANEOUSLY) {
				close_descriptors();
				last_exit_code = wait_active_calls();
			}
		}
	}
	return last_exit_code;
}

Instruction::Instruction(const std::string &_s):
	s(), calls(), active(), fds() {

	int l = 0, r = _s.size() - 1;
	for ( ; l < _s.size() && std::isspace(_s[l]); ++l);
	for ( ; r >= 0 && std::isspace(_s[r]); --r);
	s = _s.substr(l, r - l + 1) + " ";

	int last_action = FIRST;
	std::size_t last = 0, i = s.find_first_of("&|;'\"");
	while (!parse_error && i != std::string::npos && last < s.size()) {
		int action = LAST;
		bool again = false;
		switch (s[i]) {
			case '&': {
				if (i < s.size() - 2) {
					if (s[i + 1] == '&')
						action = AND;
					else
						parse_error = true;
				} else {
					action = LAST;
					background = true;
				}
				break;
			}
			case '|': {
				if (i < s.size() - 1) {
					if (s[i + 1] == '|')
						action = OR;
					else
						action = SIMULTANEOUSLY;
				} else
					parse_error = true;
				break;
			}
			case ';': {
				action = LAST;
				break;
			}
			case '\'': {
				i = s.find_first_of('\'', i + 1);
				again = true;
				break;
			}
			case '\"': {
				i = s.find_first_of('\"', i + 1);
				again = true;
				break;
			}
			default: {
				parse_error = true;
				break;
			}
		}
		if (again) {
			i = s.find_first_of("&|;'\"", i + 1);
			continue;
		}
		if (last == i) {
			last = i + 1;
			i = s.find_first_of("&|;'\"", last);
			continue;
		}
		calls.push_back(std::make_tuple(
			Call(s.substr(last, i - last)),
			last_action,
			action));
		last_action = action != LAST ? action : FIRST;
		if (action == AND || action == OR)
			last = i + 2;
		else
			last = i + 1;
		if (last >= s.size())
			last = i = std::string::npos;
		else
			i = s.find_first_of("&|;'\"", last);
	}
	if (!background)
		calls.push_back(std::make_tuple(
			Call(s.substr(last, std::string::npos)),
			last_action,
			LAST));
}

bool Instruction::is_background() const {
	return background;
}

bool Instruction::is_parse_error() const {
	return parse_error;
}

int Instruction::get_pid() const {
	return pid;
}

void Instruction::create_active_copy() {
	active_copy = &active;
}

void Instruction::output_instruction(std::ostream &out) const {
	auto get_action = [](int action) -> std::string {
		if (action == FIRST)
			return "FIRST";
		else if (action == AND)
			return "AND";
		else if (action == OR)
			return "OR";
		else if (action == SIMULTANEOUSLY)
			return "SIMULTANEOUSLY";
		else if (action == LAST)
			return "LAST";
	};

	out << "Total calls: " << calls.size() << std::endl;
	out << "Background: " << background << std::endl;
	for (int i = 0; i < calls.size(); ++i) {
		out << "Call #" << i << ": " <<
			get_action(std::get<1>(calls[i])) << " " <<
			get_action(std::get<2>(calls[i])) << std::endl;
		std::get<0>(calls[i]).output_call(out);
	}
	out << std::endl;
}

int Instruction::run() {
	pid = fork();
	if (pid == 0) {
		signal(SIGINT, instruction_sighandler);
		exit(execute());
	} else
		return pid;
}

int Instruction::wait() {
	int status = 0;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}
