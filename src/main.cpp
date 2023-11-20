/**
 * @file main.cpp
 * @brief LDAP server entry point and parallel tcp server implementation
 * @author Simon Bencik <xbenci01>
 */
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include "../include/message.h"

#define PORT 389
#define BUFFER_SIZE 32768 // 32KB

// Gracefully handle SIGINT and SIGTERM
int sockfd;
void signalHandler(int signum) {
  // Cleanup and close up stuff here
  close(sockfd);

  std::cout << "Terminating..." << std::endl;

  // Kill all children
  kill(0, SIGTERM);

  // Wait for all children to terminate
  while (waitpid(-1, NULL, 0) > 0) {
  }

  // Terminate program
  exit(signum);
}

int main(int argc, char *argv[]) {
  // Set up signal handler
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  std::string inputFile;
  int port = PORT;

  // Parse args
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-p" && i + 1 < argc) {
      port = std::stoi(argv[i + 1]);
    } else if (arg == "-f" && i + 1 < argc) {
      inputFile = argv[i + 1];
    }
  }

  // Check if input file is set
  if (inputFile.empty()) {
    std::cerr << "Error: Input file not set" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Create socket and check for errors
  sockfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "Error: Failed to create socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Create server address
  sockaddr_in6 servAddr;
  servAddr.sin6_family = AF_INET6;
  servAddr.sin6_port = htons(port);
  servAddr.sin6_addr = in6addr_any;

  // Bind socket to address
  if (bind(sockfd, (sockaddr *)&servAddr, sizeof(servAddr)) == -1) {
    std::cerr << "Error: Failed to bind socket" << std::endl;
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // Listen for connections
  if (listen(sockfd, 50) == -1) {
    std::cerr << "Error: Failed to listen for connections" << std::endl;
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  std::cout << "Listening on port " << port << std::endl;

  // Client address
  sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];

  int pid;

  // Parallel server
  while (1) {
    // Accept connection
    int clientSockfd = accept(sockfd, (sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSockfd == -1) {
      std::cerr << "Error: Failed to accept connection" << std::endl;
      close(sockfd);
      exit(EXIT_FAILURE);
    }

    if ((pid = fork()) > 0) {
      close(clientSockfd);
    } else if (pid == 0) {
      close(sockfd);

      // Get client info
      int result = getnameinfo((sockaddr *)&clientAddr, clientAddrLen, host,
                               NI_MAXHOST, service, NI_MAXSERV, 0);
      if (result) {
        std::cerr << "Error: Failed to get client info" << std::endl;
        close(clientSockfd);
        exit(EXIT_FAILURE);
      }

      std::cout << "Connection from " << host << ":" << service << std::endl;

      // Parse requests
      while (1) {
        // Read
        std::vector<unsigned char> buffer(BUFFER_SIZE);
        int bytesReceived = recv(clientSockfd, buffer.data(), BUFFER_SIZE, 0);

        // Check for errors
        if (bytesReceived == -1) {
          std::cerr << "Error: Failed to read request" << std::endl;
          close(clientSockfd);
          exit(EXIT_FAILURE);
        } else if (bytesReceived == 0) {
          std::cout << "Client disconnected" << std::endl;
          break;
        }

        // Resize buffer to actual size
        buffer.resize(bytesReceived);

        // Using polymorphism to determine the type of request
        auto ldapRequest = createLDAPRequest(buffer);
        ldapRequest->parse();
        ldapRequest->respond(clientSockfd, inputFile);

        // If ldaprequest is nullptr, it is not supported, close connection
        if (ldapRequest == nullptr) {
          std::cout << "Unsupported request received" << std::endl;
          close(clientSockfd);
          break;
        }

        // Check if request is instance of Unbind
        if (dynamic_cast<Unbind *>(ldapRequest.get())) {
          std::cout << "Unbind request received" << std::endl;
          close(clientSockfd);
          break;
        }
      }

      close(clientSockfd);
      exit(0);
    } else {
      std::cerr << "Error: Failed to fork" << std::endl;
      close(sockfd);
      exit(EXIT_FAILURE);
    }
  }
}
