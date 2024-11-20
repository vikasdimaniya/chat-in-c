#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 3434
#define BUFFER_SIZE 1024

int receive_message(int client_fd)
{
    printf("receive_message\n");
    while (1)
    {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer
        // Receive message from client
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
        {
            // Client disconnected or an error occurred
            if (bytes_received == 0)
            {
                printf("Client disconnected.\n");
            }
            else
            {
                perror("Error receiving message");
            }
            return -1; // Signal termination
        }

        // Print the received message
        printf("Client: %s\n", buffer);
    }
}
int send_message(int client_fd)
{
    printf("send_message\n");
    while (1)
    {
        char send_buffer[BUFFER_SIZE];

        // Prompt user for input
        fgets(send_buffer, BUFFER_SIZE, stdin);

        // Remove newline character from the input
        send_buffer[strcspn(send_buffer, "\n")] = 0;

        // Check if the user entered an empty message
        if (strlen(send_buffer) == 0)
        {
            return -1; // Exit condition
        }

        // Send the message to the server
        if (send(client_fd, send_buffer, strlen(send_buffer), 0) < 0)
        {
            perror("Error sending message");
            return -1;
        }

    }
}
int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    int opt = 1; // For setting socket options

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR option
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configure server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept client connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connection established with client.\n");

    // Communicate with client
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child process: PID = %d\n", getpid());
        printf("Child process doing its task.\n");
        receive_message(client_fd);
        printf("exit receive_message\n");
    } else {
        // Parent process
        printf("Parent process: PID = %d\n", getpid());
        printf("Waiting for child process to finish...\n");
        printf("Child process finished. Parent resuming.\n");
        send_message(client_fd);
        printf("exit send_message\n");
    }
    
    

    close(client_fd);
    close(server_fd);
    return 0;
}
