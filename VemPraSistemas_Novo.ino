#include <EEPROM.h>

//Carrega as bibliotecas

#include "EmonLib.h"
#include <LiquidCrystal.h>
#include <avr/eeprom.h>
#include <SPI.h>
#include <Ethernet.h>
String readString;
 


byte mac[] = { 0xA4, 0x28, 0x72, 0xCA, 0x55, 0x2F };
byte ip[] = { 192, 168, 0, 120};
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[] = { 255, 255, 255, 0 };
 
EthernetServer server(80);
 

#define eeprom_read_to(dst_p, eeprom_field, dst_size) eeprom_read_block(dst_p, (void *)offsetof(__eeprom_data, eeprom_field), MIN(dst_size, sizeof((__eeprom_data*)0)->eeprom_field))
#define eeprom_read(dst, eeprom_field) eeprom_read_to(&dst, eeprom_field, sizeof(dst))
#define eeprom_write_from(src_p, eeprom_field, src_size) eeprom_write_block(src_p, (void *)offsetof(__eeprom_data, eeprom_field), MIN(src_size, sizeof((__eeprom_data*)0)->eeprom_field))
#define eeprom_write(src, eeprom_field) { typeof(src) x = src; eeprom_write_from(&x, eeprom_field, sizeof(x)); }
#define MIN(x,y) ( x > y ? y : x )
#define     pin_sct     A1


EnergyMonitor emon1;
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

//Tensao da rede eletrica
int rede = 220;

//Pino do sensor SCT
int pino_sct = A1;

struct __eeprom_data {
double flash_kwhtotal;
};

//Cria variaveis globais
double kwhTotal;
double kwhTotal_Acc;
double vlreais;
double vlreais_Acc;
double realPower;
unsigned long ltmillis, tmillis, timems, previousMillis, refresh;
char charBuf[30];
void setup()
{
 Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
 
  
  
pinMode(pin_sct, INPUT); 
Serial.begin(9600);
emon1.current(pino_sct, 0.10); //Pino, calibracao - Cur Const= Ratio/BurdenR. 1800/62 = 29.
eeprom_read(kwhTotal, flash_kwhtotal);
previousMillis = millis();
lcd.begin(16, 2);
lcd.clear();

}

void loop()
{


EthernetClient client = server.available();
  if (client) {
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
        }
        if (c == '\n')
        {
          
          readString = "";
 
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<html>");
          client.println("<head>");
          client.println("<title>#VemPraSistemas</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<h1> Vem pra Sistemas </h1>");
          client.println("<h4>Kwh</h4>");
          client.println(kwhTotal_Acc, 10);
          client.println("<h4>Cons($)</h4>");
          client.println(vlreais_Acc, 10);
          client.println("</body>");
          client.println("</html>");
           
          delay(1);
          client.stop();
        }
      }
    }
  }

  
// Calcula quantidade de tempo desde a última measurment realpower.
ltmillis = tmillis;
tmillis = millis();
timems = tmillis - ltmillis;
double Irms = emon1.calcIrms(1480); // Calculate Irms

// Calcular o número de hoje de kWh consumido.
//kwhTotal = kwhTotal + ((realPower/1000.0) * 1.0/3600.0 * (timems/1000.0));
kwhTotal = (((Irms*127.0)/1000.0) * 1.0/3600.0 * (timems/1000.0));
kwhTotal_Acc = kwhTotal_Acc + kwhTotal;
vlreais = kwhTotal * 0.57;
vlreais_Acc = vlreais_Acc + vlreais;

  // exibe os whats da rede
  Serial.print("Watts: ");
  Serial.println(rede); // potencia aparente
  lcd.setCursor(0,0);
  lcd.print("Tens(V): ");
  lcd.setCursor(8,0);
  lcd.print(rede);
  delay(2000);

 // exibe a quantidade de kwh
  Serial.print("kwhTotal_Acc: ");
  Serial.println(kwhTotal_Acc, 10);
  lcd.setCursor(0,1);
  lcd.print("Kwh:");
  lcd.setCursor(8,1);
  lcd.print(kwhTotal_Acc);
  
// exibe o consumo de energia que o aparelho faz
  Serial.print("Consumo: ");
  Serial.println(vlreais_Acc, 10);
  lcd.setCursor(0,0);
  lcd.print("Cons($):");
  lcd.setCursor(8,0);
  lcd.print(vlreais_Acc);
  delay(2000);



  // exibe a potencia no lcd
  Serial.print("Potencia : ");
  Serial.println(Irms*rede);
  lcd.setCursor(0,1);
  lcd.print("Pot(W):");
  lcd.setCursor(8,1);
  lcd.print(Irms*rede,1);

    


  // exibe a corrente no LCD
  Serial.print("Corrente: ");
  Serial.println(Irms); // Irms
  lcd.setCursor(0,0);
  lcd.print("Corr(A):");
  lcd.setCursor(8,0);
  lcd.print(Irms);

  
  delay(2000);
  lcd.clear();


  // imprime na tela o #vempraprasistemas
  Serial.println("Vem Pra Sistemas");
  lcd.setCursor(0,0);
  lcd.print("#VemPraSistemas");
  delay(2000);
  lcd.clear();

/*
Serial.print("vlreais_Acc: ");
//Serial.print(vlreais, 10);
Serial.println(vlreais_Acc, 10);
lcd.setCursor(0,0);
lcd.print("A:");
lcd.setCursor(3,0);
lcd.print(vlreais, 10);
delay(2000);
lcd.clear();
*/

//grava na memoria a cada 1 minuto
if ((millis() - refresh)>= 100)
refresh = millis(); //actualiza a contagem.
{
//Serial.println("Gravando na EEprom");
eeprom_write(kwhTotal, flash_kwhtotal);
previousMillis=millis();
}
//Multiplica pelo valor kilowatt hora R$ 0.57 Reais
//vlreais = kwhTotal * 0.57;
}

void printFloat(float value, int places) {
// this is used to cast digits
int digit;
float tens = 0.1;
int tenscount = 0;
int i;
float tempfloat = value;

// Se certificar de que arredondar corretamente. este poderia usar pow de <math.h>, mas não parece vale a importação
// Se esta etapa arredondamento não está aqui, o valor 54,321 imprime como 54,3209

// calcular arredondamento prazo d: 0,5 / pow (10, lugares)
float d = 0.5;
if (value < 0)
d *= -1.0;
// dividir por dez para cada casa decimal
for (i = 0; i < places; i++)
d/= 10.0;
// este pequeno disso, combinado com truncamento vai arredondar os nossos valores corretamente
tempfloat += d;

// Primeiro obter dezenas de valor para ser a grande potência de dez a menos do que o valor
// Tenscount não é necessário, mas seria útil se você queria saber depois desta quantos caracteres o número tomará

if (value < 0)
tempfloat *= -1.0;
while ((tens * 10.0) <= tempfloat) {
tens *= 10.0;
tenscount += 1;
}


// escrever o negativo, se necessário
if (value < 0)
Serial.print('-');

if (tenscount == 0)
Serial.print(0, DEC);

for (i=0; i< tenscount; i++) {
digit = (int) (tempfloat/tens);
Serial.print(digit, DEC);
tempfloat = tempfloat - ((float)digit * tens);
tens /= 10.0;
}

// se não há lugares após decimal, pare agora e retorno
if (places <= 0)
return;

// caso contrário, escreva o ponto e continuar
Serial.print('.');

// Agora, escrever cada casa decimal, deslocando um dígitos por uma, para o lugar queridos e escrever o valor truncado
for (i = 0; i < places; i++) {
tempfloat *= 10.0;
digit = (int) tempfloat;
Serial.print(digit,DEC);
// uma vez escrito, subtrair fora esse dígito
tempfloat = tempfloat - (float) digit;
delay(1000);
lcd.clear();

}
}
