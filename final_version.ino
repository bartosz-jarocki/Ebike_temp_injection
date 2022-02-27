/*
 * M.Sharonov aka obelix662000
E-bike S12SN controller mod to display motor's temperature
by inserting corresponding value into communication package

https://endless-sphere.com/forums/viewtopic.php?t=73475

I have MAC wheel with LM35 temperature sensor and S12SN controller with LCD3 display. Unfortunately S12SN does not have any input wire fortemperature sensors. Also I could not find a point at the controller's PCB which reads the temperature. Finally I did it with insertion of temperature data into communication packet between controller and LCD. For communication protocol please see my previous post.
Arduino MCU (Mini 328P 16Mhz) measures temperature with ADC, reads communication packet from S12S to LCD3, repack the packet with temperature data and sends it to LCD3 instead of original one.
Schematic is as follows:
Green wire of S12S to LCD3 is cut (Tx wire). Arduino is inserted in between with Rx and Tx pins.
Rx of arduino is connected to this green wire at the S12S side
Tx of arduino is connected to the wire at the LCD3 side
GND of arduino is connected to any black wire (ground)
Vcc of arduino is connected to any 5V red wire (do not connect to LCD's power supply! it has high voltage! 24-48V)
Output of temperature sensor is connected to A0 pin of arduino.

Do not forget to set parameter C8 to 1, which allows temperature reading.


(Vcc 5+)--+-->2.7k--+-->kty81-110-->(GND)
           |         |
           +->100nF--+-----> ADC0 (Analog Port 0)
*/

//#define DEBUG         // Comment this line when DEBUG mode is not needed

#define R_CONST 3240 //resistance of constant resistor in divider
#define CORR 0 //correction value 
#define MEAS_FREQ 10 //measurements/s

const int OCR1A_SET = 16000000 / (MEAS_FREQ*1024) -1;

char SerialInputBuffer[12]; 
uint16_t pos=0;
uint32_t motorTemp=0;
uint8_t crc=0;

//float ADC_read=0;
float R_KTY=1000; //res of kty83

const int AV_nbr = 15;
int ADC_buff[AV_nbr];
int index=0;
float ADC_av=240;


void setup()
{
  Serial.begin(9600);
  
  for (int i=0; i<AV_nbr; i++){
    ADC_buff[i]=240; //dont put "0", or for first 1.5s output will go crazy
  }
  
  cli();
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  
  OCR1A = OCR1A_SET;  // = (16*10^6) / (1*1024) - 1 (must be <65536)
  TCCR1B |= (1 << WGM12);  // turn on CTC mode 
  TCCR1B |= (1 << CS12) | (1 << CS10);  // Set CS10 and CS12 bits for 1024 prescaler 
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
}

void loop()
{
  while (Serial.available() > 0) {
    char inChar = Serial.read();
    if (inChar==65) {  //start of sequence
      pos=0;
    };
    SerialInputBuffer[pos]=inChar;
    pos++;
    if (pos>11)
    {
      pos=0;
      if (SerialInputBuffer[0]==65)  //if synchronized
      {
        crc=0;
        SerialInputBuffer[9]=(int8_t)(motorTemp-15);   //negative and positive allowed
        for(int k=1;k<12;k++)
        {
          if (k!=6) crc^=SerialInputBuffer[k];
        };
        SerialInputBuffer[6]=crc;
        for (int j=0;j<12;j++){
          Serial.print(SerialInputBuffer[j]);
        }
       motorTemp = kty_read();      
      }
      
    }

  }
  
#ifdef DEBUG  //condition compilation of "debug"
 motorTemp = kty_read();
 Serial.print("ADC= ");
 Serial.println(ADC_buff[index]);
 Serial.print("ADCav= ");
 Serial.println(ADC_av);
 Serial.print("R= ");
 Serial.println(R_KTY);
 Serial.print("TEMP= ");
 Serial.println(motorTemp);
 Serial.print("INDEX= ");
 Serial.println(index);
 delay(1000);
#endif    //end of conditional compilation
}

float kty_read(){
   //float U_KTY = ADCread * (ACDREF/100)/1023,0;
   R_KTY = R_CONST/((1023/ADC_av)-1);
   // resistor values from kty83-110 data sheet, written as polynomial trend line
   return 1.06e-8*pow(R_KTY,3)-7.16e-5*pow(R_KTY,2)+0.2461*R_KTY-160.21+CORR;
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt
  int ADCread=analogRead(A0);
  ADCread=constrain(ADCread,120,500); //keep values in limits
  ADC_buff[index]=ADCread; //put value in table
   
  ADC_av=0;
  for (int i=0; i<AV_nbr; i++){ //sum up
    ADC_av+=ADC_buff[i];
  }
  
  ADC_av/=AV_nbr; //get abvarage

  index++;  //increment index and reset if biger than table size
  if(index>=AV_nbr) index=0;
}


       
