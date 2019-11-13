const int LED = LED_BUILTIN; 
//limite para ativar o LED
const int LIMIT = 3;
 
void setup() {
  Serial.begin(115200);  
  //configura o pino do LED como saída
  pinMode(LED, OUTPUT); 
}
void loop() { 
   //guarda o valor lido do sensor hall
   int measurement = 0; 
   //faz a leitura do sensor hall
   measurement = hallRead();
   Serial.print("Imprime a medida: ");
   Serial.println(measurement);
   //verifica se o valor lido é menor que o limite definido 
   if(measurement < LIMIT) {
      //liga o LED
      digitalWrite(LED, HIGH);
   }
   else   {
      //desliga o LED
      digitalWrite(LED, LOW);
   }
   delay(100);

}
