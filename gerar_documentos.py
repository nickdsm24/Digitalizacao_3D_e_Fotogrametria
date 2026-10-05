import os
import sys
import docx
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import OxmlElement
from docx.oxml.ns import qn

def set_cell_background(cell, fill_hex):
    tcPr = cell._tc.get_or_add_tcPr()
    shd = OxmlElement('w:shd')
    shd.set(qn('w:val'), 'clear')
    shd.set(qn('w:color'), 'auto')
    shd.set(qn('w:fill'), fill_hex)
    tcPr.append(shd)

def set_cell_margins(cell, top=100, bottom=100, left=150, right=150):
    tcPr = cell._tc.get_or_add_tcPr()
    tcMar = OxmlElement('w:tcMar')
    for m, val in [('top', top), ('bottom', bottom), ('left', left), ('right', right)]:
        node = OxmlElement(f'w:{m}')
        node.set(qn('w:w'), str(val))
        node.set(qn('w:type'), 'dxa')
        tcMar.append(node)
    tcPr.append(tcMar)

def create_report():
    doc = Document()

    # Page Margins (ABNT: Top/Left 3cm, Bottom/Right 2cm)
    for section in doc.sections:
        section.top_margin = Inches(1.0)
        section.bottom_margin = Inches(1.0)
        section.left_margin = Inches(1.0)
        section.right_margin = Inches(1.0)

    # Styles Setup
    styles = doc.styles
    normal_style = styles['Normal']
    normal_style.font.name = 'Arial'
    normal_style.font.size = Pt(11)
    normal_style.font.color.rgb = RGBColor(0x22, 0x22, 0x22)
    normal_style.paragraph_format.line_spacing = 1.15
    normal_style.paragraph_format.space_after = Pt(6)

    # Colors
    NAVY = RGBColor(0x00, 0x33, 0x66)
    DARK_BLUE = RGBColor(0x00, 0x4D, 0x80)
    GRAY = RGBColor(0x55, 0x55, 0x55)

    # ==================== CAPA / CABEÇALHO ====================
    title_p = doc.add_paragraph()
    title_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    title_p.paragraph_format.space_before = Pt(10)
    title_p.paragraph_format.space_after = Pt(4)
    run_inst = title_p.add_run("CURSO DE ENGENHARIA / TECNOLOGIA EM INTERNET DAS COISAS\nDISCIPLINA DE SISTEMAS EMBARCADOS E IOT")
    run_inst.font.size = Pt(11)
    run_inst.font.bold = True
    run_inst.font.color.rgb = GRAY

    doc.add_paragraph().paragraph_format.space_after = Pt(30)

    title_main = doc.add_paragraph()
    title_main.alignment = WD_ALIGN_PARAGRAPH.CENTER
    title_main.paragraph_format.space_after = Pt(12)
    run_title = title_main.add_run("RELATÓRIO TÉCNICO DE PROJETO FINAL\nSISTEMA AUTOMATIZADO DE DIGITALIZAÇÃO 3D E FOTOGRAMETRIA")
    run_title.font.size = Pt(18)
    run_title.font.bold = True
    run_title.font.color.rgb = NAVY

    sub_title = doc.add_paragraph()
    sub_title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    sub_title.paragraph_format.space_after = Pt(40)
    run_sub = sub_title.add_run("Plataforma Mecatrônica de Precisão com Controle Cinemático por ESP32,\nInterface IHM Analógica, Mira Óptica e Sustentabilidade por Upcycling")
    run_sub.font.size = Pt(12)
    run_sub.font.italic = True
    run_sub.font.color.rgb = DARK_BLUE

    doc.add_paragraph().paragraph_format.space_after = Pt(60)

    info_box = doc.add_paragraph()
    info_box.alignment = WD_ALIGN_PARAGRAPH.RIGHT
    info_box.paragraph_format.space_after = Pt(50)
    run_info = info_box.add_run("Trabalho Final de Conclusão da Disciplina de IoT\nMicrocontrolador: ESP32 NodeMCU-32S (Dual-Core)\nData de Entrega: 05 de Outubro de 2026")
    run_info.font.size = Pt(10.5)
    run_info.font.bold = True
    run_info.font.color.rgb = GRAY

    doc.add_page_break()

    # Helper Functions
    def add_h1(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(16)
        p.paragraph_format.space_after = Pt(8)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.size = Pt(14)
        run.font.bold = True
        run.font.color.rgb = NAVY
        return p

    def add_h2(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(12)
        p.paragraph_format.space_after = Pt(4)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.font.size = Pt(12)
        run.font.bold = True
        run.font.color.rgb = DARK_BLUE
        return p

    def add_bullet(bold_prefix, text):
        p = doc.add_paragraph(style='List Bullet')
        p.paragraph_format.space_after = Pt(3)
        r1 = p.add_run(bold_prefix + " ")
        r1.font.bold = True
        p.add_run(text)

    # ==================== 1. DESCRIÇÃO GERAL ====================
    add_h1("1. DESCRIÇÃO GERAL DA APLICAÇÃO")
    
    add_h2("1.1 Contextualização e Definição do Problema")
    doc.add_paragraph(
        "A fotogrametria digital é uma técnica computacional e de sensoriamento óptico que permite reconstruir malhas tridimensionais "
        "densas (.OBJ / .STL) com texturas fotorrealistas a partir de um conjunto de fotografias bidimensionais sobrepostas de um objeto real. "
        "No entanto, o processo manual tradicional de captura apresenta três gargalos críticos que comprometem a convergência dos algoritmos de Structure-from-Motion (SfM):"
    )
    add_bullet("1. Inconsistência no Espaçamento Angular:", "A rotação manual do objeto introduz ângulos irregulares entre as fotos, gerando lacunas e perda de pontos homólogos na nuvem de pontos.")
    add_bullet("2. Vibrações e Desfoque de Movimento (Motion Blur):", "A ausência de um tempo padronizado de amortecimento mecânico gera fotos borradas que inviabilizam o alinhamento de feixes.")
    add_bullet("3. Erros de Centralização Geométrica:", "Objetos posicionados fora do centro exato de rotação sofrem distorções severas de paralaxe e oclusão de geometria.")
    doc.add_paragraph(
        "O Sistema Automatizado de Digitalização 3D e Fotogrametria soluciona integralmente esses problemas através de uma bancada mecatrônica "
        "de alta precisão controlada pelo microcontrolador ESP32, integrando controle de micropassos calibrados (5120 passos/volta), mira laser colimada, display LCD 16x2 I2C, "
        "joystick analógico com auto-calibração no boot e sinalização audiovisual sincronizada."
    )

    add_h2("1.2 Objetivos do Projeto")
    doc.add_paragraph("O projeto foi orientado pelos seguintes objetivos técnicos e acadêmicos:")
    add_bullet("• Objetivo Geral:", "Projetar, construir, programar e validar uma bancada mecatrônica automatizada de baixo custo para digitalização tridimensional padronizada de objetos por fotogrametria.")
    add_bullet("• Controle Cinemático Calibrado:", "Implementar acionamento em meio-passo suave (5120 passos/volta completa calibrados) com resolução de 0,0703° por passo, garantindo fechamento perfeito de 360° sem folgas.")
    add_bullet("• Sincronismo Audiovisual de Captura:", "Integrar tempos de estabilização mecânica (800ms) com sinalização óptica (LEDs) e sonora (Buzzer) para disparo fotográfico preciso.")
    add_bullet("• Algoritmo de Retorno Inteligente:", "Desenvolver lógica cinemática de retorno à origem (0°) pelo caminho angular mais curto (Shortest Angular Path) em caso de cancelamento.")
    add_bullet("• Alinhamento Óptico por Laser:", "Incorporar mira laser colimada (reaproveitada de brinquedo Nerf) para centralização milimétrica do objeto antes do início do escaneamento, com desligamento automático durante as fotos.")

    add_h2("1.3 Motivação e Sustentabilidade (Upcycling de E-Lixo)")
    add_bullet("• Sustentabilidade e Upcycling Integrado:", "Reaproveitamento duplo de materiais descartados: a estrutura mecânica e rolamento de precisão de um leitor óptico de CD/DVD, somado ao módulo de mira laser retirado de uma pistola de brinquedo Nerf.")
    add_bullet("• Custo R$ 0,00 em Novos Componentes:", "Maximização total dos componentes eletrônicos do kit didático da disciplina (ESP32, motor 28BYJ-48, driver ULN2003, display I2C, 6 LEDs, buzzer, 1 protoboard 800 pontos e joystick), sem necessidade de novos custos financeiros.")
    add_bullet("• Aplicação Prática:", "Construção de uma ferramenta real e acessível para engenharia reversa, prototipagem rápida, impressão 3D, inspeção metrológica e preservação digital de peças.")

    # ==================== 2. ARQUITETURA DO SISTEMA ====================
    add_h1("2. DESENHO DA ARQUITETURA DO SISTEMA")
    
    add_h2("2.1 Arquitetura em Camadas")
    doc.add_paragraph(
        "O sistema foi estruturado em três camadas bem definidas para garantir modularidade, manutenibilidade e isolamento elétrico entre a lógica e a potência:"
    )
    add_bullet("1. Camada de Aplicação e IHM:", "Composta pelo Display LCD 16x2 I2C (exibição de telemetria, fotos e ângulos), Joystick analógico de 2 eixos, 6x LEDs de estado em topologia funil com resistores individuais de 300 Ω, Buzzer piezoelétrico e Diodo Laser de mira.")
    add_bullet("2. Camada de Processamento e Controle (ESP32):", "Executa a Máquina de Estados Finitos (FSM), a auto-calibração dinâmica do centro neutro do joystick, a filtragem por sobre-amostragem (16x), o controle de micropassos calibrados (5120 passos) e o algoritmo do caminho angular mais curto.")
    add_bullet("3. Camada de Atuação Mecatrônica de Potência:", "Composta pelo Driver ULN2003 (array de transistores Darlington) e o Motor de Passo 28BYJ-48 alimentados por barramento externo de 5V com GND compartilhado e desenergização térmica automática das bobinas.")

    add_h2("2.2 Máquina de Estados Finitos (FSM)")
    doc.add_paragraph("O firmware opera sob uma máquina de estados finitos determinística com 4 modos principais:")
    add_bullet("• Estado 1: MODO TESTE INICIAL:", "Executado logo após a calibração no boot. Os LEDs ciclam suavemente e a mira laser acende. Transiciona para o Modo de Execução se o joystick for movido 3 vezes na mesma direção ou após o timeout de 60 segundos.")
    add_bullet("• Estado 2: MENU DE EXECUÇÃO:", "Permite ajustar a densidade de fotos com o Joystick para Esquerda (-4) ou Direita (+4), variando entre 8, 12, 16, 24 e 36 fotos. O Laser de mira permanece aceso para ajuste fino da peça. Dois toques para CIMA (▲▲) iniciam o scan.")
    add_bullet("• Estado 3: EXECUTANDO SCAN 360°:", "O laser é desligado automaticamente para não interferir na iluminação da foto. Para cada ângulo, a mesa gira, aguarda a pausa de estabilização (800ms), aciona o LED vermelho e emite o bip de captura. Dois toques para BAIXO (▼▼) pausam o sistema.")
    add_bullet("• Estado 4: PAUSADO:", "O motor é desenergizado e o LED amarelo acende com o laser reativado. Dois toques para CIMA (▲▲) despausam e continuam o scan. Dois toques para BAIXO (▼▼) cancelam e acionam o retorno inteligente ao ponto 0°.")

    add_h2("2.3 Calibração Metrológica do Motor de Passo e Algoritmo do Caminho Mais Curto")
    doc.add_paragraph(
        "Durante a validação prática na bancada, observou-se que o motor 28BYJ-48 operando com a constante teórica de 4096 passos deixava uma margem residual de aproximadamente 72° para fechar a volta completa. "
        "Realizou-se a calibração experimental da caixa de redução interna, estabelecendo a constante real de 5120 passos por volta completa (resolução de 0,0703°/passo), resultando em um fechamento angular de 360,00° exato.\n\n"
        "Em caso de cancelamento da digitalização na pausa, o ESP32 calcula dinamicamente a menor distância angular para retornar à origem (0°):\n"
        "• Se Passos Acumulados > 2560 (> 180°): O motor avança no sentido horário os passos restantes (5120 - Passos Acumulados).\n"
        "• Se Passos Acumulados <= 2560 (<= 180°): O motor rebobina no sentido anti-horário os passos já dados (-Passos Acumulados).\n"
        "Isso reduz o tempo de retorno em até 50% e evita estresse mecânico desnecessário."
    )

    # ==================== 3. RELAÇÃO DE MATERIAIS (BOM) ====================
    add_h1("3. RELAÇÃO DE PARTES E MATERIAIS (BOM)")
    doc.add_paragraph("A tabela a seguir discrimina todos os componentes empregados no protótipo final atualizado:")

    table_bom = doc.add_table(rows=1, cols=5)
    table_bom.alignment = WD_TABLE_ALIGNMENT.CENTER
    hdr_cells = table_bom.rows[0].cells
    headers = ["Item", "Categoria", "Componente / Especificação", "Qtd.", "Função no Sistema"]
    for i, h in enumerate(headers):
        hdr_cells[i].text = h
        hdr_cells[i].paragraphs[0].runs[0].font.bold = True
        hdr_cells[i].paragraphs[0].runs[0].font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)
        set_cell_background(hdr_cells[i], "003366")
        set_cell_margins(hdr_cells[i], 120, 120, 150, 150)

    bom_data = [
        ("1", "Processamento", "Placa ESP32 NodeMCU-32S (Xtensa Dual-Core 240MHz)", "1 un.", "Núcleo de controle em tempo real e IHM"),
        ("2", "Atuação", "Motor de Passo Unipolar 28BYJ-48 (5V DC, Redução Calibrada)", "1 un.", "Tração angular com 5120 passos/volta completa"),
        ("3", "Potência", "Módulo Driver ULN2003 (Array Darlington)", "1 un.", "Chaveamento de corrente das 4 bobinas"),
        ("4", "Display / IHM", "Display LCD 16x2 com Módulo Adaptador I2C (PCF8574)", "1 un.", "Exibição de telemetria, fotos e ângulos"),
        ("5", "Entrada / IHM", "Módulo Joystick Analógico 2 Eixos (10k)", "1 un.", "Navegação e comandos de controle"),
        ("6", "Mira Óptica", "Módulo Diodo Laser Vermelho 650nm (Upcycling / Pistola Nerf)", "1 un.", "Alinhamento e centralização milimétrica da peça"),
        ("7", "Sinalização Sonora", "Buzzer Piezoelétrico 5V", "1 un.", "Alertas sonoros de início, foto e fim de ciclo"),
        ("8", "Sinalização Óptica", "6x LEDs 5mm (2 Verdes, 2 Amarelos, 2 Vermelhos em Funil)", "6 un.", "Indicação visual dos estados do sistema"),
        ("9", "Proteção", "Resistores de Carbono 300 Ω (1/4W) [6 p/ LEDs + 1 p/ Laser]", "7 un.", "Limitação individual de corrente dos LEDs e do Laser"),
        ("10", "Montagem", "Protoboard de 800 Pontos (Matriz Principal)", "1 un.", "Distribuição de sinais e barramentos de bancada"),
        ("11", "Mecânica", "Estrutura Mecânica de Leitor de CD/DVD (Upcycling)", "1 un.", "Suporte giratório e rolamento de precisão"),
        ("12", "Conexões", "Conjunto de Cabos Jumpers Flexíveis (MM, MF, FF)", "Conj.", "Interconexão elétrica dos circuitos"),
        ("13", "Alimentação", "Fonte Externa 5V/2A + Cabo USB", "1 un.", "Alimentação de potência e lógica com GND comum")
    ]

    for row_idx, data in enumerate(bom_data):
        row_cells = table_bom.add_row().cells
        bg_color = "F4F7FA" if row_idx % 2 == 1 else "FFFFFF"
        for col_idx, text in enumerate(data):
            row_cells[col_idx].text = text
            set_cell_background(row_cells[col_idx], bg_color)
            set_cell_margins(row_cells[col_idx], 80, 80, 120, 120)
            row_cells[col_idx].paragraphs[0].runs[0].font.size = Pt(9.5)

    doc.add_paragraph().paragraph_format.space_after = Pt(10)

    # ==================== 4. DIAGRAMA DE CONEXÃO ====================
    add_h1("4. DIAGRAMA DE CONEXÃO DOS COMPONENTES ELETRÔNICOS")
    
    add_h2("4.1 Pinout Geral do ESP32 NodeMCU-32S (30 Pinos)")
    doc.add_paragraph(
        "A distribuição dos pinos no ESP32 foi projetada para evitar conflitos de barramento e garantir que os canais analógicos "
        "do joystick operem exclusivamente no bloco ADC1 (GPIOs 36 e 39), mantendo os pinos I2C dedicados no barramento padrão (GPIOs 21 e 22):"
    )

    table_pin = doc.add_table(rows=1, cols=4)
    table_pin.alignment = WD_TABLE_ALIGNMENT.CENTER
    hdr_p = table_pin.rows[0].cells
    for i, h in enumerate(["Componente", "Pino do Módulo", "Pino no ESP32", "Descrição / Função"]):
        hdr_p[i].text = h
        hdr_p[i].paragraphs[0].runs[0].font.bold = True
        hdr_p[i].paragraphs[0].runs[0].font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)
        set_cell_background(hdr_p[i], "004D80")
        set_cell_margins(hdr_p[i], 100, 100, 120, 120)

    pin_data = [
        ("Display LCD 16x2", "SDA", "GPIO 21 (D21)", "Linha de Dados I2C (Endereço 0x27)"),
        ("Display LCD 16x2", "SCL", "GPIO 22 (D22)", "Linha de Clock I2C"),
        ("Display LCD 16x2", "VCC / GND", "VIN (5V) / GND", "Alimentação do display e backlight"),
        ("Driver ULN2003", "IN1", "GPIO 19 (D19)", "Bobina A do Motor de Passo"),
        ("Driver ULN2003", "IN2", "GPIO 18 (D18)", "Bobina B do Motor de Passo"),
        ("Driver ULN2003", "IN3", "GPIO 5 (D5)", "Bobina C do Motor de Passo"),
        ("Driver ULN2003", "IN4", "GPIO 17 (TX2)", "Bobina D do Motor de Passo"),
        ("Driver ULN2003", "(+) / (-)", "Fonte 5V / GND", "Alimentação de potência externa com jumper ON"),
        ("Joystick Analógico", "VRX", "GPIO 36 (VP)", "Eixo Horizontal (Leitura ADC1_CH0)"),
        ("Joystick Analógico", "VRY", "GPIO 39 (VN)", "Eixo Vertical (Leitura ADC1_CH3)"),
        ("Joystick Analógico", "VCC / GND", "3V3 / GND", "Alimentação regulada de 3.3V para precisão"),
        ("Mira Laser (Nerf)", "(+) / (-)", "GPIO 33 (D33) / GND", "Controle óptico via resistor limitador de 300 Ω"),
        ("LEDs Verdes (2x)", "Anodo (+)", "GPIO 26 (D26)", "Sinalização de Pronto / Standby (com resistores de 300 Ω)"),
        ("LEDs Amarelos (2x)", "Anodo (+)", "GPIO 27 (D27)", "Sinalização de Rotação / Pausa (com resistores de 300 Ω)"),
        ("LEDs Vermelhos (2x)", "Anodo (+)", "GPIO 14 (D14)", "Sinalização de Captura de Foto (com resistores de 300 Ω)"),
        ("Buzzer Piezo", "(+) / (-)", "GPIO 25 (D25) / GND", "Emissão de tons PWM de feedback sonoro")
    ]

    for r_idx, data in enumerate(pin_data):
        r_cells = table_pin.add_row().cells
        bg = "F4F7FA" if r_idx % 2 == 1 else "FFFFFF"
        for c_idx, text in enumerate(data):
            r_cells[c_idx].text = text
            set_cell_background(r_cells[c_idx], bg)
            set_cell_margins(r_cells[c_idx], 70, 70, 100, 100)
            r_cells[c_idx].paragraphs[0].runs[0].font.size = Pt(9.5)

    add_h2("4.2 Topologia Elétrica e Barramento de GND Comum")
    doc.add_paragraph(
        "Para eliminar ruídos induzidos pelas bobinas indutivas do motor e prevenir quedas de tensão na lógica do microcontrolador:\n"
        "1. O Driver ULN2003 é energizado diretamente pela fonte de alimentação externa de 5V.\n"
        "2. O polo negativo (GND) da fonte externa é interligado diretamente ao pino GND do ESP32, unificando a referência de terra de todo o circuito.\n"
        "3. Os potenciômetros do joystick são alimentados pelo pino 3V3 do ESP32 para garantir estabilidade métrica no conversor analógico-digital (ADC1)."
    )

    # ==================== 5. CÓDIGO-FONTE EMBARCADO ====================
    add_h1("5. CÓDIGO-FONTE EMBARCADO COMPLETO")
    doc.add_paragraph(
        "Abaixo é apresentado o código-fonte integral em C++ desenvolvido no ecossistema PlatformIO / Arduino Framework, "
        "contendo todas as rotinas de controle de micropassos calibrados (5120 passos/volta), máquina de estados, calibração dinâmica e desenergização térmica:"
    )

    # Read current main.cpp
    main_cpp_path = r"C:\Users\Nmpg2\Documents\02_Estudos\02_Faculdade\03_Internet_das_Coisas\Digitalizacao_3D_e_Fotogrametria\src\main.cpp"
    code_content = ""
    if os.path.exists(main_cpp_path):
        with open(main_cpp_path, 'r', encoding='utf-8') as f:
            code_content = f.read()

    # Add code in a shaded box
    p_code = doc.add_paragraph()
    p_code.paragraph_format.space_before = Pt(6)
    p_code.paragraph_format.space_after = Pt(6)
    p_code.paragraph_format.line_spacing = 1.0
    run_code = p_code.add_run(code_content)
    run_code.font.name = 'Consolas'
    run_code.font.size = Pt(8.0)
    run_code.font.color.rgb = RGBColor(0x11, 0x11, 0x11)

    # ==================== 6. ROTEIRO DE APRESENTAÇÃO ====================
    add_h1("6. ROTEIRO DE APRESENTAÇÃO E DEMONSTRAÇÃO (15 MINUTOS)")
    
    add_h2("6.1 Planejamento do Tempo e Estrutura de Slides")
    doc.add_paragraph(
        "Para cumprir rigorosamente o limite de 15 minutos com máxima clareza e impacto diante da banca avaliadora, a apresentação foi estruturada no seguinte formato:"
    )
    add_bullet("• Minuto 00:00 a 03:00 (Introdução e Motivação):", "Apresentação da equipe, contextualização do problema da fotogrametria manual e demonstração do conceito de sustentabilidade e upcycling duplo (leitor de CD/DVD descartado + laser de brinquedo Nerf).")
    add_bullet("• Minuto 03:00 a 06:00 (Arquitetura e Engenharia):", "Explicação do diagrama de blocos, esquema elétrico de potência, controle em meio-passo calibrado de 5120 passos/volta e algoritmo do caminho mais curto.")
    add_bullet("• Minuto 06:00 a 12:00 (Demonstração Prática ao Vivo na Bancada):", "Execução prática do protótipo ligando o sistema, calibrando, alinhando com a mira laser, ajustando fotos no LCD, iniciando o scan e demonstrando a pausa e o cancelamento inteligente.")
    add_bullet("• Minuto 12:00 a 15:00 (Resultados e Conclusão):", "Exibição do modelo 3D reconstruído em software de fotogrametria e considerações finais.")

    add_h2("6.2 Script da Demonstração Prática ao Vivo na Bancada")
    add_bullet("1. Inicialização e Calibração:", "\"Ao ligar a bancada, o ESP32 calibra dinamicamente o centro neutro do joystick e entra em modo de teste com os LEDs ciclando.\"")
    add_bullet("2. Transição e Mira Laser:", "\"Damos 3 toques no joystick para entrar no Modo de Execução. Notem que a mira laser acende automaticamente, permitindo alinhar e centralizar a peça com precisão milimétrica no centro do prato giratório.\"")
    add_bullet("3. Ajuste de Resolução no Display:", "\"Com o joystick para esquerda ou direita, escolhemos a quantidade exata de fotos no LCD (ex: 24 fotos para uma peça com detalhes médios).\"")
    add_bullet("4. Execução do Scan 360°:", "\"Dois toques para CIMA iniciam o scan. O laser apaga automaticamente para não poluir a foto, a mesa rotaciona suavemente com os 5120 passos calibrados, estabiliza as vibrações mecânicas e o LED vermelho com o bip sinalizam o momento exato de captura da imagem.\"")
    add_bullet("5. Pausa e Cancelamento Inteligente:", "\"Durante a rotação, damos dois toques para BAIXO para pausar. Ao cancelar, o ESP32 calcula o caminho mais curto até a origem (0°) e desenergiza o motor imediatamente para evitar aquecimento.\"")

    # Output paths
    output_docx = r"C:\Users\Nmpg2\Documents\02_Estudos\02_Faculdade\03_Internet_das_Coisas\Digitalizacao_3D_e_Fotogrametria\docs\relatorio_final_projeto.docx"
    doc.save(output_docx)
    print(f"Documento DOCX salvo com sucesso em: {output_docx}")
    return output_docx

if __name__ == '__main__':
    create_report()
