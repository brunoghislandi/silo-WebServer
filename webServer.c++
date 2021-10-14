/**
 * Trabalho da discplina de IOT2 (Faculdade UNISATC - Engenharia de Computação)
 * com objetivo de simular o armazenamento de um Silo que possui 1m de altura
 * e 3m de diâmetro.
 * 
 * Foi disponibilizado o uso de um sensor ultrassônico e um sensor LDR para
 * realizar as medições e lógicas de programação com base no critério desejado
 * pelo professor.
 * 
 * @board       ESP8266
 */

String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

/**
 * @brief   HTML utilizado para visualização em navegadores (Mobile e WEB)
 */

String html_1 = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv='refresh' content='1'>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IOT 2</title>
    <style>
        body,
        html {
            height: 100%;
            text-align: center;
        }

        body {
            display: flex;
            justify-content: center;
            background-color: skyblue;
        }

        section {
            margin-top: 30px;
            background: aliceblue;
            border-radius: 10px;
            padding: 15px;
            width: 800px;
            height: 300px;   
            box-shadow: 5px 5px 10px rgb(59, 59, 59);
        }

        header {
            font: normal 15pt Arial;
            color: black;
            text-align: center;
        }

        div {
            display: inline-block;
            width: 300px;
            height: 50px;
            background: rgb(135, 151, 238);
            font: normal 20pt Arial;
            color: rgb(59, 59, 59);
            margin-bottom: 7.5px;
            margin-left: 20px;
            margin-right: 20px;
            border-radius: 14px;
            font: normal 15pt Arial;
            line-height: 50px;
        }
        p{
          margin-top: 37px;
          font: normal 15pt Arial;
        }
    </style>
</head>

<body>
    <section>
        <header>
            <h1>Monitoramento do Silo</h1>
            <p>Volume: %volume%m³</p>
        </header>
        <div>Nível: %porcentagem%%</div>
        <div>%temp%</div>
        <footer>
          <p>%estado%</p>
        </footer>
    </section>
</body>

</html>
)=====";

/* Inclusão de Bibliotecas */
#include <ESP8266WiFi.h>

char ssid[] = "redeID";
char pass[] = "password";

WiFiServer server(80);

/* Definições e Constantes */
#define TRIGGER D6
#define ECHO D7
int LEDs[3] = {D8, D9, D10};
long actived;
float distance,
    volume,
    LDR,
    porcentagem;
String tmpString = "";
String estado = "Capacidade: Normal";
String temp = "Lâmpada: Ligada";
unsigned long millisBASE = millis();

/**
 * @brief   Função utilizada para medir a distância do sensor Ultrassônico (HC-SR04) 
 * 
 * 
 * @return      Print("Volume do Silo, Porcentagem de Armazenamento")
 */

void ReadUltrasonic()
{
    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER, LOW);
    actived = pulseIn(ECHO, HIGH);
    distance = actived * 0.034 / 2; //Conversão para centímetros (cm)
    volume = (9.4247779608 - ((distance / 100) * 3.1415926536 * 3)); //Conversão para volume (m³)
    porcentagem = (100 * volume) / 9.4247779608;
    Serial.print("Volume: ");
    Serial.print(volume);
    Serial.println(" m³");
    Serial.print("Porcentagem: ");
    Serial.print(porcentagem);
    Serial.println(" %");
}

/**
 * @brief   Função utilizada para medir a luminosidade do sensor LDR.  
 * 
 * 
 * @return      luminosidade do silo
 */

void ReadLDR()
{
    LDR = analogRead(A0);
    Serial.print("LDR: ");
    Serial.print(LDR);
    Serial.println(" Lumen");
}

/**
 * @brief   Função utilizada para realizar a lógica de atuação com base no volume do Silo
 * 
 * 
 * @return      volume maior que 5m³ = Print("Capacidade Normal")
 *              volume menor que 5m³ = Print("Capacidade Crítica")
 *                                     LED BUILTIN piscando
 */

void nivelCheck()
{
    if (volume < 5)
    {
        estado = "Capacidade: Crítica";
        if ((millis() - millisBASE) < 100)
        {
            digitalWrite(LED_BUILTIN, HIGH);
        }
        else
        {
            digitalWrite(LED_BUILTIN, LOW);
        }
        if ((millis() - millisBASE) > 200)
        {
            millisBASE = millis();
        }
    }
    else
    {
        estado = "Capacidade: Normal";
        digitalWrite(LED_BUILTIN, LOW);
    }
}

/**
 * @brief   Função utilizada para realizar a lógica de atuação com base na luminosidade do Silo
 * 
 * 
 * @return      luminosidade maior que 950 lúmens = Print("Lâmpada: Ligada")
 *                                                  LED Verde Aceso (OK)
 *              luminosidade menor que 950 lúmens = Print("Lâmpada: Desligada")
 *                                                  LED Vermelho Aceso (Aviso)
 */

void lumenCheck()
{
    if (LDR > 950)
    {
        temp = "Lâmpada: Desligada";
        digitalWrite(LEDs[2], HIGH);
        digitalWrite(LEDs[1], LOW);
    }
    else
    {
        temp = "Lâmpada: Ligada";
        digitalWrite(LEDs[2], LOW);
        digitalWrite(LEDs[1], HIGH);
    }
}

void setup()
{
    for (int i = 0; i < 3; i++)
    {
        pinMode(LEDs[i], OUTPUT);
    }
    pinMode(TRIGGER, OUTPUT);
    pinMode(ECHO, INPUT);

    Serial.begin(115200);

    Serial.print(F("Connecting to "));
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println(F("[CONNECTED]"));
    Serial.print("[IP ");
    Serial.print(WiFi.localIP());
    Serial.println("]");

    server.begin();
}

void loop()
{
    ReadUltrasonic();
    ReadLDR();
    nivelCheck();
    lumenCheck();

    WiFiClient client = server.available();
    if (!client)
    {
        return;
    }

    /*
    * Funções de replace para realizar a substituição do HTML
    * pelas variáveis utilizadas no código.
    */

    tmpString = html_1;
    tmpString.replace("%volume%", String(volume));
    tmpString.replace("%estado%", estado);
    tmpString.replace("%temp%", temp);
    tmpString.replace("%porcentagem%", String(porcentagem));

    client.flush();
    client.print(header);
    client.print(tmpString);

    delay(5);
}