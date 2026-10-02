#include "udp_socket.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace matched_filter {

UDPClient::UDPClient(const ConfigFile& cfg) {
	// Create IPv4 UDP socket
	sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd_ < 0) {
		spdlog::error("Failed to create socket");
	}

	// Fill out server information
	struct sockaddr_in servaddr, cliaddr;

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family = AF_INET;	// IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(cfg.port);

	// Bind socket to server
	int ret = bind(sockfd_, (const struct sockaddr*)&servaddr, sizeof(servaddr));
	if (ret < 0) {
		spdlog::error("Socket bind failed");
	}

	spdlog::info("UDP Client initialized");
}

UDPClient::~UDPClient() {
	if (sockfd_ >= 0) {
		close(sockfd_);
		spdlog::info("Close UDP socket");
	}
}

void UDPClient::ReadMessage() {
	char buffer[1024];
	ssize_t n = recv(sockfd_, buffer, sizeof(buffer) - 1, 0);

	if (n < 0) {
		spdlog::info("No data received");
	}
}

}  // namespace matched_filter