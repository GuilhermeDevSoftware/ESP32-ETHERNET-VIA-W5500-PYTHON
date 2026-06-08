#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

// ==================================================
// PINOS DO W5500
// ==================================================

#define PINO_SCK   18
#define PINO_MISO  19
#define PINO_MOSI  23
#define PINO_CS     5
#define PINO_RST   26

// ==================================================
// DHT11
// ==================================================

#define PINO_DHT  27
#define TIPO_DHT  DHT11

DHT dht(PINO_DHT, TIPO_DHT);

// ==================================================
// LED
// ==================================================

#define PINO_LED 32

bool estadoLed = false;

// ==================================================
// DADOS DO SENSOR
// ==================================================

float temperatura = 0.0;
float umidade = 0.0;

bool leituraDhtValida = false;

unsigned long ultimaLeituraDht = 0;

const unsigned long INTERVALO_DHT = 2000;

// ==================================================
// ETHERNET
// ==================================================

byte mac[] = {
  0x02, 0x12, 0x34, 0x56, 0x78, 0x90
};

EthernetServer servidor(80);

// ==================================================
// REINICIAR W5500
// ==================================================

void reiniciarW5500() {
  pinMode(PINO_RST, OUTPUT);

  digitalWrite(PINO_RST, LOW);
  delay(200);

  digitalWrite(PINO_RST, HIGH);
  delay(500);
}

// ==================================================
// ATUALIZAR DHT11
// ==================================================

void atualizarDht11() {
  unsigned long agora = millis();

  if (
    ultimaLeituraDht == 0 ||
    agora - ultimaLeituraDht >= INTERVALO_DHT
  ) {
    ultimaLeituraDht = agora;

    float novaTemperatura = dht.readTemperature();
    float novaUmidade = dht.readHumidity();

    if (
      !isnan(novaTemperatura) &&
      !isnan(novaUmidade)
    ) {
      temperatura = novaTemperatura;
      umidade = novaUmidade;

      leituraDhtValida = true;
    } else {
      Serial.println("Falha ao ler o DHT11.");
    }
  }
}

// ==================================================
// CABEÇALHO HTML
// ==================================================

void enviarCabecalhoHtml(EthernetClient &cliente) {
  cliente.println("HTTP/1.1 200 OK");
  cliente.println("Content-Type: text/html; charset=UTF-8");
  cliente.println("Cache-Control: no-store");
  cliente.println("Connection: close");
  cliente.println();
}

// ==================================================
// PÁGINA PRINCIPAL
// ==================================================

void enviarPaginaPrincipal(EthernetClient &cliente) {
  atualizarDht11();

  enviarCabecalhoHtml(cliente);

  cliente.println("<!DOCTYPE html>");
  cliente.println("<html lang='pt-BR'>");

  cliente.println("<head>");
  cliente.println("<meta charset='UTF-8'>");
  cliente.println(
    "<meta name='viewport' "
    "content='width=device-width, initial-scale=1.0'>"
  );

  cliente.println("<title>ESP32 Ethernet</title>");

  cliente.println("<style>");

  cliente.println(
    "body {"
    "font-family: Arial, sans-serif;"
    "background-color: #eeeeee;"
    "text-align: center;"
    "margin: 0;"
    "padding: 40px 10px;"
    "}"
  );

  cliente.println(
    ".painel {"
    "background-color: white;"
    "max-width: 400px;"
    "margin: auto;"
    "padding: 25px;"
    "border-radius: 12px;"
    "box-shadow: 0 2px 10px rgba(0,0,0,0.2);"
    "}"
  );

  cliente.println(
    ".valor {"
    "font-size: 20px;"
    "margin: 18px 0;"
    "}"
  );

  cliente.println(
    ".botao {"
    "display: inline-block;"
    "padding: 15px 22px;"
    "margin: 8px;"
    "color: white;"
    "text-decoration: none;"
    "border-radius: 8px;"
    "font-size: 18px;"
    "}"
  );

  cliente.println(
    ".ligar {"
    "background-color: green;"
    "}"
  );

  cliente.println(
    ".desligar {"
    "background-color: red;"
    "}"
  );

  cliente.println(
    ".status {"
    "display: inline-block;"
    "margin-top: 18px;"
    "}"
  );

  cliente.println("</style>");
  cliente.println("</head>");

  cliente.println("<body>");
  cliente.println("<div class='painel'>");

  cliente.println("<h1>ESP32 + W5500</h1>");
  cliente.println("<h2>Servidor Ethernet</h2>");

  cliente.print("<div class='valor'>Temperatura: ");

  if (leituraDhtValida) {
    cliente.print(temperatura, 1);
    cliente.print(" &deg;C");
  } else {
    cliente.print("erro na leitura");
  }

  cliente.println("</div>");

  cliente.print("<div class='valor'>Umidade: ");

  if (leituraDhtValida) {
    cliente.print(umidade, 1);
    cliente.print(" %");
  } else {
    cliente.print("erro na leitura");
  }

  cliente.println("</div>");

  cliente.print("<div class='valor'>LED: <strong>");

  if (estadoLed) {
    cliente.print("LIGADO");
  } else {
    cliente.print("DESLIGADO");
  }

  cliente.println("</strong></div>");

  cliente.println(
    "<a class='botao ligar' "
    "href='/led/ligar'>Ligar LED</a>"
  );

  cliente.println(
    "<a class='botao desligar' "
    "href='/led/desligar'>Desligar LED</a>"
  );

  cliente.println("<br>");

  cliente.println(
    "<a class='status' href='/status'>"
    "Visualizar JSON"
    "</a>"
  );

  cliente.println("</div>");
  cliente.println("</body>");
  cliente.println("</html>");
}

// ==================================================
// RESPOSTA JSON
// ==================================================

void enviarStatusJson(EthernetClient &cliente) {
  atualizarDht11();

  cliente.println("HTTP/1.1 200 OK");
  cliente.println(
    "Content-Type: application/json; charset=UTF-8"
  );
  cliente.println("Access-Control-Allow-Origin: *");
  cliente.println("Cache-Control: no-store");
  cliente.println("Connection: close");
  cliente.println();

  cliente.println("{");

  cliente.print("  \"temperatura\": ");

  if (leituraDhtValida) {
    cliente.print(temperatura, 1);
  } else {
    cliente.print("null");
  }

  cliente.println(",");

  cliente.print("  \"umidade\": ");

  if (leituraDhtValida) {
    cliente.print(umidade, 1);
  } else {
    cliente.print("null");
  }

  cliente.println(",");

  cliente.print("  \"led\": \"");

  if (estadoLed) {
    cliente.print("ligado");
  } else {
    cliente.print("desligado");
  }

  cliente.println("\",");

  cliente.println("  \"conexao\": \"ethernet\"");

  cliente.println("}");
}

// ==================================================
// REDIRECIONAR PARA A PÁGINA PRINCIPAL
// ==================================================

void redirecionarPaginaPrincipal(
  EthernetClient &cliente
) {
  cliente.println("HTTP/1.1 303 See Other");
  cliente.println("Location: /");
  cliente.println("Cache-Control: no-store");
  cliente.println("Connection: close");
  cliente.println();
}

// ==================================================
// ERRO 404
// ==================================================

void enviarErro404(EthernetClient &cliente) {
  cliente.println("HTTP/1.1 404 Not Found");
  cliente.println(
    "Content-Type: text/plain; charset=UTF-8"
  );
  cliente.println("Connection: close");
  cliente.println();

  cliente.println("Rota nao encontrada.");
}

// ==================================================
// PROCESSAR REQUISIÇÃO
// ==================================================

void processarRequisicao(
  EthernetClient &cliente,
  const String &requisicao
) {
  Serial.print("Requisicao recebida: ");
  Serial.println(requisicao);

  if (requisicao.startsWith("GET /status ")) {
    enviarStatusJson(cliente);
  }

  else if (
    requisicao.startsWith("GET /led/ligar ")
  ) {
    estadoLed = true;

    digitalWrite(PINO_LED, HIGH);

    Serial.println("LED ligado.");

    redirecionarPaginaPrincipal(cliente);
  }

  else if (
    requisicao.startsWith("GET /led/desligar ")
  ) {
    estadoLed = false;

    digitalWrite(PINO_LED, LOW);

    Serial.println("LED desligado.");

    redirecionarPaginaPrincipal(cliente);
  }

  else if (
    requisicao.startsWith("GET / ")
  ) {
    enviarPaginaPrincipal(cliente);
  }

  else {
    enviarErro404(cliente);
  }
}

// ==================================================
// ATENDER CLIENTE
// ==================================================

void atenderCliente(EthernetClient &cliente) {
  String primeiraLinha = "";

  bool primeiraLinhaCompleta = false;
  bool linhaEmBranco = false;

  unsigned long inicio = millis();

  while (
    cliente.connected() &&
    millis() - inicio < 2000
  ) {
    while (cliente.available()) {
      char caractere = cliente.read();

      if (!primeiraLinhaCompleta) {
        if (caractere == '\n') {
          primeiraLinhaCompleta = true;
        }

        else if (caractere != '\r') {
          if (primeiraLinha.length() < 150) {
            primeiraLinha += caractere;
          }
        }
      }

      if (
        caractere == '\n' &&
        linhaEmBranco
      ) {
        processarRequisicao(
          cliente,
          primeiraLinha
        );

        return;
      }

      if (caractere == '\n') {
        linhaEmBranco = true;
      }

      else if (caractere != '\r') {
        linhaEmBranco = false;
      }
    }

    delay(1);
  }

  Serial.println(
    "Tempo limite da requisicao excedido."
  );
}

// ==================================================
// SETUP
// ==================================================

void setup() {
  Serial.begin(115200);

  delay(1000);

  pinMode(PINO_LED, OUTPUT);

  digitalWrite(PINO_LED, LOW);

  estadoLed = false;

  dht.begin();

  Serial.println();
  Serial.println(
    "Iniciando ESP32 + W5500..."
  );

  reiniciarW5500();

  SPI.begin(
    PINO_SCK,
    PINO_MISO,
    PINO_MOSI,
    PINO_CS
  );

  Ethernet.init(PINO_CS);

  Serial.println(
    "Procurando endereco IP via DHCP..."
  );

  if (Ethernet.begin(mac) == 0) {
    Serial.println(
      "Falha ao obter IP via DHCP."
    );

    Serial.println(
      "Verifique o cabo Ethernet e o roteador."
    );

    return;
  }

  delay(1000);

  servidor.begin();

  Serial.println();
  Serial.println(
    "Servidor Ethernet iniciado!"
  );

  Serial.print(
    "Pagina principal: http://"
  );

  Serial.println(Ethernet.localIP());

  Serial.print(
    "Status JSON: http://"
  );

  Serial.print(Ethernet.localIP());

  Serial.println("/status");

  Serial.print(
    "Ligar LED: http://"
  );

  Serial.print(Ethernet.localIP());

  Serial.println("/led/ligar");

  Serial.print(
    "Desligar LED: http://"
  );

  Serial.print(Ethernet.localIP());

  Serial.println("/led/desligar");
}

// ==================================================
// LOOP
// ==================================================

void loop() {
  Ethernet.maintain();

  atualizarDht11();

  EthernetClient cliente = servidor.available();

  if (cliente) {
    atenderCliente(cliente);

    while (cliente.available()) {
      cliente.read();
    }

    delay(5);

    cliente.stop();

    Serial.println("Cliente desconectado.");
    Serial.println();
  }
}