  #include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <string.h>

//Bluetooth UART



// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

uint8_t txValue = 0;
char str[500];

// Fotosensor
bool flagVelocidade = false;
float dist = 5.0;

// Generate Velocity
uint8_t velToGenerate = 0;
bool flagGenerate = true;
int generatedTime1 = 0;
int timeNeededToGenerate = 0;


//-----------------------PROTÓTIPOS------------------------//

void enviarSerialBle(char *strToSend);
void IRAM_ATTR isr1();
void IRAM_ATTR isr2();
void enviarVelocidade();
int calculaVelocidade(float dist, int time);

//---------------------------------------------------------//





class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


// Struct de interrupção do botão
struct interrupcaoExterna {
  const uint8_t PIN;
  int microTime;
};

// Interrupção externa no pino 18
interrupcaoExterna interr1 = {18, 0};
interrupcaoExterna interr2 = {19, 0};


int generateTime(int vel){
  return abs(int((dist * 1000000 * 3.6) / (vel)));
}


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        strcpy(str, rxValue.c_str());
        Serial.print("Valor Recebido: ");
        Serial.println(str);



        if (strcmp(str, "ON\r\n") == 0) {
          digitalWrite(LED_BUILTIN, HIGH);//Faz o LED piscar (inverte o estado).
        }
        else if (strcmp(str, "OFF\r\n")  == 0) {
          digitalWrite(LED_BUILTIN, LOW);//Faz o LED piscar (inverte o estado).
        } else if (strcmp(str, "CMD1\r\n") == 0) {
          isr1();
        } else if (strcmp(str, "CMD2\r\n") == 0) {
          isr2();
        } else if (strcmp(str, "CMD3\r\n") == 0) {
          isr1();
          delayMicroseconds(generateTime(random(1,200)));
          isr2();
        }
        else {
          
          flagGenerate = 1;
          generatedTime1 = 0;

          try {
            char aux[100];
            float auxFloatVext[3];
            int count = 0;


            strcpy(aux, rxValue.c_str());
            aux[strlen(aux) - 2] = '\0';

            char * pch;
            printf ("Splitting string \"%s\" into tokens:\n", aux);
            pch = strtok (aux, ",");
            float distAux = 0;
            while (pch != NULL)
            {
              if (count == 0) {
                distAux = ::atof(pch);
              }
              else {
                int velAux = velToGenerate;
                velToGenerate = int(::atof(pch));
                sprintf(str, "A velocidade gerada foi mudada de %d para %d.", velAux, velToGenerate);
                enviarSerialBle(str);
                
                
              }
              count++;
              pch = strtok (NULL, ",");
            }


            if (int(distAux) == 0) {
              sprintf(str, "A distancia inserida(%s) não é válida.", aux);
              enviarSerialBle(str);
              enviarSerialBle(str);


            } else if (distAux >= 3 && distAux <= 5 ) {
              sprintf(str, "A distancia foi mudada de %.2f para %.2f.", dist, distAux);
              enviarSerialBle(str);
              Serial.println(str);
              dist = distAux;
              timeNeededToGenerate = abs(int((dist * 1000000 * 3.6) / (velToGenerate)));

            } else {
              sprintf(str, "A distancia inserida(%s) deve estar entre 3 e 5 metros.", aux);
              enviarSerialBle(str);
            }
          } catch (...) {
            char aux[100];
            strcpy(aux, rxValue.c_str());
            aux[strlen(aux) - 2] = '\0';
            sprintf(str, "A distancia inserida(%s) não é válida.", aux);
            enviarSerialBle(str);
          }
        }


        Serial.println("*********");
      }
    }
};





void enviarSerialBle(char *strToSend) {
  Serial.print("Enviando dado bluetooth");
  strcat(strToSend, "\r\n");
  Serial.print(strToSend);


  pTxCharacteristic->setValue(strToSend);
  pTxCharacteristic->notify();
}

//---------------------------------------------------------//
void IRAM_ATTR isr1() {
  digitalWrite(LED_BUILTIN, LOW);//Faz o LED piscar (inverte o estado).
  (interr1.microTime = micros());
}
//---------------------------------------------------------//


//---------------------------------------------------------//
void IRAM_ATTR isr2() {

  digitalWrite(LED_BUILTIN, HIGH);//Faz o LED piscar (inverte o estado).
  (interr2.microTime = micros());
  if (interr1.microTime != 0)
    flagVelocidade = true;

}
//---------------------------------------------------------//




//---------------------------------------------------------//
void enviarVelocidade() {
  sprintf(str, "%d", calculaVelocidade(dist, interr2.microTime - interr1.microTime));

  enviarSerialBle(str);
  interr1.microTime = 0;
  interr2.microTime = 0;
  flagVelocidade = false;
}
//---------------------------------------------------------//


//---------------------------------------------------------//
int calculaVelocidade(float dist, int time_) {
  Serial.print("Distancia:");
  Serial.println(dist);

  Serial.print("Tempo:");
  Serial.println(time_);

  Serial.print("Velocidade:");
  Serial.println((dist * 1000000 / abs(time_) * 3.6));

  return round((dist * 1000000 / abs(time_) * 3.6));
}
//---------------------------------------------------------//




void setup() {
  Serial.begin(115200);



  pinMode(LED_BUILTIN, OUTPUT);//Habilita o LED onboard como saída.
  pinMode(interr1.PIN, INPUT_PULLUP);
  pinMode(interr2.PIN, INPUT_PULLUP);

  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);


  attachInterrupt(interr1.PIN, isr1, RISING);
  attachInterrupt(interr2.PIN, isr2, RISING);

  // Create the BLE Device
  BLEDevice::init("ESP32 Nivardo UART");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE
                                          );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}



void loop() {

 

  if (velToGenerate > 0) {
    if (flagGenerate) {
      delay(50);
      generatedTime1 = micros();
      flagGenerate = false;
      digitalWrite(33, HIGH);
      delay(1);
      digitalWrite(33, LOW);

    } else if (micros() - generatedTime1 >= timeNeededToGenerate) {

      digitalWrite(32, HIGH);//Faz o LED piscar (inverte o estado).
      delay(1);
      digitalWrite(32, LOW);//Faz o LED piscar (inverte o estado).
      Serial.println(micros() - generatedTime1);
      flagGenerate = true;

    }
  }
  
  if (flagVelocidade) {
    enviarVelocidade();
  }


  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}
