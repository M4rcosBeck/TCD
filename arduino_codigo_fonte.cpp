#include <SPI.h>
#include <Ethernet.h> //Carrega as bibliotecas para conexao Ethernet
#include <Ultrasonic.h> //Carrega a biblioteca do sensor ultrassonico
#define pino_trigger 4 //Define o pino trigger do sensor ultrassonico
#define pino_echo 5 //Define o pino echo do sensor ultrassonico

const int tubo = 24; // define a altura da lixeira em cm através da marcação máxima do sensor ultrassonico.

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

EthernetServer server(80); //(port 80 é o padrao para HTTP)
EthernetClient client;
String valor1Send = "LIXEIRA INTELIGENTE";

Ultrasonic ultrasonic(pino_trigger, pino_echo); //Inicializa o sensor nos pinos definidos acima.

void setup() {

  Serial.begin(9600);
  Serial.println("Lendo os dados do sensor...");
  
  // Inicia a conexao Ethernet e o servidor
  Ethernet.begin(mac);

  // Checa se existe um um hardware Ethernet
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield nao foi encontrado.");

    while (true) {
      delay(1);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Cabo Ethernet nao esta conectado.");
  }

  // Inicia o Servidor
  server.begin();
  Serial.print("Servidor conectado em ");
  Serial.println(Ethernet.localIP());
}

/**
* COLETA AS INFORMACOES DO SENSOR ULTRASSOM, EM CM
*/
float coleta() {
  float cmMsec;
  float porcentagem;

  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);

  //Converte a leitura do sensor em porcentagem.
  porcentagem = (int(cmMsec) * 100) / tubo;
  
  //Inverte a porcentagem.
  porcentagem = (100 - porcentagem);

  //Correção da sensibilidade do sensor ultrassom.
  if(porcentagem > 0 && porcentagem < 6){
    porcentagem = 0;
  }else if(porcentagem < 0 || porcentagem > 100){
    porcentagem = 100;
  }

  //Exibe informacoes no serial monitor.
  Serial.print("Distancia em cm: ");
  Serial.print(cmMsec);
  
  delay(1000);
  
  return porcentagem;  
}

/**
* CONEXAO COM A API TAGO E ENVIO DE INFORMACOES
*/
void loop() {
  if (Serial.available()){
    valor1Send = Serial.readString();
  }

  String envio = "";

  if (client.connect("api.tago.io", 80)) {
    Serial.println("CONECTOU");
    envio = "POST /data  HTTP/1.1\n";
    envio += "Host: api.tago.io\n";
    
    envio += String("Device-Token: ") + "b9e6691d-e18f-406b-b0ad-6c663c55d55d" + "\n";
    envio += " ssl: false\n";
    envio += String("Content-Type: ") + "application/json" + "\n";

    float valorA0 = coleta();

    String dados = "[{\"variable\":\"valor1\",\"value\":\"";
    dados += valor1Send;
    dados +=  "\"},{\"variable\":\"valor2\",\"value\":\"";
    dados += String(valorA0) + "\"}]";

    envio += String("Content-Length: ") + String(dados.length()) + "\n";
    envio += "\n";
    envio += dados;
    Serial.println(envio);

    client.print(envio);

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    // Lê todas as linhas replicadas pelo servidor e imprime na Serial.
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
    Serial.println();
    Serial.println("Encerrando conexao.");
    Serial.println();
    Serial.println();

  }
  else {
    Serial.println("Nao conectou!");
  }
}