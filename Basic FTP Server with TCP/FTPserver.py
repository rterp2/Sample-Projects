from socket import *

# Function to connect to the server
def connect_to_server(serverName, serverPort):
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverName, serverPort))
    return clientSocket

# Function to list files on the server
def list_files(clientSocket):
    clientSocket.send("list".encode())
    response = clientSocket.recv(1024).decode()
    print("List of files on server:")
    print(response)

# Function to retrieve a file from the server
def retrieve_file(clientSocket, filename):
    clientSocket.send(f"retrieve {filename}".encode())
    response = clientSocket.recv(1024).decode()
    if response == "File found":
        with open(filename, 'wb') as f:
            while True:
                data = clientSocket.recv(1024)
                if data == b"EOF":
                    break  # End of file transfer
                f.write(data)
        print(f"File {filename} retrieved successfully.")
    else:
        print(response)

# Function to store a file on the server
def store_file(clientSocket, filename):
    try:
        with open(filename, 'rb') as f:
            clientSocket.send(f"store {filename}".encode())
            data = f.read(1024)
            while data:
                clientSocket.send(data)
                data = f.read(1024)
        clientSocket.send(b"EOF")  # Indicate end of file transfer
        response = clientSocket.recv(1024).decode()
        print(response)
    except FileNotFoundError:
        print(f"File {filename} not found.")

# Function to terminate the connection
def quit_connection(clientSocket):
    clientSocket.send("quit".encode())
    clientSocket.close()
    print("Connection terminated.")

# Main function
def main():
    serverName = input("Enter server name or IP address: ")
    serverPort = int(input("Enter server port: "))
    clientSocket = connect_to_server(serverName, serverPort)

    while True:
        command = input("Enter command (LIST, RETRIEVE <filename>, STORE <filename>, QUIT): ").lower().split()
        
        if not command:
            print("Please enter a command.")
            continue
        
        if command[0] == "list":
            list_files(clientSocket)
        elif command[0] == "retrieve":
            if len(command) < 2:
                print("Please provide valid filename.")
                continue
            retrieve_file(clientSocket, command[1])
        elif command[0] == "store":
            if len(command) < 2:
                print("Please provide valid filename.")
                continue
            store_file(clientSocket, command[1])
        elif command[0] == "quit":
            quit_connection(clientSocket)
            break
        else:
            print("Invalid command. Please try again.")

if __name__ == "__main__":
    main()