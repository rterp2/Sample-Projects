from socket import *
import os

# Function to list files in the server's directory
def list_files():
    files = os.listdir()
    return "\n".join(files)

# Function to handle client requests
def handle_client_request(connectionSocket):
    while True:
        try:
            command = connectionSocket.recv(1024).decode().lower()
            if command.startswith("list"):
                file_list = list_files()
                connectionSocket.send(file_list.encode())
            elif command.startswith("retrieve"):
                filename = command.split()[1]
                if os.path.exists(filename):
                    connectionSocket.send("File found".encode())
                    with open(filename, 'rb') as f:
                        data = f.read(1024)
                        while data:
                            connectionSocket.send(data)
                            data = f.read(1024)
                    connectionSocket.send(b"EOF")  # Indicate end of file
                else:
                    connectionSocket.send("File not found".encode())
            elif command.startswith("store"):
                filename = command.split()[1]
                with open(filename, 'wb') as f:
                    while True:
                        data = connectionSocket.recv(1024)
                        if data == b"EOF":
                            break  # End of file transfer
                        f.write(data)
                print(f"File {filename} stored.")
                connectionSocket.send("File stored".encode())  # Confirm file stored
            elif command.startswith("quit"):
                print("Client requested to terminate connection.")
                break
            else:
                connectionSocket.send("Invalid command".encode())
        except:
            print("Connection closed by client.")
            break

    connectionSocket.close()  # Close the connection socket
    print("Connection closed.")

# Main function
def main():
    serverPort = 12000
    serverSocket = socket(AF_INET, SOCK_STREAM)
    serverSocket.bind(("", serverPort))
    serverSocket.listen(1)
    print("FTP Server is ready to receive...")

    while True:
        connectionSocket, addr = serverSocket.accept()
        print("Connected to client:", addr)
        handle_client_request(connectionSocket)
        break  # Exit the loop after handling client request

    serverSocket.close()  # Close the server socket
    print("Server closed.")

if __name__ == "__main__":
    main()