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
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>IOT 2</title>
        <style>
            body,
            html {
                height: 100%;
                text-align: center;
            }
            body {
                font: normal 16px/21px Arial, Helvetica, sans-serif;
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
                max-width:  100%;
                height: 300px;
                box-shadow: 5px 6px 10px rgb(59, 59, 59, 0.5);
            }
            header {
                color: black;
                text-align: center;
            }
            section > div {
                display: inline-block;
                width: 300px;
                height: 50px;
                background: rgb(135, 151, 238);
                color: rgb(59, 59, 59);
                margin: 6px 20px 7.5px;
                border-radius: 14px;
                font-size: 18px;
                font-weight: bold;
                line-height: 50px;
            }
            p {
                margin-top: 37px;
            }
            #loading {
                position: absolute;
                left: 50%;
                top: 10.5%;
                margin-left: 320px;
                display: none;
            }
            .lds-ripple {
                display: inline-block;
                position: relative;
                width: 80px;
                height: 80px;
            }
            .lds-ripple div {
                position: absolute;
                border: 4px solid rgb(135, 151, 238);
                opacity: 1;
                border-radius: 50%;
                animation: lds-ripple 1s cubic-bezier(0, 0.2, 0.8, 1) infinite;
            }
            .lds-ripple div:nth-child(2) {
                animation-delay: -0.5s;
            }
            @keyframes lds-ripple {
                0% {
                    top: 36px;
                    left: 36px;
                    width: 0;
                    height: 0;
                    opacity: 1;
                }
                100% {
                    top: 0px;
                    left: 0px;
                    width: 72px;
                    height: 72px;
                    opacity: 0;
                }
            }
        </style>
    </head>
    <body>
        <div id="loading">
            <div class="lds-ripple">
                <div></div><div></div>
            </div>
        </div>
        <div id="main">
            <section>
                <header>
                    <h1>Monitoramento do Silo</h1>
                    <p>Volume: %volume% m³</p>
                </header>
                <div>Nível: %porcentagem%%</div>
                <div>%temp%</div>
                <footer>
                    <p>%estado%</p>
                </footer>
            </section>
        </div>
        <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
        <script>
            // essa função, via um "ajax" magico do jQuery recarrega a página apenas "um pedaço"
            // e substitui em tela por este pedaço mais "atualizado"
            function reloadSection(onComplete) {
                var $url = window.location.href + "?r=" + new Date().getTime();
                $("#main").load($url + " #main > section", onComplete);
            }

            // essa função chama a anterior e ao final de uma execução com sucesso, programa
            // uma próxima execução para daqui a 2 segundos...
            function timeoutReload() {
                $("#loading").show();
                reloadSection(function () {
                    $("#loading").hide();
                    setTimeout(timeoutReload, 2000); // 2000 = 2 segundos...
                });
            }

            // aqui chamamos a primeira vez que irá ser executada e agendar as próximas
            // de 2 em 2 segundos...
            timeoutReload();
        </script>
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
