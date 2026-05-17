#include <iostream>
#include <string>
#include "../../includes/Server.h"

static void printUsage(const char* progName)
{
	std::cout
		<< "Usage: " << progName << " [-h | --help]\n"
		<< "\n"
		<< "TicTacToe network server. Listens on :8085 and serves the web UI,\n"
		<< "JSON API, and live game sessions for the GTK/iOS/web clients.\n"
		<< "\n"
		<< "Options:\n"
		<< "  -h, --help    Print this message and exit\n"
		<< "\n"
		<< "All other arguments are ignored. The server reads no config\n"
		<< "from argv; port, ID space, and TTLs are compiled in via\n"
		<< "includes/Server.h.\n";
}

int main(int argc, char* argv[])
{
	/* Parse flags before constructing Server. --help exits 0 without
	 * binding; unknown flags are warned-about but still allowed for
	 * back-compat with harnesses that pass extra args. */
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "-h" || arg == "--help")
		{
			printUsage(argv[0]);
			return 0;
		}
		std::cerr << "[SERVER] warning: ignoring unknown argument '"
		          << arg << "'" << std::endl;
	}

	auto server = std::make_unique<Server>();

	return server->run();
}

