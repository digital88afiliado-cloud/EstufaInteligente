from gpiozero import Button, LED, Servo
from RPLCD.i2c import CharLCD
import time
import analogio  # Supondo uso de placa compatível como Raspberry Pi Pico
import board

# LCD via I2C (ajuste endereço conforme necessário)
lcd = CharLCD('PCF8574', 0x27)

# Servo motor
janela = Servo(board.GP6)

# Sensores analógicos
sensor_temp = analogio.AnalogIn(board.A3)
sensor_solo = analogio.AnalogIn(board.A0)
sensor_ar = analogio.AnalogIn(board.A1)
ldr = analogio.AnalogIn(board.A2)

# Botão com pull-up
botao = Button(board.A4, pull_up=True)

# Atuadores
aquecedor = LED(3)
luz_crescimento = LED(4)
rele = LED(5)
alerta = LED(13)

# Parâmetros ideais
TEMP_IDEAL_MIN = 22.0
TEMP_IDEAL_MAX = 28.0
UMID_AR_IDEAL_MIN = 60.0
UMID_SOLO_SECO = 400
LUZ_IDEAL_MIN = 600

modo_automatico = True
ultimo_monitoramento = time.time()
intervalo_monitoramento = 0.5  # segundos

# Mensagem de modo
tempo_mensagem_modo = 0
duracao_mensagem_modo = 1.0
mostrando_mensagem_modo = False

def analog_to_voltage(pin):
    return pin.value * 3.3 / 65535

def mostrar_mensagem_modo(automatico):
    global mostrando_mensagem_modo, tempo_mensagem_modo
    mostrando_mensagem_modo = True
    tempo_mensagem_modo = time.time()
    lcd.clear()
    lcd.write_string("Modo " + ("Automatico" if automatico else "Manual") + "\nAtivado")

def controlar_temperatura(temp):
    if temp > TEMP_IDEAL_MAX:
        rele.on()
        aquecedor.off()
        janela.max()
    elif temp < TEMP_IDEAL_MIN:
        aquecedor.on()
        rele.off()
        janela.min()
    else:
        aquecedor.off()
        rele.off()
        janela.min()

def controlar_irrigacao(umidade_solo):
    if umidade_solo < UMID_SOLO_SECO:
        print("ACIONANDO BOMBA DE IRRIGACAO")

def controlar_iluminacao(luz):
    if luz < LUZ_IDEAL_MIN:
        luz_crescimento.on()
    else:
        luz_crescimento.off()

def atualizar_display(temp, umid_ar, umid_solo, luz):
    lcd.clear()
    lcd.write_string(f"T:{temp:.1f}C U.A:{int(umid_ar)}%\nU.S:{umid_solo} L:{luz}")

def atualizar_display_inicial():
    lcd.write_string("Aguardando dados\nsensores...")

def verificar_alerta(temp):
    if temp > TEMP_IDEAL_MAX or temp < TEMP_IDEAL_MIN:
        alerta.on()
    else:
        alerta.off()

# Inicialização
lcd.clear()
lcd.write_string("Estufa\nInteligente")
time.sleep(1.5)
lcd.clear()
atualizar_display_inicial()

# Loop principal
while True:
    if botao.is_pressed:
        modo_automatico = not modo_automatico
        mostrar_mensagem_modo(modo_automatico)
        time.sleep(0.2)

    tempo_atual = time.time()

    if mostrando_mensagem_modo and (tempo_atual - tempo_mensagem_modo >= duracao_mensagem_modo):
        mostrando_mensagem_modo = False
        lcd.clear()
        atualizar_display_inicial()

    if not mostrando_mensagem_modo and (tempo_atual - ultimo_monitoramento >= intervalo_monitoramento):
        ultimo_monitoramento = tempo_atual

        tensao = analog_to_voltage(sensor_temp)
        temperatura = (tensao - 0.5) * 100.0

        umidade_solo = sensor_solo.value
        umidade_ar = sensor_ar.value
        luz = ldr.value
        umidade_ar_percentual = int((umidade_ar / 65535) * 100)

        if modo_automatico:
            controlar_temperatura(temperatura)
            controlar_irrigacao(umidade_solo)
            controlar_iluminacao(luz)

        atualizar_display(temperatura, umidade_ar_percentual, umidade_solo, luz)
        verificar_alerta(temperatura)

    time.sleep(0.05)
