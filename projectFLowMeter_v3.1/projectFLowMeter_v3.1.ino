#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ECHO 9
#define TRIGGER 10
#define TEMPO_AMOSTRAGEM 500
#define PULSO_TRIGGER 1
#define pushButton 12

int duracaoPulso;
float distancia;
float nivelDaAgua;
bool fora;
bool calibrando = false;

// Valor inicial (pode ser recalibrado)
float DISTANCIA_TOPO = 5; 
const int ALTURA_MAXIMA = 20;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    pinMode(TRIGGER, OUTPUT);
    pinMode(ECHO, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(pushButton, INPUT);

    Serial.begin(9600);

    lcd.init();
    lcd.backlight();

    lcd.setCursor(0, 0);
    lcd.print("Ind. de Nivel");
    lcd.setCursor(0, 1);
    lcd.print("Ultrassonico LI");
    delay(1500);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("(!) indica valor");
    lcd.setCursor(0, 1);
    lcd.print("fora do Span");
    delay(3000);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Ind. de Nivel");
    lcd.setCursor(0, 1);
    lcd.print("Iniciando...");
    delay(1500);
    lcd.clear();
}

void loop() {

    // Pressionou o botão → entra ou sai do modo de calibração
    Serial.println(digitalRead(pushButton));
    if (digitalRead(pushButton) == HIGH) {
        delay(200); // debounce
        calibrando = !calibrando;
        lcd.clear();

        if (calibrando) {
            lcd.setCursor(0, 0);
            lcd.print("Modo Calibracao");
            lcd.setCursor(0, 1);
            lcd.print("Aproxime da agua");
            delay(1500);
            lcd.clear();
        } else {
            lcd.setCursor(0, 0);
            lcd.print("Calibracao OK");
            lcd.setCursor(0, 1);
            lcd.print("Voltando...");
            delay(1200);
            lcd.clear();
        }
    }

    // ---- MODO CALIBRACAO ----
    if (calibrando) {
        modoCalibracao();
        return; // Não roda o modo normal
    }

    // ---- MODO NORMAL ----
    medirDistancia();
    atualizarLCD();
    controlarLED();
    delay(TEMPO_AMOSTRAGEM);
}



// =============================================================
// ==================== FUNÇÃO DE CALIBRAÇÃO ===================
// =============================================================

void modoCalibracao() {

    medirDistancia();

    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(distancia, 1);
    lcd.print("cm    ");

    lcd.setCursor(0, 1);
    lcd.print("Btn = SalvarTopo");

    // Se apertar o botão → salva como nova DISTANCIA_TOPO
    if (digitalRead(pushButton) == HIGH) {
        DISTANCIA_TOPO = distancia;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Novo Topo:");
        lcd.print(DISTANCIA_TOPO, 1);
        lcd.print("cm");

        lcd.setCursor(0, 1);
        lcd.print("Solte o botao");
        delay(2000);
        lcd.clear();
    }
}



// =============================================================
// ================= FUNÇÕES DE MEDIÇÃO / LCD ==================
// =============================================================

void medirDistancia() {

    // Trigger
    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER, HIGH);
    delay(PULSO_TRIGGER);
    digitalWrite(TRIGGER, LOW);

    duracaoPulso = pulseIn(ECHO, HIGH, 30000);

    if (duracaoPulso == 0) {
        distancia = 0;
        nivelDaAgua = 0;
        fora = true;
        return;
    }

    distancia = 0.01716 * duracaoPulso;
    float distanciaAteAgua = distancia - DISTANCIA_TOPO;
    nivelDaAgua = ALTURA_MAXIMA - distanciaAteAgua;

    if (nivelDaAgua < 0 || nivelDaAgua > ALTURA_MAXIMA) {
        fora = true;
    } else {
        fora = false;
    }
}

void atualizarLCD() {
    lcd.setCursor(0, 0);
    lcd.print("Dist:");
    lcd.print(distancia, 1);
    lcd.print("cm   ");

    lcd.setCursor(0, 1);
    lcd.print("Nivel:");
    lcd.print(nivelDaAgua, 1);
    lcd.print("cm ");
    Serial.println(nivelDaAgua);

    if (fora) lcd.print("!   ");
    else lcd.print("    ");
}

void controlarLED() {
    if (fora) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}
