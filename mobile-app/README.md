# Documentation technique IOTApp :

## Sommaire :
- Choix d'affichage
- Définir le serveur de destination
- Communication bidirectionnelle avec le Smartphone



### Choix d'affichage
Nous avons fais le choix de changer l'affichage des données à l'aide d'un switch présent nattivement en Androïd. Nous manipulons les données avec graçe à son eventListener suivant en envoyant au serveur des messages au format JSON : 

```java
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
```

On peut voir la présence d'un appel à une fonction **send()**. Cette fonction va permettre de créer les ***Threads d'envoies*** au serveur. Voici ci-dessous le détail de la fonction **send()**.

```java
private void send(String message){
    arrayBlockingQueue.add(message);
    UdpThread thread = new UdpThread(arrayBlockingQueue, address, "10000");
    thread.start();
}
```

Cette fonction va créer un **UDPThread** qui va lui de son côté s'occuper de la création du packet UDP ainsi que de son envoie.


### Définir le serveur de destination

L'application permet de choisir l'IP du serveur sur lequel on souhaite pointer (nous avons fait le choix de laisser par défaut le port 10000 dans le code pour des raisons de simplicité).

Nous avons donc définie un EditText que nous récupérons et mettons dans une variable que l'on pourra utiliser pour appeler la création de Thread d'envoie de packet UDP :

```java
//Identification de l'input éditable
EditText ipAddress = findViewById(R.id.ipAddress);

//Récupération de l'adresse IP contenue dans l'input
address = ipAddress.getText().toString();
```

### Communication bidirectionnelle avec le Smartphone
Pour gérer la communication bidirectionnelle nous avons donc fait le choix de créer **deux Threads**, un d'envoie et un autre de réception.

Le premier Thread, celui d'envoie est celui que nous avons présenté plus haut. Il ne s'occupe que de la création du packet UDP ainsi que de son envoie via la fonction send() de celui-ci: 

```java
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
```


Le second **Thread**, celui de réception va s'occuper de la réception de donnée qui sont émise depuis le serveur faisant la passerelle avec les microbits. Voici le détail de la classe :

```java
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
```