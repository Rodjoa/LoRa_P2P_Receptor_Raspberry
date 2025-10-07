/*******************************************************************************
 *
 * Copyright (c) 2018 Dragino
 *
 * http://www.dragino.com
 *
 *******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>       // Para sleep()
#include <stdbool.h>      // Para bool, true, false

#include <sys/ioctl.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

/* Agregadas por mi para el MQTT */
#include "MQTTClient.h"
#define ADDRESS     "tcp://test.mosquitto.org:1883"
#define CLIENTID    "ProtoLoRa_pi3"
#define TOPIC       "MQTT_Examples"
#define QOS         1
#define TIMEOUT     10000L

// =========Variables de conexion MQTT
MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

//=====Variable auxiliar para TIMESTAMP del dato
uint32_t Last_Time_Stamp = 0; //Buffer de 1 dato para el ultimo timestamp recibido



/* ################# CONFIGURACION DEL LORA ################# */
#define REG_FIFO                    0x00
#define REG_OPMODE                  0x01
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_AD         0x0E
#define REG_FIFO_RX_BASE_AD         0x0F
#define REG_RX_NB_BYTES             0x13
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS               0x12
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_MODEM_CONFIG            0x1D
#define REG_MODEM_CONFIG2           0x1E
#define REG_MODEM_CONFIG3           0x26
#define REG_SYMB_TIMEOUT_LSB  		0x1F
#define REG_PKT_SNR_VALUE			0x19
#define REG_PAYLOAD_LENGTH          0x22
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_MAX_PAYLOAD_LENGTH 		0x23
#define REG_HOP_PERIOD              0x24
#define REG_SYNC_WORD				0x39
#define REG_VERSION	  				0x42

#define PAYLOAD_LENGTH              0x40

#define REG_LNA                     0x0C
#define LNA_MAX_GAIN                0x23
#define LNA_OFF_GAIN                0x00
#define LNA_LOW_GAIN		    	0x20

#define RegDioMapping1               0x40
#define RegDioMapping2               0x41
#define RegPaConfig                  0x09
#define RegPaRamp                    0x0A
#define RegPaDac                     0x5A

#define SX72_MC2_FSK                0x00
#define SX72_MC2_SF7                0x70
#define SX72_MC2_SF8                0x80
#define SX72_MC2_SF9                0x90
#define SX72_MC2_SF10               0xA0
#define SX72_MC2_SF11               0xB0
#define SX72_MC2_SF12               0xC0

#define SX72_MC1_LOW_DATA_RATE_OPTIMIZE  0x01

#define SX1276_MC1_BW_125                0x70
#define SX1276_MC1_BW_250                0x80
#define SX1276_MC1_BW_500                0x90
#define SX1276_MC1_CR_4_5            0x02
#define SX1276_MC1_CR_4_6            0x04
#define SX1276_MC1_CR_4_7            0x06
#define SX1276_MC1_CR_4_8            0x08
#define SX1276_MC1_IMPLICIT_HEADER_MODE_ON    0x01
#define SX1276_MC2_RX_PAYLOAD_CRCON        0x04
#define SX1276_MC3_LOW_DATA_RATE_OPTIMIZE  0x08
#define SX1276_MC3_AGCAUTO                 0x04

#define LORA_MAC_PREAMBLE                  0x34

#define RXLORA_RXMODE_RSSI_REG_MODEM_CONFIG1 0x0A
#ifdef LMIC_SX1276
#define RXLORA_RXMODE_RSSI_REG_MODEM_CONFIG2 0x70
#elif LMIC_SX1272
#define RXLORA_RXMODE_RSSI_REG_MODEM_CONFIG2 0x74
#endif

#define REG_FRF_MSB                  0x06
#define REG_FRF_MID                  0x07
#define REG_FRF_LSB                  0x08
#define FRF_MSB                       0xD9
#define FRF_MID                       0x06
#define FRF_LSB                       0x66

#define OPMODE_LORA      0x80
#define OPMODE_MASK      0x07
#define OPMODE_SLEEP     0x00
#define OPMODE_STANDBY   0x01
#define OPMODE_FSTX      0x02
#define OPMODE_TX        0x03
#define OPMODE_FSRX      0x04
#define OPMODE_RX        0x05
#define OPMODE_RX_SINGLE 0x06
#define OPMODE_CAD       0x07

#define IRQ_LORA_RXTOUT_MASK 0x80
#define IRQ_LORA_RXDONE_MASK 0x40
#define IRQ_LORA_CRCERR_MASK 0x20
#define IRQ_LORA_HEADER_MASK 0x10
#define IRQ_LORA_TXDONE_MASK 0x08
#define IRQ_LORA_CDDONE_MASK 0x04
#define IRQ_LORA_FHSSCH_MASK 0x02
#define IRQ_LORA_CDDETD_MASK 0x01

#define MAP_DIO0_LORA_RXDONE   0x00
#define MAP_DIO0_LORA_TXDONE   0x40
#define MAP_DIO1_LORA_RXTOUT   0x00
#define MAP_DIO1_LORA_NOP      0x30
#define MAP_DIO2_LORA_NOP      0xC0

typedef unsigned char byte;

static const int CHANNEL = 0;

char message[50];  
bool sx1272 = true;
byte receivedbytes;

enum sf_t { SF7=7, SF8, SF9, SF10, SF11, SF12 };

int ssPin = 6;
int dio0  = 7;
int RST   = 0;
sf_t sf = SF7;
uint32_t  freq = 868100000; 
byte hello[32] = "HELLO";

/* ################# FUNCIONES ################# */

void die(const char *s) {
    perror(s);
    exit(1);
}

void selectreceiver() { digitalWrite(ssPin, LOW); }
void unselectreceiver() { digitalWrite(ssPin, HIGH); }

byte readReg(byte addr) {
    unsigned char spibuf[2];
    selectreceiver();
    spibuf[0] = addr & 0x7F;
    spibuf[1] = 0x00;
    wiringPiSPIDataRW(CHANNEL, spibuf, 2);
    unselectreceiver();
    return spibuf[1];
}

void writeReg(byte addr, byte value) {
    unsigned char spibuf[2];
    spibuf[0] = addr | 0x80;
    spibuf[1] = value;
    selectreceiver();
    wiringPiSPIDataRW(CHANNEL, spibuf, 2);
    unselectreceiver();
}

static void opmode (uint8_t mode) {
    writeReg(REG_OPMODE, (readReg(REG_OPMODE) & ~OPMODE_MASK) | mode);
}

static void opmodeLora() {
    uint8_t u = OPMODE_LORA;
    if (!sx1272) u |= 0x8;  
    writeReg(REG_OPMODE, u);
}

void SetupLoRa() {
    digitalWrite(RST, HIGH);
    delay(100);
    digitalWrite(RST, LOW);
    delay(100);

    byte version = readReg(REG_VERSION);
    if (version == 0x22) {
        printf("SX1272 detected, starting.\n");
        sx1272 = true;
    } else {
        digitalWrite(RST, LOW);
        delay(100);
        digitalWrite(RST, HIGH);
        delay(100);
        version = readReg(REG_VERSION);
        if (version == 0x12) {
            printf("SX1276 detected, starting.\n");
            sx1272 = false;
        } else {
            printf("Unrecognized transceiver.\n");
            exit(1);
        }
    }

    opmode(OPMODE_SLEEP);
    uint64_t frf = ((uint64_t)freq << 19) / 32000000;
    writeReg(REG_FRF_MSB, (uint8_t)(frf>>16));
    writeReg(REG_FRF_MID, (uint8_t)(frf>>8));
    writeReg(REG_FRF_LSB, (uint8_t)(frf>>0));
    writeReg(REG_SYNC_WORD, 0x34);

    if (sx1272) {
        if (sf == SF11 || sf == SF12) writeReg(REG_MODEM_CONFIG,0x0B);
        else writeReg(REG_MODEM_CONFIG,0x0A);
        writeReg(REG_MODEM_CONFIG2,(sf<<4) | 0x04);
    } else {
        if (sf == SF11 || sf == SF12) writeReg(REG_MODEM_CONFIG3,0x0C);
        else writeReg(REG_MODEM_CONFIG3,0x04);
        writeReg(REG_MODEM_CONFIG,0x72);
        writeReg(REG_MODEM_CONFIG2,(sf<<4) | 0x04);
    }

    if (sf == SF10 || sf == SF11 || sf == SF12) writeReg(REG_SYMB_TIMEOUT_LSB,0x05);
    else writeReg(REG_SYMB_TIMEOUT_LSB,0x08);

    writeReg(REG_MAX_PAYLOAD_LENGTH,0x80);
    writeReg(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);
    writeReg(REG_HOP_PERIOD,0xFF);
    writeReg(REG_FIFO_ADDR_PTR, readReg(REG_FIFO_RX_BASE_AD));
    writeReg(REG_LNA, LNA_MAX_GAIN);
}

/* ############ MQTT ############ */
void setupMQTT() {
    int rc;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {   //En mqttpaho MQTTCLIENT_SUCCESS  es 0 (funcion se ejecuta sin error) (MQTTClient.h)
        printf("Failed to connect, return code %d. Reconnecting in 5 seconds...\n", rc);
        delay(5);
    }
    printf("MQTT connected successfully!\n");
}

void sendToMQTT(char* payload) {
    printf("\n Entrando a funcion sentToMQTT()\n");

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    pubmsg.payload = payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    int rc;
    while ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to publish message, return code %d. Trying to reconnect...\n", rc);
        while ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
            printf("Reconnect failed, return code %d. Retrying in 5 seconds...\n", rc);
            delay(5);
        }
        printf("Reconnected to MQTT broker.\n");
    }
    MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message sent: %s\n", payload);
}

/* ############ FUNCIONES DE RECEPCION ############ */
bool receive(char *payload) {

    /*
    //Debugeo
    printf("\n Entrando a funcion receive() \n")
    printf(payload)
    */

    writeReg(REG_IRQ_FLAGS, 0x40);
    int irqflags = readReg(REG_IRQ_FLAGS);
    if((irqflags & 0x20) == 0x20) {
        printf("CRC error\n");
        writeReg(REG_IRQ_FLAGS, 0x20);
        return false;
    } else {
        byte currentAddr = readReg(REG_FIFO_RX_CURRENT_ADDR);
        byte receivedCount = readReg(REG_RX_NB_BYTES);
        receivedbytes = receivedCount;
        writeReg(REG_FIFO_ADDR_PTR, currentAddr);
        for(int i = 0; i < receivedCount; i++) //VER QUITAR ESTE FOR
            payload[i] = (char)readReg(REG_FIFO);
    }
    return true;
}

bool receivepacket() {    //Modificaremos esto para controlar la llegada de nuevos paquetes por variable booleana (cambiamos de void a bool) (agrega returns booleanos)
    /*
    printf("\n Entrando a funcion receivepacket() \n")
    printf()
    */

    long int SNR;
    int rssicorr;
    if(digitalRead(dio0) == 1) {
        if(receive(message)) {
            byte value = readReg(REG_PKT_SNR_VALUE);
            if( value & 0x80 ) value = ( ( ~value + 1 ) & 0xFF ) >> 2, SNR = -value;
            else SNR = ( value & 0xFF ) >> 2;

            rssicorr = sx1272 ? 139 : 157;
            printf("Packet RSSI: %d, RSSI: %d, SNR: %li, Length: %i\n", readReg(0x1A)-rssicorr, readReg(0x1B)-rssicorr, SNR, (int)receivedbytes);
            printf("Payload: %s\n", message);

            
            // Limpiamos IRQ para que dio0 vuelva a LOW
            writeReg(REG_IRQ_FLAGS, IRQ_LORA_RXDONE_MASK);
            // Preparamos receptor para siguiente paquete
            opmode(OPMODE_RX);

            return true;
        }
        else{
            return false;
        }
    }
}

/* CONFIGURAR POTENCIA */
static void configPower (int8_t pw) {
    if (!sx1272) {
        if(pw >= 17) pw = 15;
        else if(pw < 2) pw = 2;
        writeReg(RegPaConfig, (uint8_t)(0x80|(pw&0xf)));
        writeReg(RegPaDac, readReg(RegPaDac)|0x4);
    } else {
        if(pw > 17) pw = 17;
        else if(pw < 2) pw = 2;
        writeReg(RegPaConfig, (uint8_t)(0x80|(pw-2)));
    }
}

static void writeBuf(byte addr, byte *value, byte len) {
    unsigned char spibuf[256];
    spibuf[0] = addr | 0x80;
    for (int i = 0; i < len; i++) spibuf[i + 1] = value[i];
    selectreceiver();
    wiringPiSPIDataRW(CHANNEL, spibuf, len + 1);
    unselectreceiver();
}

void txlora(byte *frame, byte datalen) {
    writeReg(RegDioMapping1, MAP_DIO0_LORA_TXDONE|MAP_DIO1_LORA_NOP|MAP_DIO2_LORA_NOP);
    writeReg(REG_IRQ_FLAGS, 0xFF);
    writeReg(REG_IRQ_FLAGS_MASK, ~IRQ_LORA_TXDONE_MASK);
    writeReg(REG_FIFO_TX_BASE_AD, 0x00);
    writeReg(REG_FIFO_ADDR_PTR, 0x00);
    writeReg(REG_PAYLOAD_LENGTH, datalen);
    writeBuf(REG_FIFO, frame, datalen);
    opmode(OPMODE_TX);
    printf("send: %s\n", frame);
}

/* ################# MAIN ################# */
int main (int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);  // <-- Desactiva el buffer del printf
    printf("Debug activado (stdout sin buffer)\n");
    if (argc < 2) {
        printf ("Usage: argv[0] sender|rec [message]\n");
        exit(1);
    }

    wiringPiSetup();
    pinMode(ssPin, OUTPUT);
    pinMode(dio0, INPUT);
    pinMode(RST, OUTPUT);
    wiringPiSPISetup(CHANNEL, 500000);

    SetupLoRa();
    setupMQTT();

    if (!strcmp("sender", argv[1])) {
        opmodeLora();
        opmode(OPMODE_STANDBY);
        writeReg(RegPaRamp, (readReg(RegPaRamp) & 0xF0) | 0x08);
        configPower(23);
        printf("Send packets at SF%i on %.6lf Mhz.\n", sf,(double)freq/1000000);
        printf("------------------\n");
        if (argc > 2) strncpy((char *)hello, argv[2], sizeof(hello));
        while(1) {
            txlora(hello, strlen((char *)hello));
            delay(5000);
        }
    } else {
        opmodeLora();
        opmode(OPMODE_STANDBY);
        opmode(OPMODE_RX);
        printf("Listening at SF%i on %.6lf Mhz.\n", sf,(double)freq/1000000);
        fflush(stdout);
        printf("debuggeando en else if not sender\n");
        fflush(stdout);
        printf("------------------\n");
        while(1) {
            printf("Mensaje enviado.\n");
            if(receivepacket()){ //Lo hacemos condicional para ver su valor booleano DEBEMOS PONER BIEN EL CAMPO
                //Buscaremos el ultimo campo y asignaremos valor a la variable auxiliar, despues compararemos y se mandara solo si es distinto
                //Usamos la funcion strrchr() para encontrar la ocurrencia del ultimo caracter dado (UBICAREMOS LA ULTIMA COMA ",")
                char* Ultima_coma = strrchr(message, ','); 
                //strrchr(message, ',')  Devuelve un puntero a la ultima coma, por lo que Ultima_coma + 1 es un puntero al primer bit del ultimo campo
                if(Ultima_coma!=NULL){
                    uint32_t current_timestamp = strtoul(Ultima_coma+ 1, NULL, 10); // Convertir un string (lo q esta despues de la coma) a un num entero sin signo
                    //Si bien Ultima_coma apunta al primer caracter del numero, strtoul interpreta toda la secuencia de caracteres consecutivos como un 
                    // numero completo, no solo el primer caracter. (Hasta que encuentra la siguiente coma)

                    if(current_timestamp != Last_Time_Stamp){ //La fecha queda de 4 bytes
                        //Se envia
                        sendToMQTT(message);
                        Last_Time_Stamp = current_timestamp;
                        printf("Mensaje enviado.\n");


                    }
                    else{
                        printf("Mensaje duplicado, no se envia.\n");
                    }
                }

                else{
                    printf("Error: No se pudo parsear Timestamp");
                }
                    
                
            }
            delay(100);
        }
    }

    return 0;
}


//Recuerda: Tenemos que asignar el valor del actual timestamp a la variable auxiliar Last_Time_Stamp