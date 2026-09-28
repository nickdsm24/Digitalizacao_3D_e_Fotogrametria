# RELATÓRIO TÉCNICO DE PROJETO: SISTEMA AUTOMATIZADO DE DIGITALIZAÇÃO 3D E FOTOGRAMETRIA IoT

**Instituição de Ensino:** Faculdade de Engenharia / Tecnologia  
**Disciplina:** Internet das Coisas (IoT) / Sistemas Embarcados  
**Plataforma de Desenvolvimento:** ESP32 DevKit (30 Pinos) + Protocolo Bluetooth Low Energy (BLE)  
**Conceito Central:** Mecatrônica Aplicada, Automação Cinemática e *Upcycling* de E-Lixo (Custo R$ 0,00)  

---

## 1. Descrição Geral da Aplicação

### 1.1. Contexto do Projeto
A fotogrametria é uma técnica de computação gráfica e visão computacional que reconstrói modelos tridimensionais densos a partir da sobreposição de fotografias bidimensionais capturadas ao redor de um objeto físico. O gargalo clássico na fotogrametria amadora reside na inconsistência humana: espaçamentos angulares desiguais, trepidações ao disparar a câmera e falta de estabilização mecânica das peças.

O presente projeto desenvolveu uma **Plataforma Mecatrônica e IoT Automatizada** controlada pelo microcontrolador **ESP32**, que coordena de forma sincronizada giros angulares de $360^\circ$ em uma mesa rotativa de alta precisão com o acionamento remoto do obturador da câmera de um smartphone via **Bluetooth Low Energy (BLE)**.

### 1.2. Objetivos
* **Garantia de Regularidade Cinemática:** Executar fatiamento angular homogêneo com resolução precisa de meio-passo ($4096$ passos/volta);
* **Extinção de Vibrações Estruturais:** Implementar pausas temporizadas de estabilização mecânica ($600\text{ ms}$) antes de cada captura fotográfica;
* **Disparo Sem Fio Não Invasivo (BLE HID):** Acionar o obturador da câmera nativa do smartphone sem toque físico, eliminando qualquer trepidação no enquadramento;
* **Interface Homem-Máquina (IHM) Intuitiva:** Permitir a configuração dinâmica do número de fotos ($8$ a $36$ capturas), pausa, retomada e cancelamento por meio de um **Joystick Analógico** e display **LCD 16x2 I2C**;
* **Sustentabilidade e Economia Circular (*Upcycling*):** Reutilizar sucata eletrônica descartada (leitores ópticos de CD/DVD e papelão estrutural) para construir o equipamento com custo adicional de R$ 0,00.

### 1.3. Motivação e Sustentabilidade
A transição global para a economia circular demanda soluções de engenharia que reaproveitem o lixo eletrônico (*e-waste*). Este projeto demonstra como conjuntos mecânicos de alta precisão descartados de notebooks e desktops podem ser integrados a sistemas microcontrolados modernos para criar ferramentas científicas acessíveis de preservação digital e prototipagem tridimensional.

---

## 2. Desenho da Arquitetura do Sistema

A arquitetura do sistema é dividida em **três camadas interconectadas**:

```
+-----------------------------------------------------------------------------------+
|                        1. CAMADA DE CONTROLE (ESP32 DevKit)                       |
|                                                                                   |
|  [ Máquina de Estados ] <---> [ Calculadora de Passos / Ponto 0 ] <---> [ BLE HID]|
+-------+--------------------+-------------------+---------------------+------------+
        |                    |                   |                     |
        | (I2C: D21/D22)     | (GPIOs 19,18,5,17)| (ADC1: VP/VN)       | (BLE 2.4GHz)
        v                    v                   v                     v
+---------------+    +---------------+   +-------------------+   +------------------+
| Display 16x2  |    | Driver ULN2003|   | - Joystick (VRX/Y)|   | Smartphone       |
| I2C (PCF8574) |    | + 28BYJ-48    |   | - 6 LEDs (Funil)  |   | (Câmera Nativa)  |
|               |    | (Half-Step)   |   | - Buzzer + Laser  |   | (Shutter Vol+)   |
+---------------+    +-------+-------+   +-------------------+   +--------+---------+
                             | (Correia elástica)                         | (Fotos 360°)
                             v                                            v
                     +---------------+                           +------------------+
                     | Spindle DVD   |                           | Software 3D      |
                     | (Mesa 360°)   | ========================> | (3DF Zephyr /    |
                     | Objeto 3D     |                           |  Luma AI)        |
                     +---------------+                           +------------------+
```

---

## 3. Relação de Partes e Materiais (BOM)

| Categoria | Componente / Modelo | Quantidade | Função no Projeto | Origem |
| :--- | :--- | :---: | :--- | :---: |
| **Microcontrolador** | ESP32 DevKit V1 (30 pinos, Wi-Fi/BLE) | 1 un | Núcleo de processamento e rádio BLE | Kit da Disciplina |
| **Atuador Cinemático** | Motor de Passo 28BYJ-48 (5V) + Driver ULN2003 | 1 un | Tração angular precisa de 4096 passos/volta | Kit da Disciplina |
| **Mancal e Base Giratória** | Mecanismo de *Spindle* de CD/DVD descartado + DVD | 1 un | Mancal rotativo de precisão com travas esféricas | **Upcycling** |
| **Transmissão** | Polia customizada + Correia elastomérica | 1 un | Transmissão com amortecimento de vibrações | **Upcycling** |
| **Interface Visual** | Display LCD 16x2 com Adaptador I2C (PCF8574) | 1 un | Exibição de menus, fotos, ângulos e status BLE | Kit da Disciplina |
| **Interface de Comando** | Módulo Joystick Analógico de 2 Eixos | 1 un | Navegação de menus, início, pausa e cancelamento | Kit da Disciplina |
| **Sinalização Óptica** | 6x LEDs 5mm (2 Verdes, 2 Amarelos, 2 Vermelhos) | 6 un | Sinalização de Standby, Movimento e Disparo | Kit da Disciplina |
| **Passivos** | Resistores de filme de carbono $300\,\Omega$ | 6 un | Limitadores de corrente individuais dos LEDs | Kit da Disciplina |
| **Feedback Auditivo** | Buzzer Piezoelétrico | 1 un | Alertas sonoros de início, captura e conclusão | Kit da Disciplina |
| **Gabinete e Estúdio** | Caixas de papelão reaproveitadas + EVA preto | 2 un | Estrutura modular e câmara escura de luz difusa | **Upcycling** |
| **Alimentação / Protótipo**| Protoboard 800 pts + Jumpers MF/MM + Suporte 45° | 1 kit | Distribuição elétrica e berço inclinado | Kit da Disciplina |

---

## 4. Diagrama de Conexão dos Componentes Eletrônicos

### 4.1. Mapeamento de Pinos e Justificativas de Engenharia

```text
                               ESP32 DevKit V1 (30 Pinos)
                                     +-------------+
                 EN (Reset Externo) -| 01       30 |- GPIO 23 [Livre]
        [Joystick VRX]      GPIO 36 -| 02 (VP)  29 |- GPIO 22 [LCD 16x2 - I2C SCL]
        [Joystick VRY]      GPIO 39 -| 03 (VN)  28 |- GPIO 01 [TX0 Serial]
                            GPIO 34 -| 04       27 |- GPIO 03 [RX0 Serial]
                            GPIO 35 -| 05       26 |- GPIO 21 [LCD 16x2 - I2C SDA]
                            GPIO 32 -| 06       25 |- GPIO 19 [Driver ULN2003 - IN1]
        [Módulo Laser]      GPIO 33 -| 07       24 |- GPIO 18 [Driver ULN2003 - IN2]
        [Buzzer PWM]        GPIO 25 -| 08       23 |- GPIO 05 [Driver ULN2003 - IN3]
     [2x LEDs Verdes]       GPIO 26 -| 09       22 |- GPIO 17 [Driver ULN2003 - IN4]
    [2x LEDs Amarelos]      GPIO 27 -| 10       21 |- GPIO 16 [Livre]
    [2x LEDs Vermelhos]     GPIO 14 -| 11       20 |- GPIO 04 [Livre]
                            GPIO 12 -| 12       19 |- GPIO 00 [Boot]
                            GPIO 13 -| 13       18 |- GPIO 02 [LED Onboard]
                        GND (Comum) -| 14       17 |- GPIO 15 [Livre]
                  VIN (5V Potência) -| 15       16 |- 3V3 (Saída Regulada 3.3V)
                                     +-------------+
```

### 4.2. Decisões Críticas de Hardware:
1. **Uso Exclusivo do ADC1 para o Joystick:** Os eixos analógicos do Joystick foram alocados em `GPIO 36 (VP)` e `GPIO 39 (VN)`. No ESP32, os canais do conversor analógico ADC2 tornam-se inoperantes quando o rádio Bluetooth é ativado; o uso do ADC1 garante leitura ininterrupta durante a conexão sem fio.
2. **Descarte das Portas 34 e 35 para Saídas:** As portas `GPIO 34` e `35` são fisicamente restritas a entrada (*Input-Only*) no silício do ESP32. O acionamento do motor de passo foi direcionado exclusivamente para saídas digitais bidirecionais de alta velocidade (`GPIOs 19, 18, 5, 17`).
3. **Topologia dos LEDs em "Funil" com Resistores Individuais:** Cada um dos 6 LEDs possui seu próprio resistor de $300\,\Omega$ em série na perna negativa (evitando desbalanceamento de corrente por variação de $V_f$), enquanto os polos positivos de cada par de cor unem-se na protoboard para consumir apenas 1 pino do ESP32 (`D26`, `D27`, `D14`).

---

## 5. Engenharia Mecânica e Design do Gabinete

### 5.1. Estrutura Modular da Caixa
* **Base Operacional (Inferior):** Caixa principal dotada de **4 pilares de papelão nos cantos internos** que sustentam a tampa de forma rígida contra deformações mecânicas. Acomoda internamente a protoboard apoiada em um suporte inclinado a **$45^\circ$**, otimizando o comprimento dos jumpers de 15 cm e eliminando tensões mecânicas nos conectores.
* **Painel Frontal Integrado:** Recortes para o **LCD 16x2**, os **6 LEDs** e o **Joystick Analógico**, mantendo toda a fiação 100% oculta.
* **Mesa Mecânica (Tampa):** O motor de passo 28BYJ-48 foi fixado do lado externo da tampa, com a fiação descendo por um orifício dedicado. O spindle de DVD repousa sobre um calço central nivelador. A transmissão ocorre por meio de um **vão usinado na parede lateral** da câmara escura, onde a correia elástica conecta a polia customizada do motor ao spindle.
* **Câmara Escura e Suporte de Smartphone:** Cúpula superior com 3 paredes internas revestidas de material preto fosco curvado (**fundo infinito**). Abertura frontal posicionada em frente a uma torre elevada para apoio do smartphone com inclinação angular de $30^\circ$ a $45^\circ$.

```text
                      [ CÂMARA ESCURA / ESTÚDIO SUPERIOR ]
                      +----------------------------------+
                      | - 3 paredes internas (Fundo preto|
                      | - Fundo infinito curvo           |
                      | - Vão lateral para o elástico    |
                      | - Frente aberta para o celular   |
                      +==================================+
                      [ TAMPA / MESA MECÂNICA (Teto)     ]
                      - Spindle + DVD (com base em EVA)
                      - Motor de Passo (externo com fiação oculta)
                      +==================================+
                      [ BASE OPERACIONAL (Caixa Principal)]
                      - Fachada: LCD 16x2, Joystick e 6 LEDs
                      - Pilares de apoio nos 4 cantos internos
                      - Protoboard em berço inclinado a 45°
                      - ESP32, Driver ULN2003 e fiação oculta
```

---

## 6. Lógica de Controle e Firmware Embarcado

### 6.1. Destaques da Lógica Implementada
1. **Controle em Meio-Passo (Half-Step):** Acionamento do 28BYJ-48 através de uma matriz de 8 estados ($4096$ passos/volta), proporcionando torque duplicado e atenuação de vibrações.
2. **Algoritmo de Retorno pelo Caminho Angular Mais Curto (*Shortest Angular Path*):**
   No caso de cancelamento por duplo comando para baixo na pausa, o algoritmo calcula o menor trajeto angular até a posição de repouso ($0^\circ$):
   * Se $\theta \le 180^\circ$ ($\text{PassosAcumulados} \le 2048$): o sistema rebobina no sentido anti-horário;
   * Se $\theta > 180^\circ$ ($\text{PassosAcumulados} > 2048$): o sistema avança continuamente no sentido horário completando a volta. Isso minimiza o tempo de reset, reduz o consumo de energia e evita o desgaste mecânico.
3. **Disparo BLE HID Nativo:** O ESP32 atua como um controle de mídia sem fio (`Scanner 3D Shutter`), enviando o evento `KEY_MEDIA_VOLUME_UP` a cada parada estabilizada para acionar a câmera nativa do smartphone sem aplicativos proprietários.
4. **Tratamento de Brownout e Gestão Térmica:** Inclusão de desativação do registrador `RTC_CNTL_BROWN_OUT_REG` para suportar o pico momentâneo de inicialização do rádio BLE, além da desenergização automática das bobinas do motor (`desenergizarMotor()`) durante as pausas para preservar a vida útil do driver.

---

## 7. Fluxo de Reconstrução Tridimensional (Fotogrametria)

```
[ 1. Mesa IoT 360° ] ---> [ 2. Conjunto de Fotos BLE ] ---> [ 3. 3DF Zephyr / Luma AI ] ---> [ 4. Malha 3D (.OBJ / .STL) ]
```

1. **Captura:** O operador trava o foco da câmera (Bloqueio AE/AF) e inicia o escaneamento pelo Joystick. A mesa executa de 16 a 36 fotos com espaçamento milimetricamente idêntico.
2. **Processamento:** As fotos são importadas para o software fotogramétrico (**3DF Zephyr Free**, **Regard3D** ou nuvem **Luma AI**).
3. **Resultado:** Extração de nuvem de pontos densa, geração de malha poligonal texturizada e exportação do arquivo 3D para impressão tridimensional ou engenharia reversa.

---

## 8. Código-Fonte Embarcado Completo (`main.cpp`)

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BleKeyboard.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

/*
 * =========================================================================
 *  SISTEMA AUTOMATIZADO DE DIGITALIZAÇÃO 3D E FOTOGRAMETRIA
 *  Microcontrolador: ESP32 DevKit (30 Pinos)
 *  Atuador: Motor de Passo 28BYJ-48 + Driver ULN2003 (Half-Step 4096)
 *  Interface: LCD 16x2 I2C, 6 LEDs (Topologia Funil), Joystick Analógico
 *  Conectividade: Bluetooth Low Energy (BLE HID Shutter)
 *  Otimização: Shortest Angular Path Homing & Brownout Protection
 * =========================================================================
 */

// ================= PINAGEM ESP32 =================
// Joystick Analógico (Canais ADC1)
const int PIN_JOY_X      = 36; // GPIO36 (VP) - Eixo Horizontal (Esq/Dir)
const int PIN_JOY_Y      = 39; // GPIO39 (VN) - Eixo Vertical (Cima/Baixo)

// Atuadores e Sinalizadores
const int PIN_LASER      = 33; // GPIO33 (Módulo Laser de Alinhamento)
const int PIN_BUZZER     = 25; // GPIO25 (Buzzer Piezoelétrico)
const int PIN_LED_VERDE  = 26; // GPIO26 (2x LEDs Verdes - Standby / Ponto 0)
const int PIN_LED_AMAR   = 27; // GPIO27 (2x LEDs Amarelos - Giro / Pausa)
const int PIN_LED_VERM   = 14; // GPIO14 (2x LEDs Vermelhos - Captura de Foto)

// Driver ULN2003 (Motor de Passo 28BYJ-48)
const int PIN_IN1        = 19;
const int PIN_IN2        = 18;
const int PIN_IN3        = 5;
const int PIN_IN4        = 17;

// Display LCD 16x2 I2C (SDA = GPIO 21, SCL = GPIO 22)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============ MATRIZ CINEMÁTICA HALF-STEP ============
const int PASSOS_VOLTA_COMPLETA = 4096; // 4096 meio-passos por volta de 360°
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

unsigned long tempoPrimeiroToque = 0;
int toquesCima = 0;
int toquesBaixo = 0;
const unsigned long JANELA_DUPLO_TOQUE = 500; // ms

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
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Proteção contra reset por Brownout BLE
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
  digitalWrite(PIN_LASER, HIGH); // Mira laser ligada
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

      // Ajuste dinâmico de resolução angular (Esquerda -4 / Direita +4)
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
          toquesCima = 0;
          tone(PIN_BUZZER, 1400, 180);
          digitalWrite(PIN_LASER, LOW);
          passosAcumulados = 0;
          fotoAtual = 1;
          estadoAtual = EXECUTANDO;
        }
      }

      if (toquesCima > 0 && (millis() - tempoPrimeiroToque > JANELA_DUPLO_TOQUE)) {
        toquesCima = 0;
      }
      break;

    case EXECUTANDO:
      digitalWrite(PIN_LASER, LOW);

      while (fotoAtual <= totalPassos) {
        // 1. Cálculo da posição absoluta de passos
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

          moverMotor(passosParaGirar, 1200);
          passosAcumulados += passosParaGirar;
          desenergizarMotor(); // Proteção térmica
        }

        // 2. Atualiza LCD com ângulo e status BLE
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

        // 3. Pausa de Estabilização Mecânica (600 ms)
        definirLeds(LOW, HIGH, LOW);
        if (!aguardarComChecagemJoystick(600)) return;

        // 4. Disparo Fotográfico via Bluetooth BLE
        definirLeds(HIGH, LOW, LOW); // Vermelho
        bipFoto();
        dispararCameraCelular();

        // 5. Intervalo pós-disparo (1,2 s para captura do smartphone)
        if (!aguardarComChecagemJoystick(1200)) return;

        fotoAtual++;
      }

      // 6. Conclusão dos 360° exatos no Ponto 0
      if (passosAcumulados < PASSOS_VOLTA_COMPLETA) {
        long restoParaZero = PASSOS_VOLTA_COMPLETA - passosAcumulados;
        moverMotor(restoParaZero, 1200);
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
      atualizarTelaMenu();
      break;

    case PAUSADO:
      definirLeds(LOW, HIGH, LOW);
      desenergizarMotor();
      gerenciarComandosPausa();
      break;
  }
}

// ============= DECODIFICADOR DO JOYSTICK =============
ComandoJoy lerJoystick() {
  static bool stickNeutro = true;

  int x = analogRead(PIN_JOY_X);
  int y = analogRead(PIN_JOY_Y);

  // Zona morta central (repouso)
  if (x > 1400 && x < 2800 && y > 1400 && y < 2800) {
    stickNeutro = true;
    return JOY_NONE;
  }

  // Decodificação com compensação de montagem invertida
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

// ============= ACIONAMENTO DO MOTOR DE PASSO =============
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

// ============= ALGORITMO DO CAMINHO MAIS CURTO =============
void retornarAoPontoZero() {
  if (passosAcumulados > 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RESET PONTO 0...");

    long passosRestantes = PASSOS_VOLTA_COMPLETA - passosAcumulados;

    // Se já passou da metade (180°), o caminho mais curto é avançar!
    if (passosAcumulados > (PASSOS_VOLTA_COMPLETA / 2)) {
      lcd.setCursor(0, 1);
      lcd.print("Avanco Rapido ->");
      moverMotor(passosRestantes, 1000);
    } 
    // Se estava na primeira metade, o caminho mais curto é rebobinar!
    else {
      lcd.setCursor(0, 1);
      lcd.print("<- Rebobinando");
      moverMotor(-passosAcumulados, 1000);
    }

    passosAcumulados = 0;
    desenergizarMotor();
  }
}

// ============= INTERFACE E CONTROLE =============
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

  // 2x BAIXO: CANCELAR VIA CAMINHO MAIS CURTO
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
```

---

## 9. Conclusão
O sistema desenvolvido atende com rigor todos os requisitos de um projeto de Engenharia e Internet das Coisas (IoT). A integração de conceitos de *upcycling* de e-lixo viabilizou uma ferramenta mecatrônica de custo R$ 0,00 capaz de gerar conjuntos fotográficos com precisão angular industrial, automatização ponta a ponta sem fio e otimização cinemática de trajetória.
