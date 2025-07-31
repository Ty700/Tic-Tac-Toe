#include "../../includes/Server.h"

int main()
{
	auto server = std::make_unique<Server>();

	server->run();
}

