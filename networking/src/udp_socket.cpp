#include "udp_socket.h"

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace matched_filter {

UDPClient::UDPClient(const ConfigFile& cfg) {
	// Create IPv4 UDP socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}

	// Fill out server information
	struct sockaddr_in servaddr, cliaddr;

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family = AF_INET;	// IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(cfg.port);

	// Bind socket to server
	int ret = bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr));
	if (ret < 0) {
		perror("Socket bind failed");
		exit(EXIT_FAILURE);
	}
}

}  // namespace matched_filter