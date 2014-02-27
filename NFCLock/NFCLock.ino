#include <LiquidCrystal.h>
#include <Wire.h>
#include "nfc.h"
#include <EEPROM.h>
#include "RTClib.h"                                                          OROLOGIO
#define chip1 0x50 // device address for left-hand chip on our breadboard    EEPROM
#define chip2 0x51 // and the right                                          EEPROM

#define RESET   1               //pin relativo al RESET da premere per entrare in modalità scrittura/cancellazione
#define PSU   3                    //pin relativo al RESET su
#define PGIU   2                   //pin relativo al RESET giu
#define SERRATURA   4               //////////////////////////pin relativo allo sblocco della serratura
#define GREEN_LED_PIN  5           //pin relativo al led verde
#define RED_LED_PIN    6           //pin relativo al led rosso
#define BUZZER_PIN   7
#define IDLE_MODE 0
#define SAVE_MODE 1
#define DELETE_MODE 2

unsigned int pointer = 69; // we need this to be unsigned, as you may have an address > 32767                    EERPROMS
byte d=0; // example variable to handle data going in and out of EERPROMS                                        EERPROMS
RTC_DS1307 RTC;                                                                                              //  EERPROMS
LiquidCrystal lcd(8,9,10,11,12,13);
NFC_Module nfc;                   // define a nfc class 
boolean check;                    //variabile con la quale eseguo tutti i controlli all'interno dello sketch
int on_off=0;                     //variabile che utilizzo per controllare lo stato del led in modalità bistabile

int currentMode = IDLE_MODE;
int lastMode = IDLE_MODE;
int pag = 0;
int pagprima=0;
void ledForAzzera() 
{
  lcd.write("LED FOR AZZERA");
  digitalWrite(GREEN_LED_PIN,HIGH);
  digitalWrite(RED_LED_PIN,HIGH);
  for(int i=0;i<5;i++)
   {
      digitalWrite(GREEN_LED_PIN,LOW);
      delay(50);
      digitalWrite(GREEN_LED_PIN,HIGH);
      digitalWrite(RED_LED_PIN,LOW);
      delay(50);
      digitalWrite(RED_LED_PIN,HIGH);
   }
}

void ledForApri() 
{
  lcd.write("LED FOR APRI");
  for(int i=0;i<5;i++)
   {  
      digitalWrite(GREEN_LED_PIN,HIGH);
      delay(50);
      digitalWrite(GREEN_LED_PIN,LOW);
      delay(50);
   }
}

void ledForError() 
{
  lcd.write("LED FOR ERROR");
  for(int i=0;i<5;i++)
   {
     digitalWrite(RED_LED_PIN,HIGH);
     delay(50);
     digitalWrite(RED_LED_PIN,LOW);
     delay(50);
   }  
} 

void ledForIdle()
{
 // lcd.write("LED FOR IDLE");
  digitalWrite(GREEN_LED_PIN,HIGH);
  digitalWrite(RED_LED_PIN,HIGH);
}
 
void ledForSave()
{
  lcd.write("LED FOR SAVE");
  digitalWrite(GREEN_LED_PIN,HIGH);
  digitalWrite(RED_LED_PIN,LOW);
} 

void ledForDelete()
{
  lcd.write("LED FOR DELETE");
  digitalWrite(GREEN_LED_PIN,LOW);
  digitalWrite(RED_LED_PIN,HIGH);
}  

void azzera()
{
   for(int i=0;i<1023;i++)   EEPROM.write(i,0xff);
   lcd.write("Memoria Azzerata!");
   ledForAzzera();
}
 
void stampa_code(byte * code){
        lcd.write(": <");
        for (int i=0; i<5; i++) {
          if (code[i] < 16) lcd.write("0");
     //     lcd.write(code[i], HEX);      LCD NON SUPPORTA HEX 
          if(i!=4)
            lcd.write(" ");
        }
        lcd.write(">");
}

void setup() {
  RTC.begin();                                                                      //orologio
    if (! RTC.isrunning()) {                                                        //orologio
    lcd.write("RTC is NOT running!");                                               //orologio
    // following line sets the RTC to the date & time this sketch was compiled      //orologio
    RTC.adjust(DateTime(__DATE__, __TIME__));                                       //orologio
  }
  
  Wire.begin();                                                                     // wake up, I2C!
  
  lcd.begin(16, 2);
  noTone(7);   //zitto di partenza
  // put your setup code here, to run once:
  pinMode(SERRATURA,OUTPUT);                                               ////////////////////////Pin apertura 12 
  pinMode(RESET,INPUT);                                                 //imposto il pin del RESET in modalità input per verificare quando il RESET viene premuto
  pinMode(PSU,INPUT);
  pinMode(PGIU,INPUT);  
  digitalWrite(RESET,HIGH);                                             //e lo setto alto, in modo tale da attivare la resistenza di pull-up
  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(RED_LED_PIN,OUTPUT);
                                              //inizializzo la seriale sulla quale leggo i dati delle schede a 9600 bau
  if(digitalRead(RESET)==HIGH) azzera(); 
  nfc.begin();
 
  uint32_t versiondata = nfc.get_version();
  if (! versiondata) {
    lcd.write("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  lcd.write("Found chip PN5"); 
  // lcd.write((versiondata>>24) & 0xFF, HEX); 
  // lcd.write("Firmware ver. "); 
  // lcd.write((versiondata>>16) & 0xFF, DEC); 
  //lcd.write('.'); lcd.write((versiondata>>8) & 0xFF, DEC);
  
  /** Set normal mode, and disable SAM */
  nfc.SAMConfiguration();
  ledForIdle();
}
    
    void leggireset(){
    unsigned long int tempo=0;
    unsigned long int tempoFinale=0;
    boolean buttonWasPressed = false;    //controllo se il RESET è premuto
    tempo=millis();
    tempoFinale = tempo;     //se lo è salvo gli attuali millisecondi passati dall'avvio del dispositivo

    
     
     while((digitalRead(RESET)==HIGH))
     {
      
         
         lcd.write("RESET premuto... Attendo...");
         buttonWasPressed = true;
         
         tempoFinale = millis();
         if (tempoFinale - tempo < 1000 ) 
         {
             lcd.write("ENTRO IN IDLE");           
             ledForIdle();
             lastMode = currentMode;
             currentMode = IDLE_MODE;
         }
         else if (tempoFinale - tempo >= 1000 && tempoFinale  -tempo < 2000 && currentMode != SAVE_MODE) 
         {
           lcd.write("ENTRO IN SAVE");
           ledForSave();  
           lastMode = currentMode;
           currentMode = SAVE_MODE;
         } 
         else if (tempoFinale - tempo >= 2000 && currentMode != DELETE_MODE)
         {
            lcd.write("ENTRO IN DELETE");
            ledForDelete();  
            lastMode = currentMode;
            currentMode = DELETE_MODE;
         }
       
         delay(1000);
     }
    
   
    
     lcd.write("Uscito da ciclo di attesa!");

    if (buttonWasPressed)
      {
      lcd.write("Bottone premuto");
      if (currentMode == IDLE_MODE && lastMode == IDLE_MODE) 
      {
             lcd.write("Nessuna modalita particolare: Apriporta!");                                //INTEGRAZIONE APRIPORTA Scrivi su seriale
             digitalWrite(SERRATURA,HIGH);                               //INTEGRAZIONE APRIPORTA Apre la serratura senza giocare con i leds
             delay(1000);                                                //INTEGRAZIONE APRIPORTA apsetta
             digitalWrite(SERRATURA,LOW);
      }
      else if (currentMode == IDLE_MODE && lastMode == SAVE_MODE) 
      {
          lcd.write("ERO IN SAVE ENTRO IN IDLE... NON FACCIO NULLA");
      }
      else if (currentMode == IDLE_MODE && lastMode == DELETE_MODE) 
      {
          lcd.write("ERO IN DELETE ENTRO IN IDLE... NON FACCIO NULLA");
      }
      else
      {
          lcd.write("CASINO");
      }
    }


}





void leggitag()
{
 u8 buf[32],sta; 

 // do{                                                                //inizio un ciclo che finirà solo quando verrà premuto nuovamente il RESET
 
    // Polling the mifar card, buf[0] is the length of the UID //
    sta = nfc.InListPassiveTarget(buf);
    
  //  digitalWrite(SERRATURA,HIGH);                              //INTEGRAZIONE APRIPORTA Chiudo apriporta se esco dal loop RESET premuto fra l'istante 10-50 mills
   
    if(sta && buf[0] == 4){
    // the card may be Mifare Classic card, try to read the block //  
  /*  lcd.write("UUID length:");
    lcd.write(buf[0], DEC);
    lcd.write();
    lcd.write("UUID:");
    nfc.puthex(buf+1, buf[0]); */
    lcd.write("qui dati UUID");
    
    // factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF //
    u8 key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 blocknum = 4;
    // Authentication blok 4 //
    sta = nfc.MifareAuthentication(0, blocknum, buf+1, buf[0], key);
    if(sta){
      lcd.write("Autenticazione OK");
      // save read block data                                                           //se lo è passo a controllare se devo salvare o cancellare
            check=false;  
            //rimetto a false la variabile check per successivi utilizzi
            lcd.write("Controllo modalita");



            
            if(currentMode == SAVE_MODE){                                         //controllo se devo scrivere
                  lcd.write("SALVO IL TAG");
                  for(int i=0;i<1021;i+=4){                                   //in caso affermativo eseguo un ciclo che controlla tutta la EEPROM
                        if((EEPROM.read(i)==buf[1])&&(EEPROM.read(i+1)==buf[2])&&(EEPROM.read(i+2)==buf[3])&&(EEPROM.read(i+3)==buf[4])){
                            check=true;                                        //se trovo il codice della tessera letta già salvato nella EEPROM metto a true la variabile 'check'
                            break;                                             //ed esco dal ciclo
                        }
                  }
                  if(check){                                                   //quindi controllo il valore della variabile check, se è vero, significa che la tessera è già stata registrata
                        lcd.write("Tessera già registrata!");               //quindi lo comunico su seriale
                        stampa_code(buf);                
                        ledForError();
                        ledForSave();
                  }
                  else {                                                        //se la tessera non è stata trovata
                      check=false;                                             //rimetto a false la variabile check per successivi utilizzi
                      for(int i=0;i<1021;i+=4){                                //quindi eseguo un ciclo che controlla tutta la EEPROM in cerca di 5 byte successivi liberi
                        if((EEPROM.read(i)==0xff)&&(EEPROM.read(i+1)==0xff)&&(EEPROM.read(i+2)==0xff)&&(EEPROM.read(i+3)==0xff)){
                          for(int j=i;j<i+4;j++)                               //una volta trovati, partendo dal primo, fino al quinto, ci salvo il valore della tessera
                              EEPROM.write(j,buf[j-i+1]);                             //eseguendo un ciclo 5 volte
                          check=true;                                                //pongo a true la variabile check
                          break;                                               //ed esco dal ciclo
                        }
                      }
                      if(check){                                               //se la variabile check è vera, significa che ho salvato con successo, quindi
                          lcd.write("Tessera Salvata");                     //lo stampo su seriale
                          stampa_code(buf);
                          ledForApri();
                          ledForSave();
                        
                      }
                      else{                                                    //se la variabile check non è vera, significa che ho controllato tutta la memoria senza trovare 5 byte sequenziali liberi
                           lcd.write("Memoria piena");                    //quindi spamo su seriale che la memoria è piena
                           ledForError();
                           ledForSave();
                           }
                    }
                }

 
    }  else if(currentMode == DELETE_MODE){                                      //se non bisogna salvare, controllo se bisogna eliminare una tessera
 
                 
                  lcd.write("CANCELLO IL TAG");
                  int posizione=-1;                                            //quindi inizializzo a -1 la variabile posizione, che mi servirà per salvare la posizione nella EEPROM della tessera
                  lcd.write("");
                  for(int i=0;i<1021;i+=4){                                    //ed eseguo un ciclo che controlla tutta la EEPROM per cercare il codice corrispondente
                        if((EEPROM.read(i)==buf[1])&&(EEPROM.read(i+1)==buf[2])&&(EEPROM.read(i+2)==buf[3])&&(EEPROM.read(i+3)==buf[4])){
                            posizione=i;                                       //se viene trovato salvo la posizione del primo byte nella variabile posizione
                        break;                                                 //ed esco dal ciclo
                        }
                  }
                  if(posizione!=-1){                                           //quindi controllo che la variabile posizione sia diversa da -1 così da sapere se è stato trovato o meno il codice
                      for(int j=posizione;j<posizione+4;j++)                   //eseguo quindi un ciclo partendo dalla posizione 'posizione' nella EEPROM
                              EEPROM.write(j,0xff);                            //sovrascrivendo i 5 byte corrispondenti alla tessera, con il byte di default '0xff'
                      lcd.write("Scheda cancellata");                       //una volta fatto ciò, stampo su seriale l'avvenuta cancellazione
                      stampa_code(buf);
                      ledForApri();
                      ledForDelete();
                  }
                  else{                                                        //se la variabile posizione vale -1 significa che non ha trovato in memoria la tessera letta
                      lcd.write("Impossibile cancellare la scheda, non è salvata");  //quindi lo comunico su seriale
                      stampa_code(buf);
                      ledForError();
                      ledForDelete();
                    }
            }
 
            check=true;                                                        //rimetto a vero il valore della variabile check siccome il checksum è corretto
       
  }
   
  
 // }   while((digitalRead(RESET)==LOW)&&(controllo||scrivere));


   
  if(sta && currentMode == IDLE_MODE){                                                     //e controllo anche che non ci sia da salvare/scrivere una tessera
                  lcd.write("Verifica tessera");
                  lcd.write("TAG: ");
                  stampa_code(buf);
                  check=false;                                                //rimetto a false la variabile check per successivi utilizzi
                  for(int i=0;i<1021;i+=4)                                    //eseguo un ciclo che controlla tutta la EEPROM alla ricerca della tessera letta
                        if(EEPROM.read(i)==buf[1]&&EEPROM.read(i+1)==buf[2]&&EEPROM.read(i+2)==buf[3]&&EEPROM.read(i+3)==buf[4]){
                            check=true;                                       //se viene trovata metto a true la variabile check
                            break;                                            //ed esco dal ciclo
                        }
 
                     if(check){                                               //quindi controllo il valore della variabile check
                       lcd.write("Tessera valida");                        //se è vero, significa che la tessera è stata trovata e quindi è valida, e lo stampo su seriale
                       stampa_code(buf);
                       ledForApri();
                       ledForIdle();
                       digitalWrite(SERRATURA,HIGH);                          ////////////////////////sblocca serratura
                       delay(1000);                                            ////////////////////////apsetta
                       digitalWrite(SERRATURA,LOW);                           ////////////////////////blocca serratura
                       } 
                     else{                                                    //se al contrario il valore è falso
                        lcd.write("Tessera non valida!");                   //significa che ho controllato tutta la memoria senza trovare la tessera, quindi lo comunico su seriale
                        stampa_code(buf);
                        ledForError();   
                        ledForIdle();             
                        }
                      delay(500); 
    }
}

void leggippsg() {
    unsigned long int tempo=millis();
    boolean buttonWasPressed = false;    //controllo se il RESET è premuto
    unsigned long int tempartenza;
    if(digitalRead(PSU)==HIGH&&(PGIU)==LOW){
    pag=pag-1;
    delay (20);
    tempartenza=tempo;
    }
    if(digitalRead(PSU)==LOW&&(PGIU)==HIGH){
    pag=pag+1;
    delay (20);
    tempartenza=tempo;
    }
    if(tempo-tempartenza>10000) pag=0;
    //PERCHE' QUESTO COMANDO MI DA ERRORE? if(pag>10&&pag<15) int pag=1;
    //PERCHE' QUESTO COMANDO MI DA ERRORE? if(pag<0) int pag=10;
    if(digitalRead(PSU)==HIGH&&(PGIU)==HIGH){
      pag=15;
    
      pag=0;
      tempartenza=tempo;
      }
}

void stampadisplay()  {
  switch (pag) {  
      case 0:
      lcd.write("min:ora - giorno/mese/anno");
      lcd.write("TAG REGISTRATI ""numtagssaved");
      break;
      case 1:
      lcd.write("1) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("2) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 2:
      lcd.write("2) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("3) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 3:
      lcd.write("3) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("4) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 4:
      lcd.write("4) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("5) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 5:
      lcd.write("5) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("6) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 6:
      lcd.write("6) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("7) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 7:
      lcd.write("7) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("8) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 8:
      lcd.write("8) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("9) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 9:
      lcd.write("9) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("10) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 10:
      lcd.write("10) TAG""ntag" "min:ora - giorno/mese/anno");
      lcd.write("1) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 15:
      lcd.write("PREMERE RESET 1 SECONDO PER");
      lcd.write("SALVARE/CANCELLARE TAGS");
      delay (2000);
      break;
      }
}
void ora () {
    DateTime now = RTC.now();
    
/*
    lcd.write(now.year(), DEC);         // PROBLEMA COL DECAFFEINATO da convertire in stringa
    lcd.write('/');
    lcd.write(now.month(), DEC);
    lcd.write('/');
    lcd.write(now.day(), DEC);
    lcd.write(' ');
    lcd.write(now.hour(), DEC);
    lcd.write(':');
    lcd.write(now.minute(), DEC);
    lcd.write(':');
    lcd.write(now.second(), DEC);
*/
}
 

void writeData(int device, unsigned int add, byte data)                                              // EEPROM
// writes a byte of data 'data' to the chip at I2C address 'device', in memory location 'add'        // EEPROM
{
  Wire.beginTransmission(device);                                                                    // EEPROM
  Wire.write((int)(add >> 8)); // left-part of pointer address                                       // EEPROM
  Wire.write((int)(add & 0xFF)); // and the right                                                    // EEPROM
  Wire.write(data);                                                                                  // EEPROM
  Wire.endTransmission();                                                                            // EEPROM
  delay(10);                                                                                         // EEPROM
}
 
byte readData(int device, unsigned int add)                                                          // EEPROM
// reads a byte of data from memory location 'add' in chip at I2C address 'device'                   // EEPROM
{
  byte result; // returned value
  Wire.beginTransmission(device); // these three lines set the pointer position in the EEPROM
  Wire.write((int)(add >> 8)); // left-part of pointer address
  Wire.write((int)(add & 0xFF)); // and the right
  Wire.endTransmission();
  Wire.requestFrom(device,1); // now get the byte of data...
  result = Wire.read();
  return result; // and return it as a result of the function readData
}
 
void memoria()                                                                                       // EEPROM
{
  lcd.write("Writing data...");                                                                 // EEPROM
  for (int a=0; a<20; a++)
  {
    writeData(chip1,a,a);
    writeData(chip2,a,a); // looks like a tiny EEPROM RAID solution!
  }
  lcd.write("Reading data...");
  for (int a=0; a<20; a++)
  {
    lcd.write("chip1 pointer ");
    lcd.write(a);
    lcd.write(" holds ");
    d=readData(chip1,a);
 //   lcd.write(d, DEC);                           // PROBLEMA COL DECAFFEINATO da convertire in stringa
  }
  for (int a=0; a<20; a++)
  {
    lcd.write("chip2 pointer ");
    lcd.write(a);
    lcd.write(" holds ");
    d=readData(chip2,a);
 //   lcd.write(d, DEC);                           // PROBLEMA COL DECAFFEINATO da convertire in stringa
  } 
}




void loop() {
 // memoria();         // schatc preso dal nostro sito per scrivere e leggere sulla EEPROM gli spazi disponibili
 // ora();             // schatc preso dagli esempi arduino per scrittura ora PS: risolvere problema decaffeinato
 // leggitag();        // verifica la scheda letta, verifica quelle in memoria e tramite variabili da leggireset salva o cancella eeprom. può richiamare apriporta.
 // leggireset();      // conta il tempo del reset. Se sopra i 2 sec conta il tempo senza pressione ed entra in save/delete. se nessun tasto premuto x 1 min o seve delete concluse  si esce
 // leggippsg();       // conta il tempo trascorso dalla pressione dei tasti su e giu e valuta cosa far stamapare da stampadisplay()
if (pag!=pagprima)  stampadisplay();   // richiama la stampa su display in fase di loop in base a variabili che arrivano da lettura tag, leggippsg(), ecc.
pagprima=pag;
}
