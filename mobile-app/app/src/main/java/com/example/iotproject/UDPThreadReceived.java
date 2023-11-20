package com.example.iotproject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.concurrent.BlockingQueue;

public class UDPThreadReceived extends Thread{
        private BlockingQueue<String> queueR;
        private String ip;
        private String port;

        public UDPThreadReceived(BlockingQueue<String> queueR, String ip, String port) {
            this.queueR = queueR;
            this.ip = ip;
            this.port = port;
        }

        @Override
        public void run() {
            while(true){
                    this.receive();
            }
        }

        public void receive() {
            DatagramSocket UDPSocket = null;
            try {
                byte[] data = new byte[1024];
                UDPSocket = new DatagramSocket(Integer.parseInt("10001"));
                DatagramPacket packet = new DatagramPacket(data, data.length);
                System.out.println("Asking for data !");

                UDPSocket.receive(packet);

                String receivedData = new String(packet.getData(), 0, packet.getLength());
                queueR.put(String.valueOf(packet.getData()));

                System.out.println("Données reçues : " + receivedData);

            } catch (IOException | InterruptedException e) {
                e.printStackTrace();
            } finally {
                Thread.currentThread().interrupt();
            }
        }
}
