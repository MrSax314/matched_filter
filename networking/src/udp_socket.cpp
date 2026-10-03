#include "udp_socket.h"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cuda_runtime.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "rp_buffer.h"

namespace matched_filter {

#define RECV_TIMEOUT_S 1
// TODO(as3): Consider moving to config file

UDPClient::UDPClient(const int& port, const size_t max_message_size)
	: max_message_size_(max_message_size) {
	// Allocate pinned memory for incoming messages
	cudaHostAlloc(reinterpret_cast<void**>(&pinned_buffer_), max_message_size,
				  cudaHostAllocDefault);

	// Create IPv4 UDP socket
	sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd_ < 0) {
		spdlog::error("Failed to create socket");
	}

	// Socket options (linux) - support non blocking recv calls
	struct timeval timeout;
	timeout.tv_sec = RECV_TIMEOUT_S;
	timeout.tv_usec = 0;
	setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	// Fill out server information
	struct sockaddr_in servaddr, cliaddr;
	servaddr.sin_family = AF_INET;	// IPv4
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	// Bind socket to server
	int ret = bind(sockfd_, (const struct sockaddr*)&servaddr, sizeof(servaddr));
	if (ret < 0) {
		spdlog::error("Socket bind failed");
	}

	spdlog::info("Initialized UDP socket");
}

UDPClient::~UDPClient() {
	if (sockfd_ >= 0) {
		close(sockfd_);
		spdlog::info("Closed UDP socket");
	}
}

void UDPClient::ReadMessage(RPBuffer& buffer) {
	ssize_t n = recv(sockfd_, pinned_buffer_, max_message_size_, 0);
	if (n < 0) {
		spdlog::info("No data received");
		return;
	} else {
		spdlog::info("Received {0} bytes", n);
	}

	// Validate packet
	auto* header = reinterpret_cast<IqPacketHeader*>(pinned_buffer_);
	if (!IsValidHeader(header, n)) {
		spdlog::error("Invalid header received");
		return;
	}

	// Find which range window data is for
}

bool UDPClient::IsValidHeader(IqPacketHeader* header, ssize_t byte_ct) {
	if (header->magic != kIqMagic) {
		spdlog::warn("Received unauthorized message");
		return false;
	}
	if (header->version != kIqVersion) {
		spdlog::warn("Received message of incorrect version");
		return false;
	}

	if (header->header_size < sizeof(IqPacketHeader)) {
		spdlog::warn("Received message of incorrect version");
		return false;
	}

	if (header->num_samples) return true;
}

}  // namespace matched_filter