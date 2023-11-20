package com.example.iotproject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.concurrent.BlockingQueue;

public class UdpThread extends Thread {
    private BlockingQueue<String> queue;
    private String ip;
    private String port;

    public UdpThread(BlockingQueue<String> queue, String ip, String port) {
        this.queue = queue;
        this.ip = ip;
        this.port = port;
    }

    @Override
    public void run() {
        try {
            String message = queue.take();
            this.send(message);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void send(String message){
        try {
            DatagramSocket UDPSocket = new DatagramSocket();
            InetAddress address = InetAddress.getByName(this.ip);
            byte[] data = message.getBytes();
            DatagramPacket packet = new DatagramPacket(data, data.length, address, Integer.parseInt(this.port));
            UDPSocket.send(packet);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}