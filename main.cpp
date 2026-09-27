#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <BleKeyboard.h>

// ================= PINAGEM ESP32 (30 PINOS) =================
const int PIN_POT        = 36; // GPIO36 (VP - Potenciômetro ADC1)
const int PIN_BOTAO      = 32; // GPIO32 (Push Button com INPUT_PULLUP)
const int PIN_LASER      = 33; // GPIO33 (Módulo Mini Laser)
const int PIN_BUZZER     = 25; // GPIO25 (Buzzer Piezoelétrico)
const int PIN_LED_VERDE  = 26; // GPIO26 (LED Verde - Concluído/Pronto)
const int PIN_LED_AMAR   = 27; // GPIO27 (LED Amarelo - Girando/Estabilizando)
const int PIN_LED_VERM   = 14; // GPIO14 (LED Vermelho - Captura de Foto)

// Driver ULN2003 (Motor de Passo 28BYJ-48)
const int PIN_IN1        = 19;
const int PIN_IN2        = 18;
const int PIN_IN3        = 5;
const int PIN_IN4        = 17;

// Display LCD 16x2 I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Motor 28BYJ-48 (2048 passos por volta completa)
const int PASSOS_VOLTA_COMPLETA = 2048;
Stepper motorPasso(PASSOS_VOLTA_COMPLETA, PIN_IN1, PIN_IN3, PIN_IN2, PIN_IN4);

// ============ BLUETOOTH BLE SHUTTER ============
BleKeyboard bleKeyboard("Scanner 3D Shutter", "ESP32", 100);

// ============ MÁQUINA DE ESTADOS ============
enum EstadoSistema {
  PARADO,
  EXECUTANDO,
  PAUSADO
};

EstadoSistema estadoAtual = PARADO;

// Variáveis de controle
int totalPassos = 24;
int fotoAtual = 1;
long passosAcumulados = 0;
int anguloAtual = 0;

bool ultimoEstadoBotao = HIGH;
unsigned long tempoPrimeiroCliquePausa = 0;
int cliquesNaPausa = 0;
const unsigned long JANELA_DUPLO_CLIQUE = 450;

int ultimaLeituraPot = -999;
bool ultimoStatusBle = false;
unsigned long tempoAtualizacaoPot = 0;

// Protótipos das funções
void definirLeds(bool vermelho, bool amarelo, bool verde);
void desenergizarMotor();
void bipFoto();
void bipFimCiclo();
bool detectarClique();
void atualizarPassosPotenciometro(bool forcarExibicao);
bool aguardarComChecagemBotao(int tempoMs);
void gerenciarBotoesPausa();
void retornarAoPontoZero();
void dispararCameraCelular();

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMAR, OUTPUT);
  pinMode(PIN_LED_VERM, OUTPUT);
  pinMode(PIN_LASER, OUTPUT);
  
  pinMode(PIN_BOTAO, INPUT_PULLUP);
  delay(100);
  ultimoEstadoBotao = digitalRead(PIN_BOTAO);

  motorPasso.setSpeed(12);

  // Inicializa LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FOTOGRAMETRIA 3D");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando BLE...");
  
  // Inicia o serviço Bluetooth BLE
  bleKeyboard.begin();
  delay(1200);

  definirLeds(LOW, LOW, HIGH); // Verde
  digitalWrite(PIN_LASER, HIGH);
  desenergizarMotor();

  atualizarPassosPotenciometro(true);
}

// ================= LOOP PRINCIPAL =================
void loop() {
  switch (estadoAtual) {
    case PARADO:
      definirLeds(LOW, LOW, HIGH);
      digitalWrite(PIN_LASER, HIGH);
      atualizarPassosPotenciometro(false);
      
      // Clique simples para INICIAR
      if (detectarClique()) {
        tone(PIN_BUZZER, 1200, 150);
        digitalWrite(PIN_LASER, LOW);
        
        passosAcumulados = 0;
        fotoAtual = 1;
        estadoAtual = EXECUTANDO;
      }
      break;

    case EXECUTANDO:
      digitalWrite(PIN_LASER, LOW);

      while (fotoAtual <= totalPassos) {
        // 1. Calcula passos acumulados absolutos do motor
        long posicaoAlvoPassos = round((fotoAtual - 1) * ((float)PASSOS_VOLTA_COMPLETA / totalPassos));
        long passosParaGirar = posicaoAlvoPassos - passosAcumulados;

        if (passosParaGirar > 0) {
          definirLeds(LOW, HIGH, LOW); // Amarelo (Girando)
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("GIRANDO MESA...");
          lcd.setCursor(0, 1);
          lcd.print("Passo: +");
          lcd.print(passosParaGirar);
          lcd.print(" stp");

          motorPasso.step(passosParaGirar);
          passosAcumulados += passosParaGirar;
          desenergizarMotor();
        }

        // 2. Atualiza LCD com foto e status do Bluetooth
        anguloAtual = round((passosAcumulados / (float)PASSOS_VOLTA_COMPLETA) * 360.0);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("FOTO: ");
        lcd.print(fotoAtual);
        lcd.print("/");
        lcd.print(totalPassos);

        lcd.setCursor(0, 1);
        lcd.print(anguloAtual);
        lcd.print((char)223); // Símbolo de grau
        lcd.print(bleKeyboard.isConnected() ? " [BLE:ON]" : " [BLE:--]");

        // 3. Estabilização mecânica (600ms para zerar trepidações)
        definirLeds(LOW, HIGH, LOW);
        if (!aguardarComChecagemBotao(600)) return;

        // 4. DISPARO DA FOTO (VIA BLUETOOTH BLE + LED Vermelho + Bip)
        definirLeds(HIGH, LOW, LOW); // Vermelho
        bipFoto();
        dispararCameraCelular();

        // 5. Intervalo pós-foto (1.2s para a câmera do celular processar)
        if (!aguardarComChecagemBotao(1200)) return;

        fotoAtual++;
      }

      // 6. Fecha volta completa de 360° no Ponto 0
      if (passosAcumulados < PASSOS_VOLTA_COMPLETA) {
        long restoParaZero = PASSOS_VOLTA_COMPLETA - passosAcumulados;
        motorPasso.step(restoParaZero);
        passosAcumulados += restoParaZero;
      }
      desenergizarMotor();

      bipFimCiclo();
      definirLeds(LOW, LOW, HIGH); // Verde
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SCAN CONCLUIDO!");
      lcd.setCursor(0, 1);
      lcd.print("100% NO PONTO 0");
      delay(2500);

      passosAcumulados = 0;
      fotoAtual = 1;
      estadoAtual = PARADO;
      atualizarPassosPotenciometro(true);
      break;

    case PAUSADO:
      definirLeds(LOW, HIGH, LOW);
      desenergizarMotor();
      gerenciarBotoesPausa();
      break;
  }
}

// ============= DISPARADOR BLUETOOTH BLE =============
void dispararCameraCelular() {
  if (bleKeyboard.isConnected()) {
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    Serial.println(">>> FOTO DISPARADA VIA BLUETOOTH <<<");
  } else {
    Serial.println("Aviso: Bluetooth desconectado (foto ignorada via BLE)");
  }
}

// ============= FUNÇÕES AUXILIARES =============
void retornarAoPontoZero() {
  if (passosAcumulados > 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RETORNANDO...");
    lcd.setCursor(0, 1);
    lcd.print("Voltando p/ 0");

    motorPasso.setSpeed(8); // Velocidade suave no retorno
    motorPasso.step(-passosAcumulados);
    motorPasso.setSpeed(12);

    passosAcumulados = 0;
    desenergizarMotor();
  }
}

void atualizarPassosPotenciometro(bool forcarExibicao) {
  if (!forcarExibicao && (millis() - tempoAtualizacaoPot < 120)) return;
  tempoAtualizacaoPot = millis();

  bool statusBleAtual = bleKeyboard.isConnected();

  long soma = 0;
  for (int i = 0; i < 10; i++) {
    soma += analogRead(PIN_POT);
    delayMicroseconds(50);
  }
  int leituraMedia = soma / 10;

  bool bleMudou = (statusBleAtual != ultimoStatusBle);

  if (abs(leituraMedia - ultimaLeituraPot) > 50 || forcarExibicao || bleMudou) {
    ultimaLeituraPot = leituraMedia;
    ultimoStatusBle = statusBleAtual;
    int passosCalculados = map(leituraMedia, 0, 4095, 8, 36);

    if (passosCalculados != totalPassos || forcarExibicao || bleMudou) {
      totalPassos = passosCalculados;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(statusBleAtual ? "BLE CONECTADO :)" : "PAREAR BLUETOOTH");
      lcd.setCursor(0, 1);
      lcd.print("FOTOS: ");
      lcd.print(totalPassos);
      lcd.print(" pts");
    }
  }
}

bool aguardarComChecagemBotao(int tempoMs) {
  int fatias = tempoMs / 50;
  for (int i = 0; i < fatias; i++) {
    if (detectarClique()) {
      tone(PIN_BUZZER, 800, 200);
      estadoAtual = PAUSADO;
      cliquesNaPausa = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(">> PAUSADO <<");
      lcd.setCursor(0, 1);
      lcd.print("1x Retoma|2x Zero");
      return false;
    }
    delay(50);
  }
  return true;
}

void gerenciarBotoesPausa() {
  if (detectarClique()) {
    cliquesNaPausa++;
    if (cliquesNaPausa == 1) {
      tempoPrimeiroCliquePausa = millis();
    } else if (cliquesNaPausa == 2) {
      tone(PIN_BUZZER, 500, 300);
      retornarAoPontoZero();
      
      fotoAtual = 1;
      cliquesNaPausa = 0;
      estadoAtual = PARADO;
      atualizarPassosPotenciometro(true);
      return;
    }
  }

  if (cliquesNaPausa == 1 && (millis() - tempoPrimeiroCliquePausa > JANELA_DUPLO_CLIQUE)) {
    tone(PIN_BUZZER, 1200, 150);
    cliquesNaPausa = 0;
    estadoAtual = EXECUTANDO;
  }
}

bool detectarClique() {
  static unsigned long tempoPressionado = 0;
  static bool botaoTravado = false;
  
  bool leitura = digitalRead(PIN_BOTAO);

  if (leitura == LOW) {
    if (tempoPressionado == 0) {
      tempoPressionado = millis();
    } else if (!botaoTravado && (millis() - tempoPressionado > 50)) {
      botaoTravado = true;
      return true;
    }
  } else {
    tempoPressionado = 0;
    botaoTravado = false;
  }

  return false;
}

void desenergizarMotor() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

void definirLeds(bool vermelho, bool amarelo, bool verde) {
  digitalWrite(PIN_LED_VERM,  vermelho);
  digitalWrite(PIN_LED_AMAR,  amarelo);
  digitalWrite(PIN_LED_VERDE, verde);
}

void bipFoto() {
  tone(PIN_BUZZER, 1800, 80);
}

void bipFimCiclo() {
  tone(PIN_BUZZER, 1000, 100);
  delay(120);
  tone(PIN_BUZZER, 1400, 100);
  delay(120);
  tone(PIN_BUZZER, 1800, 250);
}