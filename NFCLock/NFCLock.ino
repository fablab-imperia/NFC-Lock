
#include <Wire.h>
#include "nfc.h"
#include <EEPROM.h>

#define PULSANTE   12               //pin relativo al pulsante da premere per entrare in modalità scrittura/cancellazione
#define PSU   13                    //pin relativo al pulsante su
#define PGIU   14                   //pin relativo al pulsante giu
#define SERRATURA   8               //////////////////////////pin relativo allo sblocco della serratura
#define GREEN_LED_PIN  5           //pin relativo al led verde
#define RED_LED_PIN    6           //pin relativo al led rosso

#define IDLE_MODE 0
#define SAVE_MODE 1
#define DELETE_MODE 2

NFC_Module nfc;                   // define a nfc class 
boolean check;                    //variabile con la quale eseguo tutti i controlli all'interno dello sketch
int on_off=0;                     //variabile che utilizzo per controllare lo stato del led in modalità bistabile

int currentMode = IDLE_MODE;
int lastMode = IDLE_MODE;

void ledForAzzera() 
{
  Serial.println("LED FOR AZZERA");
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
  Serial.println("LED FOR APRI");
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
  Serial.println("LED FOR ERROR");
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
 // Serial.println("LED FOR IDLE");
  digitalWrite(GREEN_LED_PIN,HIGH);
  digitalWrite(RED_LED_PIN,HIGH);
}
 
void ledForSave()
{
  Serial.println("LED FOR SAVE");
  digitalWrite(GREEN_LED_PIN,HIGH);
  digitalWrite(RED_LED_PIN,LOW);
} 

void ledForDelete()
{
  Serial.println("LED FOR DELETE");
  digitalWrite(GREEN_LED_PIN,LOW);
  digitalWrite(RED_LED_PIN,HIGH);
}  

void azzera()
{
   for(int i=0;i<1023;i++)   EEPROM.write(i,0xff);
   Serial.println("Memoria Azzerata!");
   ledForAzzera();
}
 
void stampa_code(byte * code){
        Serial.print(": <");
        for (int i=0; i<5; i++) {
          if (code[i] < 16) Serial.print("0");
          Serial.print(code[i], HEX);
          if(i!=4)
            Serial.print(" ");
        }
        Serial.println(">");
}

void setup() {
  // put your setup code here, to run once:
  pinMode(SERRATURA,OUTPUT);                                               ////////////////////////Pin apertura 12 
  pinMode(PULSANTE,INPUT);                                                 //imposto il pin del pulsante in modalità input per verificare quando il pulsante viene premuto
  pinMode(PSU,INPUT);
  pinMode(PGIU,INPUT);  
  digitalWrite(PULSANTE,HIGH);                                             //e lo setto alto, in modo tale da attivare la resistenza di pull-up
  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(RED_LED_PIN,OUTPUT);
  Serial.begin(9600);                                                       //inizializzo la seriale sulla quale leggo i dati delle schede a 9600 bau
  if(digitalRead(PULSANTE)==HIGH) azzera(); 
  nfc.begin();
 
  uint32_t versiondata = nfc.get_version();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  /** Set normal mode, and disable SAM */
  nfc.SAMConfiguration();
  ledForIdle();
}
    
    void leggireset(){
    unsigned long int tempo=0;
    unsigned long int tempoFinale=0;
    boolean buttonWasPressed = false;    //controllo se il pulsante è premuto
    tempo=millis();
    tempoFinale = tempo;     //se lo è salvo gli attuali millisecondi passati dall'avvio del dispositivo

    
     
     while((digitalRead(PULSANTE)==HIGH))
     {
      
         
         Serial.println("Pulsante premuto... Attendo...");
         buttonWasPressed = true;
         
         tempoFinale = millis();
         if (tempoFinale - tempo < 1000 ) 
         {
             Serial.println("ENTRO IN IDLE");           
             ledForIdle();
             lastMode = currentMode;
             currentMode = IDLE_MODE;
         }
         else if (tempoFinale - tempo >= 1000 && tempoFinale  -tempo < 2000 && currentMode != SAVE_MODE) 
         {
           Serial.println("ENTRO IN SAVE");
           ledForSave();  
           lastMode = currentMode;
           currentMode = SAVE_MODE;
         } 
         else if (tempoFinale - tempo >= 2000 && currentMode != DELETE_MODE)
         {
            Serial.println("ENTRO IN DELETE");
            ledForDelete();  
            lastMode = currentMode;
            currentMode = DELETE_MODE;
         }
       
         delay(1000);
     }
    
   
    
     Serial.println("Uscito da ciclo di attesa!");

    if (buttonWasPressed)
      {
      Serial.println("Bottone premuto");
      if (currentMode == IDLE_MODE && lastMode == IDLE_MODE) 
      {
             Serial.println("Nessuna modalita particolare: Apriporta!");                                //INTEGRAZIONE APRIPORTA Scrivi su seriale
             digitalWrite(SERRATURA,HIGH);                               //INTEGRAZIONE APRIPORTA Apre la serratura senza giocare con i leds
             delay(1000);                                                //INTEGRAZIONE APRIPORTA apsetta
             digitalWrite(SERRATURA,LOW);
      }
      else if (currentMode == IDLE_MODE && lastMode == SAVE_MODE) 
      {
          Serial.println("ERO IN SAVE ENTRO IN IDLE... NON FACCIO NULLA");
      }
      else if (currentMode == IDLE_MODE && lastMode == DELETE_MODE) 
      {
          Serial.println("ERO IN DELETE ENTRO IN IDLE... NON FACCIO NULLA");
      }
      else
      {
          Serial.println("CASINO");
      }
    }


}





void leggitag()
{
 u8 buf[32],sta; 

 // do{                                                                //inizio un ciclo che finirà solo quando verrà premuto nuovamente il pulsante
 
    // Polling the mifar card, buf[0] is the length of the UID //
    sta = nfc.InListPassiveTarget(buf);
    
  //  digitalWrite(SERRATURA,HIGH);                              //INTEGRAZIONE APRIPORTA Chiudo apriporta se esco dal loop pulsante premuto fra l'istante 10-50 mills
   
    if(sta && buf[0] == 4){
    // the card may be Mifare Classic card, try to read the block //  
    Serial.print("UUID length:");
    Serial.print(buf[0], DEC);
    Serial.println();
    Serial.print("UUID:");
    nfc.puthex(buf+1, buf[0]);
    Serial.println();
    
    // factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF //
    u8 key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 blocknum = 4;
    // Authentication blok 4 //
    sta = nfc.MifareAuthentication(0, blocknum, buf+1, buf[0], key);
    if(sta){
      Serial.println("Autenticazione OK");
      // save read block data                                                           //se lo è passo a controllare se devo salvare o cancellare
            check=false;  
            //rimetto a false la variabile check per successivi utilizzi
            Serial.println("Controllo modalita");



            
            if(currentMode == SAVE_MODE){                                         //controllo se devo scrivere
                  Serial.println("SALVO IL TAG");
                  for(int i=0;i<1021;i+=4){                                   //in caso affermativo eseguo un ciclo che controlla tutta la EEPROM
                        if((EEPROM.read(i)==buf[1])&&(EEPROM.read(i+1)==buf[2])&&(EEPROM.read(i+2)==buf[3])&&(EEPROM.read(i+3)==buf[4])){
                            check=true;                                        //se trovo il codice della tessera letta già salvato nella EEPROM metto a true la variabile 'check'
                            break;                                             //ed esco dal ciclo
                        }
                  }
                  if(check){                                                   //quindi controllo il valore della variabile check, se è vero, significa che la tessera è già stata registrata
                        Serial.print("Tessera già registrata!");               //quindi lo comunico su seriale
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
                          Serial.print("Tessera Salvata");                     //lo stampo su seriale
                          stampa_code(buf);
                          ledForApri();
                          ledForSave();
                        
                      }
                      else{                                                    //se la variabile check non è vera, significa che ho controllato tutta la memoria senza trovare 5 byte sequenziali liberi
                           Serial.println("Memoria piena");                    //quindi spamo su seriale che la memoria è piena
                           ledForError();
                           ledForSave();
                           }
                    }
                }

 
    }  else if(currentMode == DELETE_MODE){                                      //se non bisogna salvare, controllo se bisogna eliminare una tessera
 
                 
                  Serial.println("CANCELLO IL TAG");
                  int posizione=-1;                                            //quindi inizializzo a -1 la variabile posizione, che mi servirà per salvare la posizione nella EEPROM della tessera
                  Serial.println("");
                  for(int i=0;i<1021;i+=4){                                    //ed eseguo un ciclo che controlla tutta la EEPROM per cercare il codice corrispondente
                        if((EEPROM.read(i)==buf[1])&&(EEPROM.read(i+1)==buf[2])&&(EEPROM.read(i+2)==buf[3])&&(EEPROM.read(i+3)==buf[4])){
                            posizione=i;                                       //se viene trovato salvo la posizione del primo byte nella variabile posizione
                        break;                                                 //ed esco dal ciclo
                        }
                  }
                  if(posizione!=-1){                                           //quindi controllo che la variabile posizione sia diversa da -1 così da sapere se è stato trovato o meno il codice
                      for(int j=posizione;j<posizione+4;j++)                   //eseguo quindi un ciclo partendo dalla posizione 'posizione' nella EEPROM
                              EEPROM.write(j,0xff);                            //sovrascrivendo i 5 byte corrispondenti alla tessera, con il byte di default '0xff'
                      Serial.print("Scheda cancellata");                       //una volta fatto ciò, stampo su seriale l'avvenuta cancellazione
                      stampa_code(buf);
                      ledForApri();
                      ledForDelete();
                  }
                  else{                                                        //se la variabile posizione vale -1 significa che non ha trovato in memoria la tessera letta
                      Serial.print("Impossibile cancellare la scheda, non è salvata");  //quindi lo comunico su seriale
                      stampa_code(buf);
                      ledForError();
                      ledForDelete();
                    }
            }
 
            check=true;                                                        //rimetto a vero il valore della variabile check siccome il checksum è corretto
       
  }
   
  
 // }   while((digitalRead(PULSANTE)==LOW)&&(controllo||scrivere));


   
  if(sta && currentMode == IDLE_MODE){                                                     //e controllo anche che non ci sia da salvare/scrivere una tessera
                  Serial.println("Verifica tessera");
                  Serial.println("TAG: ");
                  stampa_code(buf);
                  check=false;                                                //rimetto a false la variabile check per successivi utilizzi
                  for(int i=0;i<1021;i+=4)                                    //eseguo un ciclo che controlla tutta la EEPROM alla ricerca della tessera letta
                        if(EEPROM.read(i)==buf[1]&&EEPROM.read(i+1)==buf[2]&&EEPROM.read(i+2)==buf[3]&&EEPROM.read(i+3)==buf[4]){
                            check=true;                                       //se viene trovata metto a true la variabile check
                            break;                                            //ed esco dal ciclo
                        }
 
                     if(check){                                               //quindi controllo il valore della variabile check
                       Serial.print("Tessera valida");                        //se è vero, significa che la tessera è stata trovata e quindi è valida, e lo stampo su seriale
                       stampa_code(buf);
                       ledForApri();
                       ledForIdle();
                       digitalWrite(SERRATURA,HIGH);                          ////////////////////////sblocca serratura
                       delay(1000);                                            ////////////////////////apsetta
                       digitalWrite(SERRATURA,LOW);                           ////////////////////////blocca serratura
                       } 
                     else{                                                    //se al contrario il valore è falso
                        Serial.print("Tessera non valida!");                   //significa che ho controllato tutta la memoria senza trovare la tessera, quindi lo comunico su seriale
                        stampa_code(buf);
                        ledForError();   
                        ledForIdle();             
                        }
                      delay(500); 
    }
}

void leggippsg() {
    unsigned long int tempo=millis();
    boolean buttonWasPressed = false;    //controllo se il pulsante è premuto
    unsigned long int tempartenza;
    if(digitalRead(PSU)==HIGH&&(PGIU)==LOW){
    int pag=pag-1;
    delay (20);
    tempartenza=tempo;
    }
    if(digitalRead(PSU)==LOW&&(PGIU)==HIGH){
    int pag=pag+1;
    delay (20);
    tempartenza=tempo;
    }
    if(tempo-tempartenza>10000) int pag=0;
    //PERCHE' QUESTO COMANDO MI DA ERRORE? if(pag>10&&pag<15) int pag=1;
    //PERCHE' QUESTO COMANDO MI DA ERRORE? if(pag<0) int pag=10;
    if(digitalRead(PSU)==HIGH&&(PGIU)==HIGH){
    unsigned long int pag=15;
    pag=0;
    tempartenza=tempo;
    }
}

void stampadisplay()  {
  int pag;
  switch (pag) {  
      case 0:
      Serial.println("min:ora - giorno/mese/anno");
      Serial.println("TAG REGISTRATI ""numtagssaved");
      break;
      case 1:
      Serial.println("1) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("2) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 2:
      Serial.println("2) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("3) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 3:
      Serial.println("3) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("4) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 4:
      Serial.println("4) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("5) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 5:
      Serial.println("5) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("6) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 6:
      Serial.println("6) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("7) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 7:
      Serial.println("7) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("8) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 8:
      Serial.println("8) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("9) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 9:
      Serial.println("9) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("10) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 10:
      Serial.println("10) TAG""ntag" "min:ora - giorno/mese/anno");
      Serial.println("1) TAG""ntag" "min:ora - giorno/mese/annp");
      break;
      case 15:
      Serial.println("PREMERE RESET 1 SECONDO PER");
      Serial.println("SALVARE/CANCELLARE TAGS");
      delay (2000);
      break;

      }
}

void loop() {
  stampadisplay();   // richiama la stampa su display in fase di loop in base a variabili che arrivano da lettura tag, leggippsg(), ecc.
  leggireset();      // conta il tempo del reset. Se sopra i 2 sec conta il tempo senza pressione ed entra in save/delete. se nessun tasto premuto x 1 min o seve delete concluse  si esce
  leggippsg();       // conta il tempo trascorso dalla pressione dei tasti su e giu e valuta cosa far stamapare da stampadisplay()
  leggitag();        // verifica la scheda letta, verifica quelle in memoria e tramite variabili da leggireset salva o cancella eeprom. può richiamare apriporta.
}
