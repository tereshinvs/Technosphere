#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp> 
#include <boost/exception_ptr.hpp>

#include <string>
#include <iostream>

#include "Handler.hpp"
#include "Request.hpp"
#include "Response.hpp"

#include "test.cpp"

int main(int argc, char *argv[]) {
	std::string root_path = "./";
	unsigned http_port = 8080;
	bool log_need = false;
	bool help_need = false;

	boost::program_options::options_description opts("Options");
	opts.add_options()
		("root,r", boost::program_options::value<std::string>(), "Root")
		("port,p", boost::program_options::value<unsigned>(), "Port")
		("log,l", "Log")
		("help,h", "Help");

	boost::program_options::variables_map vm;
	try {
		boost::program_options::parsed_options parsed =
			boost::program_options::command_line_parser(argc, argv).
				options(opts).allow_unregistered().run();
		boost::program_options::store(parsed, vm);

		if (vm.count("root"))
			root_path = vm["root"].as<std::string>();
		if (vm.count("port"))
			http_port = vm["port"].as<unsigned>();
		if (vm.count("log"))
			log_need = true;
		if (vm.count("help"))
			help_need = true;
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return 0;
	}

	http_server::Handler handler(root_path);
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), http_port);
	boost::asio::ip::tcp::acceptor acceptor(io_service, ep);
	acceptor.listen();
	while (true) {
		boost::asio::ip::tcp::socket slave(io_service);
		acceptor.accept(slave);
		if (log_need)
			std::cout << "Accepted" << std::endl;

		char buf[65536] = {0};
		try {
			slave.receive(boost::asio::buffer(buf, 65536));
		} catch (boost::exception &exc) {
			std::cout << boost::diagnostic_information(exc) << std::endl;
			continue;
		}

		std::string strreq(buf);
		if (log_need)
			std::cout << strreq << std::endl;

		http_server::Request request(strreq);
		if (log_need) {
			std::cout << "********************************" << std::endl;
			std::cout << request.to_http() << std::endl;
			std::cout << "********************************" << std::endl;
		}

		http_server::Response response = handler.handle(request);
		if (log_need)
			std::cout << "Response:\n" << response.to_http() << std::endl;

		slave.send(boost::asio::buffer(response.to_http()));
		if (log_need)
			std::cout << "Sended" << std::endl;
	}
	return 0;
}
