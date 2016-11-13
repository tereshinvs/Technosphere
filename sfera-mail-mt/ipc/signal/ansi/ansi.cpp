#include <iostream>
#include <csignal>

#include <unistd.h>

void signal_handler(int signum);

void set_signal(int signum, sighandler_t handler)
{
	std::string handler_str;
	if(handler == SIG_DFL)
	{
		handler_str = "SIG_DFL";
	}
	else if(handler == SIG_IGN)
	{
		handler_str = "SIG_IGN";
	}
	else if(handler == signal_handler)
	{
		handler_str = "signal_handler";
	}

	std::cout << getpid() << ": signal(" << signum << ", " << handler_str << ");" << std::endl;
	signal(signum, handler);
}

void signal_handler(int signum)
{
	std::cout << getpid() << ": signal_handler(" << signum << ");" << std::endl;
	switch(signum)
	{
	case SIGUSR1:
		set_signal(SIGUSR1, signal_handler);
		break;
	case SIGUSR2:
		set_signal(SIGUSR2, SIG_DFL);
		break;
	}
}


int child()
{
	set_signal(SIGUSR1, signal_handler);
	set_signal(SIGUSR2, signal_handler);
	set_signal(SIGTERM, SIG_IGN);
	while(true) { pause(); }
}

void sleep_and_kill(pid_t & pid, int signum)
{
	sleep(1);
	std::cout << getpid() << ": kill(" << pid << ", " << signum << ");" << std::endl;
	kill(pid, signum);
}

int parent(pid_t & child_pid)
{
	sleep_and_kill(child_pid, SIGTERM);
	sleep_and_kill(child_pid, SIGTERM);
	sleep_and_kill(child_pid, SIGTERM);
	sleep_and_kill(child_pid, SIGUSR1);
	sleep_and_kill(child_pid, SIGUSR1);
	sleep_and_kill(child_pid, SIGUSR1);
	sleep_and_kill(child_pid, SIGUSR2);
	sleep_and_kill(child_pid, SIGUSR2);
	sleep_and_kill(child_pid, SIGUSR2);
}

int main(int argc, char ** argv)
{
	pid_t pid = fork();
	switch(pid)
	{
	case -1:
		return 1;
	case 0:
		return child();
	default:
		return parent(pid);
	}
	return 0;
}
