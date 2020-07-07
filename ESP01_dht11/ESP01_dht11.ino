#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define L1 2
#define DHTPIN 0
#define DHTTYPE DHT11


float temperature;
float umidity;
long lastMsg = 0;

//..........Dados da Rede......................................................//
const char* ssid = "nome da rede";
const char* password = "senha";


//....informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT


const char* mqttServer = "servidorMQTT";   //server
const char* mqttUser = "";                     //user
const char* mqttPassword = "";                //password
const int mqttPort = 1883;                     //port


//.........Tópicos mqtt que serão recebidos e enviados........................................//

const char* mqttTopicSub1 = "topic001";           //tópico que sera assinado botão
const char* mqttTopicPubtemp = "topictmp";     //tópico que sera assinado temperatura
const char* mqttTopicPubumid = "topicumd";     //tópico que sera assinado temperatura
//const char* mqttTopicPubtemp ="topictmp";      //tópico que sera assinado umidade


//....................................................................................................//

WiFiClient EspClientSampaio;
PubSubClient client(EspClientSampaio);

DHT dht(DHTPIN, DHTTYPE);


void setup()
{
  Serial.begin(9600);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

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

  ArduinoOTA.onStart([]() {
     Serial.println("Iniciando OTA");
    });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) {

  //armazena msg recebida em uma sring
  payload[length] = '\0';
  String strMSG = String((char*)payload);

  //Serial.print("Mensagem mqtt: ");
  Serial.print(topic);
  Serial.print("/");
  Serial.println(strMSG);

  //aciona saída conforme msg recebida
  if (strMSG == "1") {        //se msg "1"
    digitalWrite(L1, LOW);  //coloca saída em LOW para ligar a Lampada - > o módulo RELE usado tem acionamento invertido. Se necessário ajuste para o seu modulo
  } else if (strMSG == "0") {  //se msg "0"
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

    if (conectado) {

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

  ArduinoOTA.handle();


  if (!client.connected()) {
    reconect();
  }
  client.loop();


  // Publica o valor de temperatura e umidade a cada 10 segundos
  long now = millis();
  if (now - lastMsg > 5000) {
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
