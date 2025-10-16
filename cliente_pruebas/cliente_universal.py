import socket
import threading

# Configuración del cliente
SERVER_IP = 'Ip.del.servidor'  # Cambia esto a la IP del servidor
SERVER_PORT = 12500       # Cambia esto si el puerto del servidor es diferente
BUFFER_SIZE = 1024

# Función para recibir mensajes desde el servidor
def receive_messages(client_socket):
    while True:
        try:
            message = client_socket.recv(BUFFER_SIZE).decode('utf-8')
            if message:
                print(f"\n[Servidor]: {message}")
            else:
                print("\n[Servidor]: Conexión cerrada.")
                break
        except (ConnectionResetError, ConnectionAbortedError):
            print("\n[Error]: Conexión perdida con el servidor.")
            break
        except Exception as e:
            print(f"\n[Error inesperado]: {e}")
            break

# Función principal del cliente
def start_client():
    client_name = input("Introduce tu nombre: ").strip()

    if not client_name:
        print("El nombre no puede estar vacío.")
        return

    # Conexión con el servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((SERVER_IP, SERVER_PORT))
        client_socket.send(client_name.encode('utf-8'))
        print(f"Conectado al servidor {SERVER_IP}:{SERVER_PORT} como {client_name}")
    except Exception as e:
        print(f"Error al conectarse al servidor: {e}")
        return

    # Inicia un hilo para recibir mensajes
    threading.Thread(target=receive_messages, args=(client_socket,), daemon=True).start()

    # Ciclo para enviar mensajes
    try:
        while True:
            message = input("\nEscribe tu mensaje (formato: destinatario: mensaje): ").strip()
            if message.lower() == "salir":
                print("Desconectándote del servidor...")
                break
            if ":" not in message:
                print("Error: El mensaje debe tener el formato 'destinatario: mensaje'.")
                continue

            client_socket.send(message.encode('utf-8'))
    except KeyboardInterrupt:
        print("\nDesconexión manual.")
    finally:
        client_socket.close()

if __name__ == "__main__":
    start_client()