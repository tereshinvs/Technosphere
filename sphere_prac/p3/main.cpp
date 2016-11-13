#include <boost/asio.hpp>
#include <boost/program_options.hpp>

int main() {
	std::string config_path = "./";
	bool log_need = false;
	bool help_need = false;

	boost::program_options::options_description opts("Options");
	opts.add_options()
		("config,c", boost::program_options::value<std::string>(), "Config")
		("log,l", "Log")
		("help,h", "Help");

	boost::program_options::variables_map vm;
	try {
		boost::program_options::parsed_options parsed =
			boost::program_options::command_line_parser(argc, argv).
				options(opts).allow_unregistered().run();
		boost::program_options::store(parsed, vm);

		if (vm.count("config"))
			config_path = vm["config"].as<std::string>();
		if (vm.count("log"))
			log_need = true;
		if (vm.count("help"))
			help_need = true;
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return 0;
	}

	std::set<Client> clients;
	std::mutex clients_mtx;
	ClientBuilder builder(
		MAX_BUFFER_SIZE,
		[&clients, &clients_mtx](const Client &client) {
			clients_mtx.lock();
			clients.erase(client);
			clients_mtx.unlock();
		});

	boost::asio::io_service io_service;
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), http_port);
	boost::asio::ip::tcp::acceptor acceptor(io_service, ep);
	acceptor.listen();
	while (true) {
		boost::asio::ip::tcp::socket slave(io_service);
		acceptor.accept(slave);
		if (log_need)
			std::cout << "Accepted" << std::endl;

		clients.insert(builder.build_client(slave));
/*		char buf[65536] = {0};
		try {
			slave.receive(boost::asio::buffer(buf, 65536));
		} catch (boost::exception &exc) {
			std::cout << boost::diagnostic_information(exc) << std::endl;
			continue;
		}

		if (log_need)
			std::cout << strreq << std::endl;

		slave.send(boost::asio::buffer(response.to_http()));
		if (log_need)
			std::cout << "Sended" << std::endl;*/
	}
	return 0;
}
