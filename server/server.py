# Program to control passerelle between Android application
# and micro-controller through USB tty
import socketserver
import serial
import threading
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
import json
import socket
import time


HOST = "0.0.0.0"
UDP_PORT = 10000
MICRO_COMMANDS = ["TL", "LT"]
LAST_VALUE = ""
ANDROID_CLIENT_IP = "172.20.10.3"

# INFLUX DB CLIENT
INFLUX_BUCKET = "serverbucket"
INFLUX_ORG = "server"
INFLUX_TOKEN = "ccbd189d7c8090282f0d558a5cb02163ce691803be748c80ad0fcacbdfff3eb6"
INFLUX_URL = "http://localhost:8086"


def sendUDPMessage(msg, ip, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(bytes(msg.encode('utf-8')), (ip, port))
        print(f"Message <{msg}> sent to {ip}:{port}")
    except Exception as e:
        print(f"Error sending UDP message: {e}")
    finally:
        sock.close()


class ThreadedUDPRequestHandler(socketserver.BaseRequestHandler):

    def handle(self):
        data = self.request[0].strip()
        decodedData = json.loads(data)
        messageContent = decodedData["message"]
        currentThread = threading.current_thread()
        print("{}: client: {}, wrote: {}".format(
            currentThread.name, self.client_address, messageContent))
        if messageContent != "":
            if messageContent in MICRO_COMMANDS:  # Send message through UART
                json_message = json.dumps({"message": messageContent})
                sendUARTMessage(json_message)
            elif messageContent == "getValues()":  # Send last message to Android Client
                json_last_value = json.dumps(
                    {"message": LAST_VALUE})
                sendUDPMessage(json_last_value, ANDROID_CLIENT_IP, 10001)

            else:
                print("Unknown message: ", messageContent)


class ThreadedUDPServer(socketserver.ThreadingMixIn, socketserver.UDPServer):
    pass


# Send serial message
SERIALPORT = "/dev/tty.usbmodem1202"

BAUDRATE = 115200
ser = serial.Serial()


def initUART():
    # ser = serial.Serial(SERIALPORT, BAUDRATE)
    ser.port = SERIALPORT
    ser.baudrate = BAUDRATE
    ser.bytesize = serial.EIGHTBITS  # number of bits per bytes
    ser.parity = serial.PARITY_NONE  # set parity check: no parity
    ser.stopbits = serial.STOPBITS_ONE  # number of stop bits
    ser.timeout = None  # block read

    # ser.timeout = 0             #non-block read
    # ser.timeout = 2              #timeout block read
    ser.xonxoff = False  # disable software flow control
    ser.rtscts = False  # disable hardware (RTS/CTS) flow control
    ser.dsrdtr = False  # disable hardware (DSR/DTR) flow control
    # ser.writeTimeout = 0     #timeout for write
    print("Starting Up Serial Monitor")
    try:
        ser.open()
    except serial.SerialException:
        print("Serial {} port not available".format(SERIALPORT))
        exit()


def sendUARTMessage(msg):
    msg_bytes = msg.encode('utf-8')
    ser.write(msg_bytes)
    print("Message <{}> sent to micro-controller.".format(msg))


# Main program logic follows:
if __name__ == '__main__':
    initUART()
    client = influxdb_client.InfluxDBClient(
        url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
    writeApi = client.write_api(write_options=SYNCHRONOUS)
    print('Press Ctrl-C to quit.')

    server = ThreadedUDPServer((HOST, UDP_PORT), ThreadedUDPRequestHandler)
    serverThread = threading.Thread(target=server.serve_forever)
    serverThread.daemon = True
    try:
        serverThread.start()
        print("Server started at {} port {}".format(HOST, UDP_PORT))
        while ser.isOpen():
            if (ser.inWaiting() > 0):
                data = ser.readline()
                # json_str = data.decode('utf-8')
                dataDecoded = json.loads(data)
                LAST_VALUE = dataDecoded
                temperature = dataDecoded["t"]
                luminosite = dataDecoded["l"]
                onePoint = influxdb_client.Point("messages").field(
                    "temperature", temperature).field("luminosite", luminosite)
                writeApi.write(bucket=INFLUX_BUCKET,
                               org=INFLUX_ORG, record=onePoint)
    except (KeyboardInterrupt, SystemExit):
        server.shutdown()
        server.server_close()
        ser.close()
        exit()
