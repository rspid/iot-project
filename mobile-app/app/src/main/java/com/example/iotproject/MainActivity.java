package com.example.iotproject;

import androidx.appcompat.app.AppCompatActivity;

import android.app.AsyncNotedAppOp;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public class MainActivity extends AppCompatActivity {

    private String ip;
    private String address;
    private DatagramSocket UDPSocket;
    private UdpThread thread;
    private ArrayBlockingQueue<String> arrayBlockingQueue = new ArrayBlockingQueue<>(10);
    private ArrayBlockingQueue<String> arrayReceivedQueue = new ArrayBlockingQueue<>(10);


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Switch sw1 = findViewById(R.id.switch1);

        TextView lblV1 = findViewById(R.id.labelV1);
        TextView lblV2 = findViewById(R.id.labelV2);

        EditText ipAddress = findViewById(R.id.ipAddress);


        Button getVal = findViewById(R.id.getValues);


        sw1.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                address = ipAddress.getText().toString();
                if(isChecked){
                    lblV1.setText("Température");
                    lblV2.setText("Luminosité");
                    send("{\"message\":\"TL\"}");
                }else{
                    lblV1.setText("Luminosité");
                    lblV2.setText("Température");
                    send("{\"message\":\"LT\"}");
                }
            }
        });



        getVal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                address = ipAddress.getText().toString();
                UDPThreadReceived threadReceived = new UDPThreadReceived(arrayReceivedQueue, address, "10001");
                threadReceived.start();
                send("{\"message\":\"getValues()\"}");
                try {
                    String message = arrayReceivedQueue.take();
                    System.out.println(message);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        });
    }

    private void send(String message){
        arrayBlockingQueue.add(message);
        UdpThread thread = new UdpThread(arrayBlockingQueue, address, "10000");
        thread.start();
    }
}