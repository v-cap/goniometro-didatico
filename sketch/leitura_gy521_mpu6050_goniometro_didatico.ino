#include <Wire.h>

const byte MPU = 0x68;

// Dados brutos
int16_t AcX, AcY, AcZ;
int16_t Temp;
int16_t GyX, GyY, GyZ;

// Offsets do giroscópio
float offsetGX = 0;
float offsetGY = 0;
float offsetGZ = 0;

// Filtro complementar
float roll = 0;
float pitch = 0;

// Referência anatômica
float referencia = 0;

// Controle temporal
unsigned long ultimoCiclo;

// Taxa de amostragem
unsigned long contadorAmostras = 0;
unsigned long ultimoRelatorio = 0;

//--------------------------------------------------
void escreverRegistro(byte reg, byte valor)
{
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  Wire.write(valor);
  Wire.endTransmission(true);
}

//--------------------------------------------------
void configurarMPU()
{
  escreverRegistro(0x6B, 0x00); // Wake Up
  escreverRegistro(0x1A, 0x03); // DLPF 42 Hz
  escreverRegistro(0x1B, 0x00); // Gyro ±250°/s
  escreverRegistro(0x1C, 0x00); // Accel ±2g
  escreverRegistro(0x19, 0x09); // 100 Hz
}

//--------------------------------------------------
void lerMPU()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU, 14, true);

  AcX = (Wire.read() << 8) | Wire.read();
  AcY = (Wire.read() << 8) | Wire.read();
  AcZ = (Wire.read() << 8) | Wire.read();

  Temp = (Wire.read() << 8) | Wire.read();

  GyX = (Wire.read() << 8) | Wire.read();
  GyY = (Wire.read() << 8) | Wire.read();
  GyZ = (Wire.read() << 8) | Wire.read();
}

//--------------------------------------------------
void calibrarGiroscopio()
{
  const int N = 1000;

  long somaGX = 0;
  long somaGY = 0;
  long somaGZ = 0;

  Serial.println();
  Serial.println("=================================");
  Serial.println("CALIBRACAO DO GIROSCOPIO");
  Serial.println("MANTENHA O SENSOR PARADO");
  Serial.println("=================================");
  Serial.println();

  delay(3000);

  for (int i = 0; i < N; i++)
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);

    Wire.requestFrom(MPU, 6, true);

    int16_t gx = (Wire.read() << 8) | Wire.read();
    int16_t gy = (Wire.read() << 8) | Wire.read();
    int16_t gz = (Wire.read() << 8) | Wire.read();

    somaGX += gx;
    somaGY += gy;
    somaGZ += gz;

    delay(2);
  }

  offsetGX = (float)somaGX / N;
  offsetGY = (float)somaGY / N;
  offsetGZ = (float)somaGZ / N;

  Serial.println("CALIBRACAO CONCLUIDA");
  Serial.println();
}

//--------------------------------------------------
void inicializarAngulos()
{
  lerMPU();

  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  roll =
      atan2(ay, az)
      * 180.0 / PI;

  pitch =
      atan2(
          -ax,
          sqrt(ay * ay + az * az))
      * 180.0 / PI;
}

//--------------------------------------------------
void setup()
{
  Serial.begin(115200);

  Wire.begin();

  configurarMPU();

  delay(500);

  calibrarGiroscopio();

  inicializarAngulos();

  ultimoCiclo = micros();
  ultimoRelatorio = millis();

  Serial.println("============================================");
  Serial.println(" GONIOMETRO DIGITAL PARA COTOVELO");
  Serial.println("============================================");
  Serial.println();
  Serial.println("1. Posicione o braco ao lado do corpo");
  Serial.println("2. Antebraco apontando para baixo");
  Serial.println("3. Digite z e pressione ENTER");
  Serial.println("4. Inicie o movimento");
  Serial.println();
}

//--------------------------------------------------
void loop()
{
  //-----------------------------------------
  // Controle de 100 Hz
  //-----------------------------------------

  while ((micros() - ultimoCiclo) < 10000)
  {
  }

  unsigned long agora = micros();

  float dt =
      (agora - ultimoCiclo)
      / 1000000.0;

  ultimoCiclo = agora;

  //-----------------------------------------
  // Leitura
  //-----------------------------------------

  lerMPU();

  //-----------------------------------------
  // Conversão acelerômetro
  //-----------------------------------------

  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  //-----------------------------------------
  // Conversão giroscópio
  //-----------------------------------------

  float gx =
      (GyX - offsetGX) / 131.0;

  float gy =
      (GyY - offsetGY) / 131.0;

  //-----------------------------------------
  // Ângulos do acelerômetro
  //-----------------------------------------

  float rollAcc =
      atan2(ay, az)
      * 180.0 / PI;

  float pitchAcc =
      atan2(
          -ax,
          sqrt(ay * ay + az * az))
      * 180.0 / PI;

  //-----------------------------------------
  // Integração do giroscópio
  //-----------------------------------------

  roll += gx * dt;
  pitch += gy * dt;

  //-----------------------------------------
  // Filtro Complementar
  //-----------------------------------------

  const float alpha = 0.98;

  roll =
      alpha * roll +
      (1.0 - alpha) * rollAcc;

  pitch =
      alpha * pitch +
      (1.0 - alpha) * pitchAcc;

  //-----------------------------------------
  // Definir Referência
  //-----------------------------------------

  if (Serial.available())
  {
    char c = Serial.read();

    if (c == 'z' || c == 'Z')
    {
      referencia = pitch;

      Serial.println();
      Serial.println("*********************************");
      Serial.println("REFERENCIA DEFINIDA");
      Serial.print("Pitch inicial: ");
      Serial.print(referencia, 2);
      Serial.println(" graus");
      Serial.println("*********************************");
      Serial.println();
    }
  }

  //-----------------------------------------
  // Goniômetro
  //-----------------------------------------

  float angulo = fabs(pitch - referencia);

  if (angulo < 0.5)
  {
    angulo = 0;
  }

  //-----------------------------------------
  // Exibição amigável
  //-----------------------------------------

  static unsigned long ultimoDisplay = 0;

  if (millis() - ultimoDisplay >= 200)
  {
    ultimoDisplay = millis();

    Serial.println("---------------------------------");
    Serial.print("Flexao do Cotovelo: ");
    Serial.print(angulo, 1);
    Serial.println(" graus");
  }

  //-----------------------------------------
  // Taxa real de amostragem
  //-----------------------------------------

  contadorAmostras++;

  if (millis() - ultimoRelatorio >= 5000)
  {
    float taxaReal =
        contadorAmostras *
        1000.0 /
        (millis() - ultimoRelatorio);

    // Serial.println("---------------------------------");
    // Serial.print("Taxa de Amostragem: ");
    // Serial.print(taxaReal, 1);
    // Serial.println(" Hz");
    // Serial.println("---------------------------------");

    contadorAmostras = 0;
    ultimoRelatorio = millis();
  }
}
