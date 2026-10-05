#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/*
 * =========================================================================
 *  SISTEMA AUTOMATIZADO DE DIGITALIZAÇÃO 3D E FOTOGRAMETRIA
 *  Microcontrolador: ESP32 DevKit V1 (30 Pinos)
 * 
 *  - Auto-Teste Inicial (Sequência):
 *      1. LEDs (Verde, Amarelo, Vermelho)
 *      2. Display LCD 16x2
 *      3. Laser Nerf de Centralização
 *      4. Motor de Passo 28BYJ-48 (Giro Teste Horário / Anti-Horário)
 *      5. Leitura da Posição do Joystick no Display
 *      * Saída do Teste: 4 cliques para BAIXO (▼▼▼▼)
 * 
 *  - Menu Principal:
 *      * Ajuste de Fotos: Esquerda (-4) / Direita (+4)
 *      * 2x CIMA (▲▲): Iniciar ciclo de 360°
 * 
 *  - Controle em Execução e Pausa:
 *      * 2x BAIXO (▼▼): Pausar o giro
 *      * Na Pausa: 2x CIMA (▲▲) retoma | 2x BAIXO (▼▼) cancela pelo caminho mais curto
 * =========================================================================
 */

// ================= MAPEAMENTO DE PINOS =================
// Joystick Analógico (Canais ADC1)
const int PIN_JOY_X      = 36; // GPIO36 (VP) - Eixo Vertical na montagem física
const int PIN_JOY_Y      = 39; // GPIO39 (VN) - Eixo Horizontal na montagem física

// Atuadores e Sinalizadores
const int PIN_BUZZER     = 25; // GPIO25 - Buzzer Piezoelétrico (Feedback Sonoro)
const int PIN_LASER      = 33; // GPIO33 - Mira Laser Nerf (com Resistor 300R)

// LEDs de Status (Topologia em Funil com Resistores de 300R)
const int PIN_LED_VERDE  = 26; // GPIO26 - 2x LEDs Verdes (Standby / Ponto 0)
const int PIN_LED_AMAR   = 27; // GPIO27 - 2x LEDs Amarelos (Giro / Pausa)
const int PIN_LED_VERM   = 14; // GPIO14 - 2x LEDs Vermelhos (Momento da Foto)

// Driver ULN2003 (Motor de Passo 28BYJ-48)
const int PIN_IN1        = 19; // GPIO19 - Fase A
const int PIN_IN2        = 18; // GPIO18 - Fase B
const int PIN_IN3        = 5;  // GPIO05 - Fase C
const int PIN_IN4        = 17; // GPIO17 - Fase D (TX2)

// Display LCD 16x2 I2C (SDA = GPIO 21, SCL = GPIO 22)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============ CINEMÁTICA E CALIBRAÇÃO (5427 PASSOS = 360°) ============
const int PASSOS_VOLTA_COMPLETA       = 5427; // Calibrado para volta de 360,00° exatos
const int DELAY_PASSO_PADRAO_MICROS   = 1300; // Intervalo de passo nominal
const int DELAY_RETORNO_RAPIDO_MICROS = 1100; // Intervalo de passo para retorno rápido

// Matriz de acionamento em Meio-Passo (Half-Step)
const int passosMatriz[8][4] = {
  {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0},
  {0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 1}, {1, 0, 0, 1}
};
int indicePasso = 0;

// ============ MÁQUINA DE ESTADOS FINITOS ============
enum EstadoGlobal {
  MODO_TESTE_INICIAL,
  MENU_PRONTO_EXECUCAO,
  EXECUTANDO_SCAN,
  PAUSADO_SCAN
};
EstadoGlobal estadoAtual = MODO_TESTE_INICIAL;
EstadoGlobal estadoAnterior = MODO_TESTE_INICIAL;

enum ComandoJoy {
  JOY_NONE,
  JOY_UP,
  JOY_DOWN,
  JOY_LEFT,
  JOY_RIGHT
};

// Variáveis de Controle de Ciclo
int totalPassos = 24;      // Resolução padrão (24 fotos / 15° por foto)
int fotoAtual = 1;
long passosAcumulados = 0; // Odômetro angular absoluto
int anguloAtual = 0;

// Controle do Modo de Teste
int etapaTeste = 0; // 0=LEDs, 1=LCD, 2=Laser, 3=Motor, 4=Joystick
unsigned long tempoInicioEtapaTeste = 0;
int toquesBaixoTeste = 0;
unsigned long tempoPrimeiroToqueBaixoTeste = 0;
const unsigned long JANELA_4_TOQUES_TESTE = 2000; // Janela de 2s para os 4 cliques

// Temporizadores Independentes de Duplo Clique
unsigned long tempoPrimeiroToqueCima = 0;
unsigned long tempoPrimeiroToqueBaixo = 0;
int toquesCima = 0;
int toquesBaixo = 0;
const unsigned long JANELA_DUPLO_TOQUE = 700; // ms

// Calibração do Ponto Central do Joystick
int centroJoyX = 2048;
int centroJoyY = 2048;

// Protótipos de Funções
void definirLeds(bool vermelho, bool amarelo, bool verde);
void desenergizarMotor();
void moverMotor(long passos, int delayMicros = DELAY_PASSO_PADRAO_MICROS);
void bipCurto();
void bipFoto();
void bipFimCiclo();
void bipInicioEtapaTeste();
int lerAnalogicoFiltrado(int pino);
void calibrarCentroJoystick();
ComandoJoy lerJoystick();
void transitarParaEstado(EstadoGlobal novoEstado);
void atualizarTelaMenu();
bool checarDuploBaixoParaPausa(int tempoEsperaMs);
void gerenciarComandosPausa();
void retornarAoPontoZeroMenorDistancia();
void executarModoTesteSequencial();

// ================= INICIALIZAÇÃO (SETUP) =================
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=======================================================");
  Serial.println("   SISTEMA DE DIGITALIZACAO 3D - MODO AUTONOMO IoT     ");
  Serial.println("=======================================================");

  // Configuração dos pinos de saída
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LASER, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMAR, OUTPUT);
  pinMode(PIN_LED_VERM, OUTPUT);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);

  desenergizarMotor();
  definirLeds(LOW, LOW, LOW);
  digitalWrite(PIN_LASER, LOW);

  // Inicialização do barramento I2C e Display LCD 16x2
  Wire.begin(21, 22);
  delay(50);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SCANNER 3D IOT  ");
  lcd.setCursor(0, 1);
  lcd.print("Calibrando Joy..");

  calibrarCentroJoystick();

  // Início do auto-teste de componentes
  etapaTeste = 0;
  tempoInicioEtapaTeste = millis();
  bipInicioEtapaTeste();
}

// ================= LAÇO PRINCIPAL (LOOP) =================
void loop() {
  switch (estadoAtual) {
    // -------------------------------------------------------------------------
    // ESTADO 1: AUTO-TESTE SEQUENCIAL DE COMPONENTES
    // -------------------------------------------------------------------------
    case MODO_TESTE_INICIAL: {
      executarModoTesteSequencial();
      break;
    }

    // -------------------------------------------------------------------------
    // ESTADO 2: MENU DE CONFIGURAÇÃO E PRONTO PARA EXECUÇÃO
    // -------------------------------------------------------------------------
    case MENU_PRONTO_EXECUCAO: {
      ComandoJoy cmd = lerJoystick();

      // Ajuste de densidade fotográfica: Direita (+4) / Esquerda (-4)
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

      // Duplo clique para CIMA (▲▲) inicia a digitalização 360°
      if (cmd == JOY_UP) {
        toquesCima++;
        if (toquesCima == 1) {
          tempoPrimeiroToqueCima = millis();
          Serial.println(">> [MENU] 1x CIMA recebido. Pressione novamente para Iniciar!");
        } else if (toquesCima >= 2) {
          toquesCima = 0;
          Serial.println(">> [MENU] 2x CIMA Confirmado! Iniciando escaneamento...");
          tone(PIN_BUZZER, 1800, 200);
          passosAcumulados = 0;
          fotoAtual = 1;
          transitarParaEstado(EXECUTANDO_SCAN);
          return;
        }
      }

      if (toquesCima > 0 && (millis() - tempoPrimeiroToqueCima > JANELA_DUPLO_TOQUE)) {
        toquesCima = 0;
      }
      break;
    }

    // -------------------------------------------------------------------------
    // ESTADO 3: EXECUÇÃO DO SCANNER (GIRO + ESTABILIZAÇÃO + CAPTURA)
    // -------------------------------------------------------------------------
    case EXECUTANDO_SCAN: {
      while (fotoAtual <= totalPassos) {
        // 1. Cálculo cinemático da posição alvo
        long posicaoAlvoPassos = round((fotoAtual - 1) * ((float)PASSOS_VOLTA_COMPLETA / totalPassos));
        long passosParaGirar = posicaoAlvoPassos - passosAcumulados;

        // 2. Giro mecânico da mesa rotativa
        if (passosParaGirar > 0) {
          definirLeds(LOW, HIGH, LOW); // LED Amarelo aceso (Mesa girando)
          lcd.setCursor(0, 0);
          lcd.print("GIRANDO MESA... ");
          lcd.setCursor(0, 1);
          char bufPasso[17];
          snprintf(bufPasso, sizeof(bufPasso), "Passo: +%-4ld stp", passosParaGirar);
          lcd.print(bufPasso);

          moverMotor(passosParaGirar, DELAY_PASSO_PADRAO_MICROS);
          passosAcumulados += passosParaGirar;
          desenergizarMotor(); // Proteção térmica e economia de energia
        }

        // 3. Atualização de status no visor LCD (preenchimento exato de 16 caracteres)
        anguloAtual = round((passosAcumulados / (float)PASSOS_VOLTA_COMPLETA) * 360.0);
        char bufStatus1[17], bufStatus2[17];
        snprintf(bufStatus1, sizeof(bufStatus1), "FOTO: %2d/%-2d     ", fotoAtual, totalPassos);
        snprintf(bufStatus2, sizeof(bufStatus2), "ANG:%3d%c [STAB] ", anguloAtual, (char)223);
        
        lcd.setCursor(0, 0);
        lcd.print(bufStatus1);
        lcd.setCursor(0, 1);
        lcd.print(bufStatus2);

        // 4. Pausa de estabilização mecânica (600 ms para anular inércia)
        definirLeds(LOW, HIGH, LOW);
        if (!checarDuploBaixoParaPausa(600)) return;

        // 5. Sinal de captura estável (LED Vermelho + Bip sonoro)
        definirLeds(HIGH, LOW, LOW); // LED Vermelho aceso (Momento da foto)
        lcd.setCursor(9, 1);
        lcd.print("[FOTO] "); // 7 caracteres limpam perfeitamente [STAB] 
        bipFoto();
        Serial.printf(">> [CAPTURA FOTO %d/%d] Angulo: %d graus\n", fotoAtual, totalPassos, anguloAtual);

        // 6. Intervalo pós-disparo (1.2 s para gravação da foto no celular)
        if (!checarDuploBaixoParaPausa(1200)) return;

        fotoAtual++;
      }

      // 7. Fechamento dos 360° exatos no Ponto 0
      if (passosAcumulados < PASSOS_VOLTA_COMPLETA) {
        long resto = PASSOS_VOLTA_COMPLETA - passosAcumulados;
        moverMotor(resto, DELAY_PASSO_PADRAO_MICROS);
        passosAcumulados += resto;
      }
      desenergizarMotor();

      bipFimCiclo();
      lcd.setCursor(0, 0);
      lcd.print("SCAN CONCLUIDO! ");
      lcd.setCursor(0, 1);
      lcd.print("100% NO PONTO 0 ");
      delay(2500);

      passosAcumulados = 0;
      fotoAtual = 1;
      transitarParaEstado(MENU_PRONTO_EXECUCAO);
      break;
    }

    // -------------------------------------------------------------------------
    // ESTADO 4: PAUSADO
    // -------------------------------------------------------------------------
    case PAUSADO_SCAN: {
      gerenciarComandosPausa();
      break;
    }
  }
}

// ============= ROTINA SEQUENCIAL DE AUTO-TESTE DE COMPONENTES =============
void executarModoTesteSequencial() {
  ComandoJoy cmd = lerJoystick();

  // Monitora saída: 4 cliques para BAIXO (▼▼▼▼) encerram o teste
  if (cmd == JOY_DOWN) {
    toquesBaixoTeste++;
    Serial.printf(">> [MODO TESTE] Toque BAIXO (%d/4)\n", toquesBaixoTeste);
    bipCurto();

    if (toquesBaixoTeste == 1) {
      tempoPrimeiroToqueBaixoTeste = millis();
    } else if (toquesBaixoTeste >= 4) {
      Serial.println(">> [MODO TESTE] 4x BAIXO -> Encerrando auto-teste!");
      tone(PIN_BUZZER, 1800, 300);
      transitarParaEstado(MENU_PRONTO_EXECUCAO);
      return;
    }
  }

  // Reseta contador de toques se ultrapassar a janela de tempo
  if (toquesBaixoTeste > 0 && (millis() - tempoPrimeiroToqueBaixoTeste > JANELA_4_TOQUES_TESTE)) {
    toquesBaixoTeste = 0;
  }

  unsigned long tempoDecorridoEtapa = millis() - tempoInicioEtapaTeste;

  switch (etapaTeste) {
    // Etapa 1: Teste dos LEDs (Verde -> Amarelo -> Vermelho -> Todos)
    case 0: {
      lcd.setCursor(0, 0);
      lcd.print("1. TESTE LEDS   ");
      lcd.setCursor(0, 1);
      char buf[17];
      snprintf(buf, sizeof(buf), "Sair: 4x v (%d/4)", toquesBaixoTeste);
      lcd.print(buf);

      int subFase = (tempoDecorridoEtapa / 500) % 4;
      switch (subFase) {
        case 0: definirLeds(LOW, LOW, HIGH); break; // Verde
        case 1: definirLeds(LOW, HIGH, LOW); break; // Amarelo
        case 2: definirLeds(HIGH, LOW, LOW); break; // Vermelho
        case 3: definirLeds(HIGH, HIGH, HIGH); break; // Todos
      }

      if (tempoDecorridoEtapa > 3000) {
        definirLeds(LOW, LOW, LOW);
        etapaTeste = 1;
        tempoInicioEtapaTeste = millis();
        bipInicioEtapaTeste();
      }
      break;
    }

    // Etapa 2: Teste do Display LCD 16x2
    case 1: {
      definirLeds(LOW, LOW, LOW);
      lcd.setCursor(0, 0);
      lcd.print("2. TESTE DISPLAY");
      lcd.setCursor(0, 1);
      lcd.print("LCD 16x2 I2C: OK");

      if (tempoDecorridoEtapa > 2500) {
        etapaTeste = 2;
        tempoInicioEtapaTeste = millis();
        bipInicioEtapaTeste();
      }
      break;
    }

    // Etapa 3: Teste da Mira Laser Nerf (GPIO 33)
    case 2: {
      lcd.setCursor(0, 0);
      lcd.print("3. TESTE LASER  ");
      lcd.setCursor(0, 1);
      
      bool estadoLaser = ((tempoDecorridoEtapa / 400) % 2) == 0;
      digitalWrite(PIN_LASER, estadoLaser ? HIGH : LOW);
      lcd.print(estadoLaser ? "MIRA LASER: [ON]" : "MIRA LASER:[OFF]");

      if (tempoDecorridoEtapa > 3000) {
        digitalWrite(PIN_LASER, LOW);
        etapaTeste = 3;
        tempoInicioEtapaTeste = millis();
        bipInicioEtapaTeste();
      }
      break;
    }

    // Etapa 4: Teste do Motor de Passo (Giro Bidirecional 250 Passos)
    case 3: {
      lcd.setCursor(0, 0);
      lcd.print("4. TESTE MOTOR  ");
      lcd.setCursor(0, 1);
      lcd.print("Giro Horario -> ");
      definirLeds(LOW, HIGH, LOW);

      moverMotor(250, DELAY_PASSO_PADRAO_MICROS);
      delay(200);

      lcd.setCursor(0, 1);
      lcd.print("<- Anti-Horario ");
      moverMotor(-250, DELAY_PASSO_PADRAO_MICROS);
      desenergizarMotor();
      definirLeds(LOW, LOW, LOW);

      etapaTeste = 4;
      tempoInicioEtapaTeste = millis();
      bipInicioEtapaTeste();
      break;
    }

    // Etapa 5: Reconhecimento da Posição do Joystick no Display (Comprimento exato de 16 caracteres)
    case 4: {
      digitalWrite(PIN_LASER, HIGH);
      definirLeds(LOW, LOW, HIGH);

      int x = lerAnalogicoFiltrado(PIN_JOY_X);
      int y = lerAnalogicoFiltrado(PIN_JOY_Y);
      int deltaX = x - centroJoyX;
      int deltaY = y - centroJoyY;

      lcd.setCursor(0, 0);
      lcd.print("5. JOY: "); // 8 caracteres (colunas 0 a 7)
      
      const int ZONA_MORTA = 350;
      // Cada string abaixo tem exatamente 8 caracteres para completar 16 sem estourar a linha
      if (abs(deltaX) < ZONA_MORTA && abs(deltaY) < ZONA_MORTA) {
        lcd.print("[CENTRO]");
      } else if (abs(deltaX) >= abs(deltaY)) {
        if (deltaX > ZONA_MORTA) lcd.print("BAIXO   ");
        else if (deltaX < -ZONA_MORTA) lcd.print("CIMA    ");
      } else {
        if (deltaY > ZONA_MORTA) lcd.print("ESQUERDA");
        else if (deltaY < -ZONA_MORTA) lcd.print("DIREITA ");
      }

      lcd.setCursor(0, 1);
      char bufJoy[17];
      snprintf(bufJoy, sizeof(bufJoy), "4x baixo (%d/4) ", toquesBaixoTeste);
      lcd.print(bufJoy);

      delay(60);
      break;
    }
  }
}

// ============= GERENCIADOR CENTRALIZADO DE TRANSIÇÃO DE ESTADOS =============
void transitarParaEstado(EstadoGlobal novoEstado) {
  estadoAnterior = estadoAtual;
  estadoAtual = novoEstado;

  // Reseta contadores de comandos
  toquesCima = 0;
  toquesBaixo = 0;
  toquesBaixoTeste = 0;

  switch (novoEstado) {
    case MENU_PRONTO_EXECUCAO:
      desenergizarMotor();
      definirLeds(LOW, LOW, HIGH);   // LED Verde aceso (Pronto para iniciar)
      digitalWrite(PIN_LASER, HIGH); // Mira laser ligada para centralizar a peça
      tone(PIN_BUZZER, 1600, 150);
      atualizarTelaMenu();
      break;

    case EXECUTANDO_SCAN:
      digitalWrite(PIN_LASER, LOW);  // Mira laser desligada durante as capturas
      break;

    case PAUSADO_SCAN:
      desenergizarMotor();
      definirLeds(LOW, HIGH, LOW);   // LED Amarelo aceso (Pausa)
      digitalWrite(PIN_LASER, HIGH); // Mira laser ligada para checagem visual da peça
      tone(PIN_BUZZER, 800, 200);

      lcd.setCursor(0, 0);
      lcd.print(">> PAUSADO <<   ");
      lcd.setCursor(0, 1);
      lcd.print("Seguir | Sair   "); // 16 caracteres exatos para limpar a linha
      break;

    default:
      break;
  }
}

// ============= CHECAGEM DE 2x BAIXO PARA PAUSA DURANTE O GIRO =============
bool checarDuploBaixoParaPausa(int tempoEsperaMs) {
  int fatias = tempoEsperaMs / 40;
  for (int i = 0; i < fatias; i++) {
    ComandoJoy cmd = lerJoystick();

    if (cmd == JOY_DOWN) {
      toquesBaixo++;
      Serial.printf(">> [EXECUCAO] Toque BAIXO para Pausa (%d/2)\n", toquesBaixo);
      if (toquesBaixo == 1) {
        tempoPrimeiroToqueBaixo = millis();
      } else if (toquesBaixo >= 2) {
        toquesBaixo = 0;
        Serial.println(">> [EXECUCAO] 2x BAIXO detectado -> Pausando sistema!");
        transitarParaEstado(PAUSADO_SCAN);
        return false;
      }
    }

    if (toquesBaixo > 0 && (millis() - tempoPrimeiroToqueBaixo > JANELA_DUPLO_TOQUE)) {
      toquesBaixo = 0;
    }

    delay(40);
  }
  return true;
}

// ============= GERENCIAMENTO DE COMANDOS NA PAUSA =============
void gerenciarComandosPausa() {
  ComandoJoy cmd = lerJoystick();

  // 2x CIMA (▲▲): Retomar digitalização
  if (cmd == JOY_UP) {
    toquesCima++;
    Serial.printf(">> [PAUSA] Toque CIMA recebido (%d/2)\n", toquesCima);
    if (toquesCima == 1) {
      tempoPrimeiroToqueCima = millis();
    } else if (toquesCima >= 2) {
      toquesCima = 0;
      Serial.println(">> [PAUSA] 2x CIMA -> Retomando digitalização!");
      tone(PIN_BUZZER, 1400, 150);
      transitarParaEstado(EXECUTANDO_SCAN);
      return;
    }
  }

  // 2x BAIXO (▼▼): Cancelar e retornar ao Ponto 0 pelo caminho mais curto
  if (cmd == JOY_DOWN) {
    toquesBaixo++;
    Serial.printf(">> [PAUSA] Toque BAIXO recebido (%d/2)\n", toquesBaixo);
    if (toquesBaixo == 1) {
      tempoPrimeiroToqueBaixo = millis();
    } else if (toquesBaixo >= 2) {
      toquesBaixo = 0;
      Serial.println(">> [PAUSA] 2x BAIXO -> Cancelando via menor caminho!");
      tone(PIN_BUZZER, 500, 300);
      retornarAoPontoZeroMenorDistancia();

      fotoAtual = 1;
      transitarParaEstado(MENU_PRONTO_EXECUCAO);
      return;
    }
  }

  if (toquesCima > 0 && (millis() - tempoPrimeiroToqueCima > JANELA_DUPLO_TOQUE)) {
    toquesCima = 0;
  }
  if (toquesBaixo > 0 && (millis() - tempoPrimeiroToqueBaixo > JANELA_DUPLO_TOQUE)) {
    toquesBaixo = 0;
  }
}

// ============= RETORNO AO PONTO ZERO PELO CAMINHO MAIS CURTO =============
void retornarAoPontoZeroMenorDistancia() {
  if (passosAcumulados > 0) {
    lcd.setCursor(0, 0);
    lcd.print("RESET PONTO 0...");

    long passosRestantes = PASSOS_VOLTA_COMPLETA - passosAcumulados;

    // Se passou da metade (> 180°), o trajeto mais rápido é avançar no sentido horário
    if (passosAcumulados > (PASSOS_VOLTA_COMPLETA / 2)) {
      lcd.setCursor(0, 1);
      lcd.print("Avanco Rapido ->");
      Serial.printf(">> [RESET ZERO] Avanco rapido: +%ld passos\n", passosRestantes);
      moverMotor(passosRestantes, DELAY_RETORNO_RAPIDO_MICROS);
    } 
    // Se está na primeira metade (<= 180°), o trajeto mais rápido é rebobinar
    else {
      lcd.setCursor(0, 1);
      lcd.print("<- Rebobinando  ");
      Serial.printf(">> [RESET ZERO] Rebobinando: -%ld passos\n", passosAcumulados);
      moverMotor(-passosAcumulados, DELAY_RETORNO_RAPIDO_MICROS);
    }

    passosAcumulados = 0;
    desenergizarMotor();
  }
}

// ============= INTERFACE DO MENU PRINCIPAL =============
void atualizarTelaMenu() {
  lcd.setCursor(0, 0);
  lcd.print("DIGITALIZACAO 3D");
  lcd.setCursor(0, 1);
  char buf[17];
  snprintf(buf, sizeof(buf), "FOTOS: %-2d       ", totalPassos); // 16 caracteres exatos para limpar resquícios
  lcd.print(buf);
}

// ============= FILTRAGEM RÁPIDA DE LEITURA ANALÓGICA =============
int lerAnalogicoFiltrado(int pino) {
  long soma = 0;
  for (int i = 0; i < 4; i++) {
    soma += analogRead(pino);
    delayMicroseconds(20);
  }
  return soma / 4;
}

// ============= AUTO-CALIBRAÇÃO DO CENTRO DO JOYSTICK =============
void calibrarCentroJoystick() {
  long sx = 0, sy = 0;
  for (int i = 0; i < 20; i++) {
    sx += analogRead(PIN_JOY_X);
    sy += analogRead(PIN_JOY_Y);
    delay(5);
  }
  centroJoyX = sx / 20;
  centroJoyY = sy / 20;
  Serial.printf(">>> [CALIBRACAO] Centro: X=%d | Y=%d <<<\n", centroJoyX, centroJoyY);
}

// ============= DECODIFICADOR DO JOYSTICK (COMPENSAÇÃO DE 90°) =============
ComandoJoy lerJoystick() {
  static bool stickNeutro = true;

  int x = lerAnalogicoFiltrado(PIN_JOY_X);
  int y = lerAnalogicoFiltrado(PIN_JOY_Y);

  int deltaX = x - centroJoyX;
  int deltaY = y - centroJoyY;

  const int ZONA_MORTA = 350;
  bool noCentro = (abs(deltaX) < ZONA_MORTA) && (abs(deltaY) < ZONA_MORTA);

  if (noCentro) {
    stickNeutro = true;
    return JOY_NONE;
  }

  if (stickNeutro) {
    stickNeutro = false; // Trava o comando até retornar ao centro

    // Compensação da montagem física: Eixo X atua como Cima/Baixo
    if (abs(deltaX) >= abs(deltaY)) {
      if (deltaX > ZONA_MORTA) {
        Serial.printf(">> [JOYSTICK] BAIXO (dX=%+d, dY=%+d)\n", deltaX, deltaY);
        return JOY_DOWN;
      }
      if (deltaX < -ZONA_MORTA) {
        Serial.printf(">> [JOYSTICK] CIMA (dX=%+d, dY=%+d)\n", deltaX, deltaY);
        return JOY_UP;
      }
    } else {
      if (deltaY > ZONA_MORTA) {
        Serial.printf(">> [JOYSTICK] ESQUERDA (dX=%+d, dY=%+d)\n", deltaX, deltaY);
        return JOY_LEFT;
      }
      if (deltaY < -ZONA_MORTA) {
        Serial.printf(">> [JOYSTICK] DIREITA (dX=%+d, dY=%+d)\n", deltaX, deltaY);
        return JOY_RIGHT;
      }
    }
  }

  return JOY_NONE;
}

// ============= ACIONAMENTO DO MOTOR DE PASSO ULN2003 =============
void moverMotor(long passos, int delayMicros) {
  int direcao = (passos > 0) ? 1 : -1;
  long total = abs(passos);

  for (long i = 0; i < total; i++) {
    indicePasso += direcao;
    if (indicePasso >= 8) indicePasso = 0;
    if (indicePasso < 0) indicePasso = 7;

    digitalWrite(PIN_IN1, passosMatriz[indicePasso][0]);
    digitalWrite(PIN_IN2, passosMatriz[indicePasso][1]);
    digitalWrite(PIN_IN3, passosMatriz[indicePasso][2]);
    digitalWrite(PIN_IN4, passosMatriz[indicePasso][3]);

    delayMicroseconds(delayMicros);
  }
}

// Desativa todas as bobinas para proteção térmica
void desenergizarMotor() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

// ============= CONTROLE DOS LEDS DE STATUS =============
void definirLeds(bool vermelho, bool amarelo, bool verde) {
  digitalWrite(PIN_LED_VERM,  vermelho);
  digitalWrite(PIN_LED_AMAR,  amarelo);
  digitalWrite(PIN_LED_VERDE, verde);
}

// ============= EMISSÃO DE SINAIS SONOROS (BUZZER) =============
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

void bipInicioEtapaTeste() {
  tone(PIN_BUZZER, 1500, 120);
}