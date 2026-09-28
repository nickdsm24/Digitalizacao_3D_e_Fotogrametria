#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BleKeyboard.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ================= PINAGEM ESP32 (30 PINOS) =================
// Joystick Analógico (Apenas os eixos X e Y)
const int PIN_JOY_X      = 21; // GPIO36 (VP) - Eixo Horizontal (Esq/Dir)
const int PIN_JOY_Y      = 19; // GPIO39 (VN) - Eixo Vertical (Cima/Baixo)

// Atuadores e Sinalizadores
const int PIN_LASER      = 33; // GPIO33 (Módulo Mini Laser)
const int PIN_BUZZER     = 25; // GPIO25 (Buzzer)
const int PIN_LED_VERDE  = 4; // GPIO26 (LEDs Verdes - Pronto / Standby)
const int PIN_LED_AMAR   = 2; // GPIO27 (LEDs Amarelos - Girando / Pausa)
const int PIN_LED_VERM   = 15; // GPIO14 (LEDs Vermelhos - Captura de Foto)

// Driver ULN2003 (Motor de Passo 28BYJ-48)
const int PIN_IN1        = 34;
const int PIN_IN2        = 35;
const int PIN_IN3        = 32;
const int PIN_IN4        = 33;

// Display LCD 16x2 I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============ CONTROLE DO MOTOR (HALF-STEP SUAVE) ============
const int PASSOS_VOLTA_COMPLETA = 4096; // 4096 meio-passos = 360° exatos
const int passosMatriz[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};
int indicePassoMotor = 0;

// ============ BLUETOOTH BLE SHUTTER ============
BleKeyboard bleKeyboard("Scanner 3D Shutter", "ESP32", 100);

// ============ MÁQUINA DE ESTADOS ============
enum EstadoSistema {
  PARADO,
  EXECUTANDO,
  PAUSADO
};
EstadoSistema estadoAtual = PARADO;

// Comandos do Joystick (Sem SW)
enum ComandoJoy {
  JOY_NONE,
  JOY_UP,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT
};

// Variáveis de controle de ciclo
int totalPassos = 24;            // Padrão em 24 fotos por volta
int fotoAtual = 1;
long passosAcumulados = 0;       // Odômetro absoluto
int anguloAtual = 0;

// Variáveis de controle de toques do Joystick
unsigned long tempoPrimeiroToque = 0;
int toquesCima = 0;
int toquesBaixo = 0;
const unsigned long JANELA_DUPLO_TOQUE = 500; // Janela em ms para detectar 2 toques

// Protótipos de funções
void definirLeds(bool vermelho, bool amarelo, bool verde);
void desenergizarMotor();
void moverMotor(long passos, int delayMicros = 1200);
void bipCurto();
void bipFoto();
void bipFimCiclo();
ComandoJoy lerJoystick();
void atualizarTelaMenu();
bool aguardarComChecagemJoystick(int tempoMs);
void gerenciarComandosPausa();
void retornarAoPontoZero();
void dispararCameraCelular();

// ================= SETUP =================
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Desativa o detector de brownout
  
  Serial.begin(115200);
  
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMAR, OUTPUT);
  pinMode(PIN_LED_VERM, OUTPUT);
  pinMode(PIN_LASER, OUTPUT);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);

  // Inicializa LCD I2C
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FOTOGRAMETRIA 3D");
  lcd.setCursor(0, 1);
  lcd.print("JOYSTICK + BLE  ");

  bleKeyboard.begin();
  delay(1200);

  definirLeds(LOW, LOW, HIGH);   // Verde (Pronto no Ponto 0)
  digitalWrite(PIN_LASER, HIGH); // Mira laser ligada para centralizar
  desenergizarMotor();

  atualizarTelaMenu();
}

// ================= LOOP PRINCIPAL =================
void loop() {
  ComandoJoy cmd = lerJoystick();

  switch (estadoAtual) {
    case PARADO:
      definirLeds(LOW, LOW, HIGH);
      digitalWrite(PIN_LASER, HIGH);

      // Ajuste de fotos: Direita (+4) / Esquerda (-4)
      if (cmd == JOY_RIGHT) {
        if (totalPassos < 36) {
          totalPassos += 4;
          bipCurto();
          atualizarTelaMenu();
        }
      } else if (cmd == JOY_LEFT) {
        if (totalPassos > 8) {
          totalPassos -= 4;
          bipCurto();
          atualizarTelaMenu();
        }
      }

      // Detecção de 2x CIMA para INICIAR
      if (cmd == JOY_UP) {
        toquesCima++;
        if (toquesCima == 1) {
          tempoPrimeiroToque = millis();
        } else if (toquesCima >= 2) {
          // 2x CIMA: INICIAR!
          toquesCima = 0;
          tone(PIN_BUZZER, 1400, 180);
          digitalWrite(PIN_LASER, LOW);
          passosAcumulados = 0;
          fotoAtual = 1;
          estadoAtual = EXECUTANDO;
        }
      }

      // Reseta contagem de toques se passou da janela
      if (toquesCima > 0 && (millis() - tempoPrimeiroToque > JANELA_DUPLO_TOQUE)) {
        toquesCima = 0;
      }
      break;

    case EXECUTANDO:
      digitalWrite(PIN_LASER, LOW);

      while (fotoAtual <= totalPassos) {
        // 1. Calcula passos angulares do motor
        long posicaoAlvoPassos = round((fotoAtual - 1) * ((float)PASSOS_VOLTA_COMPLETA / totalPassos));
        long passosParaGirar = posicaoAlvoPassos - passosAcumulados;

        if (passosParaGirar > 0) {
          definirLeds(LOW, HIGH, LOW); // Amarelos (Girando)
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("GIRANDO MESA...");
          lcd.setCursor(0, 1);
          lcd.print("Passo: +");
          lcd.print(passosParaGirar);
          lcd.print(" stp");

          moverMotor(passosParaGirar, 1200);
          passosAcumulados += passosParaGirar;
          desenergizarMotor();
        }

        // 2. Atualiza LCD com foto e status BLE
        anguloAtual = round((passosAcumulados / (float)PASSOS_VOLTA_COMPLETA) * 360.0);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("FOTO: ");
        lcd.print(fotoAtual);
        lcd.print("/");
        lcd.print(totalPassos);

        lcd.setCursor(0, 1);
        lcd.print(anguloAtual);
        lcd.print((char)223);
        lcd.print(bleKeyboard.isConnected() ? " [BLE:ON]" : " [BLE:--]");

        // 3. Estabilização mecânica (600ms)
        definirLeds(LOW, HIGH, LOW);
        if (!aguardarComChecagemJoystick(600)) return; // Se puxou para BAIXO, pausa

        // 4. DISPARO DA FOTO (Bluetooth BLE + LEDs Vermelhos + Bip)
        definirLeds(HIGH, LOW, LOW); // Vermelhos
        bipFoto();
        dispararCameraCelular();

        // 5. Intervalo pós-disparo (1.2s)
        if (!aguardarComChecagemJoystick(1200)) return;

        fotoAtual++;
      }

      // 6. Fechamento dos 360° exatos no Ponto 0
      if (passosAcumulados < PASSOS_VOLTA_COMPLETA) {
        long restoParaZero = PASSOS_VOLTA_COMPLETA - passosAcumulados;
        moverMotor(restoParaZero, 1200);
        passosAcumulados += restoParaZero;
      }
      desenergizarMotor();

      bipFimCiclo();
      definirLeds(LOW, LOW, HIGH); // Verdes
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SCAN CONCLUIDO!");
      lcd.setCursor(0, 1);
      lcd.print("100% NO PONTO 0");
      delay(2500);

      passosAcumulados = 0;
      fotoAtual = 1;
      estadoAtual = PARADO;
      atualizarTelaMenu();
      break;

    case PAUSADO:
      definirLeds(LOW, HIGH, LOW);
      desenergizarMotor();
      gerenciarComandosPausa();
      break;
  }
}

// ============= LEITOR DO JOYSTICK (SEM O BOTÃO SW) =============
ComandoJoy lerJoystick() {
  static bool stickNeutro = true;

  int x = analogRead(PIN_JOY_X);
  int y = analogRead(PIN_JOY_Y);

  // Zona morta central (repouso)
  if (x > 1400 && x < 2800 && y > 1400 && y < 2800) {
    stickNeutro = true;
    return JOY_NONE;
  }

  // Se o joystick estava no centro e foi empurrado para uma direção:
  if (stickNeutro) {
    if (y < 600) {
      stickNeutro = false;
      return JOY_UP;
    } else if (y > 3400) {
      stickNeutro = false;
      return JOY_DOWN;
    } else if (x < 600) {
      stickNeutro = false;
      return JOY_LEFT;
    } else if (x > 3400) {
      stickNeutro = false;
      return JOY_RIGHT;
    }
  }

  return JOY_NONE;
}

// ============= DISPARADOR BLUETOOTH BLE =============
void dispararCameraCelular() {
  if (bleKeyboard.isConnected()) {
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    Serial.println(">>> [BLE] Foto disparada via Bluetooth <<<");
  } else {
    Serial.println(">>> [Aviso] BLE desconectado (disparo ignorado) <<<");
  }
}

// ============= CONTROLE DO MOTOR DE PASSO =============
void moverMotor(long passos, int delayMicros) {
  int direcao = (passos > 0) ? 1 : -1;
  long total = abs(passos);

  for (long i = 0; i < total; i++) {
    indicePassoMotor += direcao;
    if (indicePassoMotor >= 8) indicePassoMotor = 0;
    if (indicePassoMotor < 0) indicePassoMotor = 7;

    digitalWrite(PIN_IN1, passosMatriz[indicePassoMotor][0]);
    digitalWrite(PIN_IN2, passosMatriz[indicePassoMotor][1]);
    digitalWrite(PIN_IN3, passosMatriz[indicePassoMotor][2]);
    digitalWrite(PIN_IN4, passosMatriz[indicePassoMotor][3]);

    delayMicroseconds(delayMicros);
  }
}

void desenergizarMotor() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

void retornarAoPontoZero() {
  if (passosAcumulados > 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RETORNANDO...");
    lcd.setCursor(0, 1);
    lcd.print("Voltando p/ 0");

    moverMotor(-passosAcumulados, 1200);
    passosAcumulados = 0;
    desenergizarMotor();
  }
}

// ============= INTERFACE E PAUSA =============
void atualizarTelaMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(bleKeyboard.isConnected() ? "BLE CONECTADO :)" : "PAREAR BLUETOOTH");
  lcd.setCursor(0, 1);
  lcd.print("FOTOS: ");
  lcd.print(totalPassos);
  lcd.print(" pts");
}

bool aguardarComChecagemJoystick(int tempoMs) {
  int fatias = tempoMs / 50;
  for (int i = 0; i < fatias; i++) {
    ComandoJoy cmd = lerJoystick();
    // 1 toque para BAIXO -> PAUSAR
    if (cmd == JOY_DOWN) {
      tone(PIN_BUZZER, 800, 200);
      estadoAtual = PAUSADO;
      toquesBaixo = 0;
      toquesCima = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(">> PAUSADO <<");
      lcd.setCursor(0, 1);
      lcd.print("^ Volta | v Canc");
      return false;
    }
    delay(50);
  }
  return true;
}

void gerenciarComandosPausa() {
  ComandoJoy cmd = lerJoystick();

  // 1x CIMA: RETOMAR
  if (cmd == JOY_UP) {
    tone(PIN_BUZZER, 1200, 150);
    toquesBaixo = 0;
    estadoAtual = EXECUTANDO;
    return;
  }

  // 2x BAIXO: CANCELAR E RETORNAR AO PONTO ZERO
  if (cmd == JOY_DOWN) {
    toquesBaixo++;
    if (toquesBaixo == 1) {
      tempoPrimeiroToque = millis();
    } else if (toquesBaixo >= 2) {
      toquesBaixo = 0;
      tone(PIN_BUZZER, 500, 300);
      retornarAoPontoZero();

      fotoAtual = 1;
      estadoAtual = PARADO;
      atualizarTelaMenu();
      return;
    }
  }

  if (toquesBaixo > 0 && (millis() - tempoPrimeiroToque > JANELA_DUPLO_TOQUE)) {
    toquesBaixo = 0;
  }
}

void definirLeds(bool vermelho, bool amarelo, bool verde) {
  digitalWrite(PIN_LED_VERM,  vermelho);
  digitalWrite(PIN_LED_AMAR,  amarelo);
  digitalWrite(PIN_LED_VERDE, verde);
}

void bipCurto() {
  tone(PIN_BUZZER, 1600, 40);
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