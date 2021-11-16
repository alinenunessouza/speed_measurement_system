//#include <TonePlayer.h>
#include <LiquidCrystal.h>
#include <NewPing.h>

//TonePlayer tone1 (TCCR1A, TCCR1B, OCR1AH, OCR1AL, TCNT1H, TCNT1L); 

#define SONAR_NUM 2                         // Numero de sensores
#define DISTANCIA_SENSORES 30               // Distância entre os dois sensores de 30 cm
#define DISTANCIA_MINIMA 2                  // Distância mínima para iniciar a medição em 2 cm (para teste)
#define DISTANCIA_MAXIMA 35                 // Distância máxima de alcance da medição em 35 cm (para teste)
#define DISTANCIA_MAXIMA_ALCANCE 200        // Maximum distance (in cm) to ping.
#define VELOCIDADE_MAXIMA 50

// Milisegundos de intervalo entre medicoes (29ms e o tempo mínimo para evitar conflito entre os sensores)  
#define PING_INTERVAL 33                    // Cada sensor é atuado em intervalos de 33ms

// Armazena a quantidade de vezes que a medicao deve ocorrer,para cada sensor  
unsigned long pingTimer[SONAR_NUM]; 
uint8_t currentSensor = 0;                  // Armazena o sensor que esta ativo  
float t[SONAR_NUM];                         // Armazena o tempo das medições
unsigned int cm[SONAR_NUM];                 // Armazena o numero de medicoes
float velocidadeMedia;

int iLed = 2;
int iStatus;
int iStatusLed;

int pinoSpeaker = 9;

// pinos que serão utilizados pelo displays
LiquidCrystal display(8, 3, 10, 11, 12, 13);

NewPing sonar[SONAR_NUM] = {    // Sensor object array.
  //NewPing(Pino_Trigger, Pino_Echo, Distancia_Maxima)
  NewPing(4, 5, DISTANCIA_MAXIMA_ALCANCE),  // Each sensor's trigger pin, echo pin, and max distance to ping. 
  NewPing(7, 6, DISTANCIA_MAXIMA_ALCANCE)
};

void setup() {
  Serial.begin(115200);         // Open serial monitor at 115200 baud to see ping results

  configurarPinosSensor();

  configurarPinosLCD();
  
  //pinMode (pinoSpeaker, OUTPUT);
  
  pinMode(iLed, OUTPUT);
}

void configurarPinosLCD(){
  //inicializa o display definindo o número de colunas e linhas com o comando
  display.begin(16, 2);
}

void configurarPinosSensor(){
  pingTimer[0] = millis() + 75; // Primeira medicao começa com 75ms  
  //Define o tempo de inicializacao de cada sensor
  //Vai adicionar um delay de 33ms ao sensor 2
  for (uint8_t i = 1; i < SONAR_NUM; i++)
   pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
}

void loop() {
  
  comutarLed();
  
  // Loop entre todos os sensores 
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    
   if (millis() >= pingTimer[i]) {

    //Define o tempo que o próximo sensor será acionado
    pingTimer[i] += PING_INTERVAL * SONAR_NUM;  

    // Ciclo do sensor completo  
    if (i == 0 && currentSensor == SONAR_NUM - 1){
      calcularVelocidade();
    }
    
    // Reseta o timer antes de ler o proximo sensor  
    sonar[currentSensor].timer_stop();     
    
    // Número do sensor sendo acionado
    currentSensor = i;           

    // Se nao houver eco do sensor, seta a distância como zero   
    //cm[currentSensor] = 0;

    sonar[currentSensor].ping_timer(echoCheck);
   }
 }  
}

void comutarLed(){
  if(iStatus == HIGH){
    if(iStatusLed == LOW){
      iStatusLed = HIGH;
      digitalWrite(iLed, HIGH);
    }
  }
  else {
    iStatusLed = LOW;
    digitalWrite(iLed, LOW);
  }
}

void echoCheck() { 
  //Se receber um sinal (eco), calcula a distancia
    
  if (sonar[currentSensor].check_timer())  
  
    //se é o sensor 0, já tem que ter valor no 1
    if(currentSensor == 1 || (currentSensor == 0 && cm[1] > 0)){//VERIFICAR
      
      if((sonar[currentSensor].ping_result / US_ROUNDTRIP_CM)>=DISTANCIA_MINIMA && (sonar[currentSensor].ping_result / US_ROUNDTRIP_CM)<DISTANCIA_MAXIMA){
        cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;  
        t[currentSensor] = millis();
      }
      
    }
 }  

void calcularVelocidade(){
  
   Serial.print("cm[0] : "); 
   Serial.println(cm[0]);
   
   Serial.print("cm[1] : "); 
   Serial.println(cm[1]);

   int z = cm[0] - cm[1];
   
   if(cm[0] >= DISTANCIA_MINIMA && cm[1] >= DISTANCIA_MINIMA && abs(z) <= 10){
    
     Serial.print("cm[0] : "); 
     Serial.println(cm[0]);
     
     Serial.print("cm[1] : "); 
     Serial.println(cm[1]);
     
     float y = sqrt(pow(abs(z), 2) + pow(DISTANCIA_SENSORES, 2));
     float deltaT = t[0] - t[1];
     float velocidadeComSinal = y/abs(deltaT);
     velocidadeMedia = abs(velocidadeComSinal)*1000; //cm/s
    
     Serial.print("t[0] : "); 
     Serial.println(t[0]);
     
     Serial.print("t[1] : "); 
     Serial.println(t[1]);
     
     Serial.print("Velocidae : "); 
     Serial.println(velocidadeMedia);

     if(velocidadeMedia >= VELOCIDADE_MAXIMA){
        analogWrite(pinoSpeaker, 200);//0 a 255 o pwm
        delay(200);
        analogWrite(pinoSpeaker, 0);//0 a 255 o pwm
        //tone1.tone (220);  // 220 Hz
        
        //acende o led
        iStatus = HIGH;
        
        printarStatusAcima();
     }
     else {
        //tone1.noTone ();
        analogWrite(pinoSpeaker, 0); //0 a 255 o pwm
        iStatus = LOW;
        printarStatusOk();
     }
  
     cm[0]=0;
     cm[1]=0;
   }  
}

void printarVelocidade(){
  display.clear();
  
  display.setCursor(0, 0);
  display.print("VEL.: ");
  display.print(velocidadeMedia, 1);
  display.print(" cm/s");
}

void printarStatusOk(){

  printarVelocidade();
  
  display.setCursor(0, 1);
  display.print("OK!");

  delay(250);
}

void printarStatusAcima(){
  printarVelocidade();
  
  display.setCursor(0, 1);
  display.print("ACIMA!");

  delay(250);
}
