#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define L1 2
#define DHTPIN 0
#define DHTTYPE DHT11


float temperature;
float umidity;
long lastMsg = 0;




//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqttServer = "endereço do servidor broker";   //server
const char* mqttUser = "";              //user
const char* mqttPassword = "";      //password
const int mqttPort = 1883;                     //port


//.........Tópicos mqtt que serão recebidos e enviados........................................//

const char* mqttTopicSub1 ="topic001";            //tópico que sera assinado botão
const char* mqttTopicPubtemp ="topictmp";      //tópico que sera assinado temperatura
const char* mqttTopicPubumid ="topicumd";      //tópico que sera assinado temperatura
//const char* mqttTopicPubtemp ="topictmp";      //tópico que sera assinado umidade


//....................................................................................................//

WiFiClient EspClientSampaio;
PubSubClient client(EspClientSampaio);

DHT dht(DHTPIN, DHTTYPE);

 
void setup()
{
  Serial.begin(9600);
  Serial.println();
  delay(1000);

  dht.begin();

  WiFi.begin("SSID", "senha");

  Serial.print("Connecting");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Conectando no servidor mqtt................");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Conectando ao Broker MQTT...");
    
    if (client.connect("EspClientSampaioId", mqttUser, mqttPassword )) {
      Serial.println("Conectado");  
   } else {
      Serial.print("falha estado  ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }


  client.subscribe(mqttTopicSub1);

  
  
  
  pinMode(L1, OUTPUT);


}

void callback(char* topic, byte* payload, unsigned int length) {

  //armazena msg recebida em uma sring
  payload[length] = '\0';
  String strMSG = String((char*)payload);

  //Serial.print("Mensagem mqtt: ");
  Serial.print(topic);
  Serial.print("/");
  Serial.println(strMSG);
  //Serial.print("Mensagem chegou do tópico: ");
  //Serial.println(topic);
  //Serial.print("Mensagem:");
  //Serial.print(strMSG);
  //Serial.println();
  //Serial.println("-----------------------");
 
  //aciona saída conforme msg recebida 
  if (strMSG == "1"){         //se msg "1"
     digitalWrite(L1, LOW);  //coloca saída em LOW para ligar a Lampada - > o módulo RELE usado tem acionamento invertido. Se necessário ajuste para o seu modulo
  }else if (strMSG == "0"){   //se msg "0"
     digitalWrite(L1, HIGH);   //coloca saída em HIGH para desligar a Lampada - > o módulo RELE usado tem acionamento invertido. Se necessário ajuste para o seu modulo
  }
 
}


void reconect() {
  //Enquanto estiver desconectado
  while (!client.connected()) {
    Serial.print("Tentando conectar ao servidor MQTT");
     
    bool conectado = strlen(mqttUser) > 0 ?
                     client.connect("ESP8266Client", mqttUser, mqttPassword) :
                     client.connect("ESP8266Client");

    if(conectado) {
    
      Serial.println("Conectado!");
   
      //subscreve no tópico
      client.subscribe(mqttTopicSub1, 0); //nivel de qualidade: QoS 0


    } else {
    
      Serial.println("Falha durante a conexão.Code: ");
      Serial.println( String(client.state()).c_str());
      Serial.println("Tentando novamente em 10 s");

      //Aguarda 10 segundos 
      delay(10000);
    }
  }
}

void loop() {
  
   if (!client.connected()) {
    reconect();
  }
  client.loop();

 
  // Publica o valor de temperatura e umidade a cada 10 segundos
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
 
  
  temperature = dht.readTemperature(); 
  umidity = dht.readHumidity();
  
  
  char tmpMSG[8];
  dtostrf(temperature, 1, 2, tmpMSG);
  
  char umdMSG[8];
  dtostrf(umidity, 1, 2, umdMSG);

  client.publish(mqttTopicPubtemp, tmpMSG);
  client.publish(mqttTopicPubumid, umdMSG);



  }


  
  
  }
  
  
