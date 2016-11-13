#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp> 
#include <boost/exception_ptr.hpp>

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <ctime>
//#include <cmemory>

#include "Semaphore.hpp"
#include "HashTable.hpp"
#include "Coworker.hpp"
#include "sock_ops.h"
#include "map_file.h"

const int PORT = 8080;

const int N_COWORKERS = 4;

const std::string DATA_FILE_NAME = "./cache_data.data";
const std::string SEMAPHORE_FILE_NAME = "./semaphore.sem";
const int SEMAPHORE_PROJID = 0;
const std::size_t KEY_SIZE = 32;
const std::size_t VALUE_SIZE = 256;
const std::size_t N_RECORDS = 3000;
const std::size_t DATA_SIZE =
	N_RECORDS * (KEY_SIZE + VALUE_SIZE + sizeof(unsigned) + sizeof(char));

int main(int argc, char *argv[]) {
	bool save_map = false, help_need = false;

	boost::program_options::options_description opts("Options");
	opts.add_options()
		("old,o", "Old")
		("help,h", "Help");

	boost::program_options::variables_map vm;
	try {
		boost::program_options::parsed_options parsed =
			boost::program_options::command_line_parser(argc, argv).
				options(opts).allow_unregistered().run();
		boost::program_options::store(parsed, vm);

		if (vm.count("old"))
			save_map = true;
		if (vm.count("help"))
			help_need = true;
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return 0;
	}

	char *data = map_file(DATA_FILE_NAME.data(), DATA_SIZE);
	if (data == nullptr)
		return 0;
	if (!save_map)
		std::memset(data, DATA_SIZE, 0);
	Semaphore semaphore(N_RECORDS, N_COWORKERS, SEMAPHORE_FILE_NAME, SEMAPHORE_PROJID);
	std::shared_ptr<HashTable> hash_table(new HashTable(
		data, semaphore, DATA_SIZE,
		N_RECORDS, KEY_SIZE, VALUE_SIZE,
		[](const std::string &key, std::size_t k) -> std::size_t {
			return (std::hash<std::string>()(key) + k) % N_RECORDS;
		}));
	hash_table->run_garbage_collector();

	std::vector<Coworker> coworkers;
	for (int i = 0; i < N_COWORKERS; ++i) {
		coworkers.push_back(Coworker(hash_table, i));
		coworkers[i].run();
	}
	std::srand(std::time(0));

	int master_socket = get_master_socket_and_listen(PORT);
	if (master_socket == -1)
		return 0;
	while (true) {
		int slave = get_slave_socket(master_socket);
		if (slave != -1)
			coworkers[std::rand() % N_COWORKERS].send_fd(slave);
		close_socket(slave);
	}

	close_socket(master_socket);
	for (auto &p: coworkers)
		p.stop();
	hash_table.reset();
	unmap_file(data, DATA_SIZE);

	return 0;
}
