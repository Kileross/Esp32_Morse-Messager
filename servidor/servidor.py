import socket
import threading

# Diccionario para almacenar las conexiones de clientes por nombre
clients = {}

# Función para manejar la comunicación con cada cliente
def handle_client(client_socket, client_address, server_name):
    print(f"Conexión establecida con {client_address}")

    # Registra el cliente en el servidor
    clients[server_name] = (client_socket, client_address)

    while True:
        try:
            # Recibe el mensaje del cliente
            message = client_socket.recv(1024).decode('utf-8')
            if not message:
                break

            print(f"Mensaje de {server_name}: {message}")

            # Envía el mensaje al destinatario
            parts = message.split(":", 1)
            if len(parts) == 2:
                recipient = parts[0].strip()
                text_message = parts[1].strip()
                if recipient in clients:
                    recipient_socket, _ = clients[recipient]
                    recipient_socket.send(f"{server_name}:{text_message}".encode('utf-8'))
                else:
                    client_socket.send(f"Error: 100-{recipient}".encode('utf-8')) #Usuario no encontrado
        except ConnectionResetError:
            break

    # Elimina al cliente de la lista al desconectarse
    print(f"{server_name} se ha desconectado.")
    del clients[server_name]
    client_socket.close()

# Configura el servidor
def start_server():
    server_ip = '0.0.0.0'
    server_port = 12500
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((server_ip, server_port))
    server.listen(5)
    print(f"Servidor escuchando en {server_ip}:{server_port}...")

    try:
        while True:
            client_socket, client_address = server.accept()
            server_name = client_socket.recv(1024).decode('utf-8')
            threading.Thread(target=handle_client, args=(client_socket, client_address, server_name)).start()
    except KeyboardInterrupt:
        print("Servidor detenido manualmente.")
    finally:
        server.close()

if __name__ == "__main__":
    start_server()