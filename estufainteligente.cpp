#include <LiquidCrystal.h> 

#include <Servo.h> 

  

// LCD: RS, E, D4, D5, D6, D7 

LiquidCrystal lcd(7, 8, 9, 10, 11, 12); 

Servo janela; 

  

// Pinos dos sensores 

const int sensorTempPin = A3;       // TMP36 

const int sensorSoloPin = A0;       // Umidade do solo 

const int sensorArPin = A1;         // Umidade do ar (simulado) 

const int ldrPin = A2;              // Fotoresistor 

const int botaoPin = A4;            // Botão modo automático/manual 

  

// Pinos dos atuadores 

const int aquecedorPin = 3; 

const int luzCrescimentoPin = 4; 

const int relePin = 5; 

const int servoPin = 6; 

const int alertaPin = 13; 

  

// Parâmetros ideais 

const float TEMP_IDEAL_MIN = 22.0; 

const float TEMP_IDEAL_MAX = 28.0; 

const float UMID_AR_IDEAL_MIN = 60.0; 

const int UMID_SOLO_SECO = 400; 

const int LUZ_IDEAL_MIN = 600; 

  

bool modoAutomatico = true; 

unsigned long ultimoMonitoramento = 0; 

const unsigned long intervaloMonitoramento = 500; // 500 ms para atualização rápida 

  

// Variáveis para debounce do botão 

int botaoEstadoAtual = HIGH; 

int botaoEstadoAnterior = HIGH; 

unsigned long ultimoDebounceTime = 0; 

const unsigned long debounceDelay = 10; // 10 ms debounce 

  

// Controle de mensagem de modo no LCD 

unsigned long tempoMensagemModo = 0; 

const unsigned long duracaoMensagemModo = 1000; // 1 segundo mensagem modo 

bool mostrandoMensagemModo = false; 

  

void setup() { 

  lcd.begin(16, 2); 

  janela.attach(servoPin); 

  

  pinMode(aquecedorPin, OUTPUT); 

  pinMode(luzCrescimentoPin, OUTPUT); 

  pinMode(relePin, OUTPUT); 

  pinMode(alertaPin, OUTPUT); 

  pinMode(botaoPin, INPUT_PULLUP); 

  

  Serial.begin(9600); 

  

  // Mensagem inicial no LCD dividida em duas linhas para não cortar 

  lcd.clear(); 

  lcd.setCursor(0, 0); 

  lcd.print("Estufa"); 

  lcd.setCursor(0, 1); 

  lcd.print("Inteligente"); 

  delay(1500); 

  lcd.clear(); 

  

  atualizarDisplayInicial(); 

} 

  

void loop() { 

  // Leitura e debounce do botão 

  int leitura = digitalRead(botaoPin); 

  if (leitura != botaoEstadoAnterior) { 

    ultimoDebounceTime = millis(); 

  } 

  if ((millis() - ultimoDebounceTime) > debounceDelay) { 

    if (leitura != botaoEstadoAtual) { 

      botaoEstadoAtual = leitura; 

      if (botaoEstadoAtual == LOW) { // Botão pressionado 

        modoAutomatico = !modoAutomatico; 

        mostrarMensagemModo(modoAutomatico); 

      } 

    } 

  } 

  botaoEstadoAnterior = leitura; 

  

  unsigned long tempoAtual = millis(); 

  

  // Atualizar sensores e atuadores se não estiver mostrando mensagem modo 

  if (!mostrandoMensagemModo && (tempoAtual - ultimoMonitoramento >= intervaloMonitoramento)) { 

    ultimoMonitoramento = tempoAtual; 

  

    int leituraAnalogica = analogRead(sensorTempPin); 

    float tensao = leituraAnalogica * (5.0 / 1023.0); 

    float temperatura = (tensao - 0.5) * 100.0; 

  

    int umidadeSolo = analogRead(sensorSoloPin); 

    int umidadeAr = analogRead(sensorArPin); 

    int luz = analogRead(ldrPin); 

    float umidadeArPercentual = map(umidadeAr, 0, 1023, 0, 100); 

  

    if (modoAutomatico) { 

      controlarTemperatura(temperatura); 

      controlarIrrigacao(umidadeSolo); 

      controlarIluminacao(luz); 

    } else { 

      // No modo manual, você pode adicionar controle via Serial se quiser 

    } 

  

    atualizarDisplay(temperatura, umidadeArPercentual, umidadeSolo, luz); 

    verificarAlerta(temperatura); 

  } 

  

  // Controla o tempo que a mensagem do modo fica no LCD 

  if (mostrandoMensagemModo && (tempoAtual - tempoMensagemModo >= duracaoMensagemModo)) { 

    mostrandoMensagemModo = false; 

    lcd.clear(); 

    atualizarDisplayInicial(); 

  } 

} 

  

void mostrarMensagemModo(bool automatico) { 

  mostrandoMensagemModo = true; 

  tempoMensagemModo = millis(); 

  lcd.clear(); 

  lcd.setCursor(0, 0); 

  if (automatico) { 

    lcd.print("Modo Automatico"); 

  } else { 

    lcd.print("Modo Manual"); 

  } 

  lcd.setCursor(0, 1); 

  lcd.print("Ativado"); 

} 

  

void controlarTemperatura(float temperatura) { 

  if (temperatura > TEMP_IDEAL_MAX) { 

    digitalWrite(relePin, HIGH); 

    digitalWrite(aquecedorPin, LOW); 

    janela.write(90); 

  } else if (temperatura < TEMP_IDEAL_MIN) { 

    digitalWrite(aquecedorPin, HIGH); 

    digitalWrite(relePin, LOW); 

    janela.write(0); 

  } else { 

    digitalWrite(aquecedorPin, LOW); 

    digitalWrite(relePin, LOW); 

    janela.write(0); 

  } 

} 

  

void controlarIrrigacao(int umidadeSolo) { 

  if (umidadeSolo < UMID_SOLO_SECO) { 

    Serial.println("ACIONANDO BOMBA DE IRRIGACAO"); 

    // Controle da bomba sem delay pode ser implementado aqui 

  } 

} 

  

void controlarIluminacao(int luz) { 

  if (luz < LUZ_IDEAL_MIN) { 

    digitalWrite(luzCrescimentoPin, HIGH); 

  } else { 

    digitalWrite(luzCrescimentoPin, LOW); 

  } 

} 

  

void atualizarDisplay(float temperatura, float umidadeAr, int umidadeSolo, int luz) { 

  lcd.clear(); 

  lcd.setCursor(0, 0); 

  lcd.print("T:"); 

  lcd.print(temperatura, 1); 

  lcd.print("C U.A:"); 

  lcd.print(umidadeAr, 0); 

  lcd.print("%"); 

  

  lcd.setCursor(0, 1); 

  lcd.print("U.S:"); 

  lcd.print(umidadeSolo); 

  lcd.print(" L:"); 

  lcd.print(luz); 

} 

  

void atualizarDisplayInicial() { 

  lcd.setCursor(0, 0); 

  lcd.print("Aguardando dados"); 

  lcd.setCursor(0, 1); 

  lcd.print("sensores..."); 

} 

  

void verificarAlerta(float temperatura) { 

  if (temperatura > TEMP_IDEAL_MAX || temperatura < TEMP_IDEAL_MIN) { 

    digitalWrite(alertaPin, HIGH); 

  } else { 

    digitalWrite(alertaPin, LOW); 

  } 

} 