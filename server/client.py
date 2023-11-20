import socket
import time
import json

UDP_IP = "127.0.0.1"  # Adresse IP du serveur
UDP_PORT = 10001       # Port UDP du serveur

# Créer un socket UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

try:
    while True:
        message = input("Entrez un message (TL ou LT) : ")

        message_json = json.dumps({"message": message})

        # Envoyer le message au serveur
        sock.sendto(message_json.encode('utf-8'), (UDP_IP, UDP_PORT))

        # Attendre un court instant avant d'envoyer le prochain message
        time.sleep(1)

except KeyboardInterrupt:
    print("Arrêt du client UDP.")
finally:
    # Fermer le socket lorsque vous avez terminé
    sock.close()
