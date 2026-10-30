#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "BluetoothSerial.h"

// =======================
// PIN SETUP ESP32
// =======================
#define SDA_PIN 21
#define SCL_PIN 22
#define VIBRATION_PIN 25

// =======================
// WIFI ACCESS POINT
// =======================
const char* apSSID = "SORE-Monitor";
const char* apPassword = "GANTI_PASSWORD_AP_2026"; // UBAH sebelum perangkat digunakan

IPAddress localIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
BluetoothSerial SerialBT;

// =======================
// MPU6050 SETUP
// =======================
const int MPU_ADDR = 0x68;

int16_t AcX, AcY, AcZ;
int16_t GyX, GyY, GyZ;
int16_t TempRaw;

float angleX = 0; // Roll relatif orientasi modul (derajat)
float angleY = 0; // Pitch relatif orientasi modul (derajat)
bool sensorOK = true;
uint8_t sensorReadFailures = 0;

// =======================
// TRAINING DATA
// =======================
float totalTegak = 0;
float totalTegakX = 0;
int jumlahTegak = 0;
float baselineTegakX = 0;
float baselineTegakY = 0;
bool tegakSudahDilatih = false;

float totalBongkok = 0;
float totalBongkokX = 0;
int jumlahBongkok = 0;
float baselineBongkokX = 0;
float baselineBongkokY = 0;
bool bongkokSudahDilatih = false;

// =======================
// POSTURE SETUP
// =======================
unsigned long bendDelay = 3000; // 3 detik

// Target awal rancangan: deviasi pitch ATAU roll lebih dari 15 derajat.
const float bendThreshold = 15.0;
// Histeresis: setelah motor menyala, baru berhenti pada deviasi <= 8 derajat.
const float uprightReleaseThreshold = 8.0;

bool isBending = false;
bool warningAlreadyCounted = false;
bool vibrationLockedOn = false;
unsigned long bendStartTime = 0;

// =======================
// SYSTEM STATUS
// =======================
bool systemActive = false;

int dailyWarningCount = 0;
unsigned long totalBadPostureTime = 0;
unsigned long lastLoopTime = 0;

String postureStatus = "BELUM TRAINING";
String vibrationStatus = "OFF";

float jarakKeTegak = 0; // Deviasi maksimum antara pitch/roll terhadap tegak
float jarakKeBongkok = 0;
float jarakPitch = 0;
float jarakRoll = 0;
float currentBendDuration = 0;

// =======================
// DASHBOARD HTML RINGAN
// =======================
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>SORE Smart Posture Reminder</title>

<style>
body{
  margin:0;
  padding:14px;
  font-family:Arial,sans-serif;
  background:#DDF2FF;
  color:#0F172A;
}
.wrap{
  max-width:520px;
  margin:auto;
}
.header{
  background:linear-gradient(135deg,#003B73,#008C9E);
  color:white;
  padding:22px;
  border-radius:22px;
  margin-bottom:14px;
  box-shadow:0 8px 18px rgba(0,0,0,.18);
}
.logo{
  font-size:42px;
  font-weight:900;
  letter-spacing:2px;
}
.sub{
  font-size:16px;
  font-weight:bold;
  margin-top:5px;
}
.card{
  background:white;
  border-radius:20px;
  padding:16px;
  margin-bottom:14px;
  box-shadow:0 6px 16px rgba(0,0,0,.12);
}
.status{
  text-align:center;
  font-size:24px;
  font-weight:900;
  padding:18px;
  border-radius:18px;
  background:#FEF3C7;
  color:#B45309;
}
.net{
  text-align:center;
  font-size:12px;
  color:#64748B;
  margin-top:8px;
}
.grid{
  display:grid;
  grid-template-columns:1fr 1fr;
  gap:10px;
}
.box{
  background:#F8FBFF;
  border-radius:16px;
  padding:14px;
  text-align:center;
}
.label{
  font-size:12px;
  color:#64748B;
  font-weight:bold;
}
.value{
  font-size:25px;
  color:#003B73;
  font-weight:900;
  margin-top:5px;
}
.row{
  display:flex;
  justify-content:space-between;
  gap:10px;
  border-bottom:1px solid #E2E8F0;
  padding:10px 0;
  font-size:14px;
}
.row:last-child{
  border-bottom:none;
}
.btns{
  display:grid;
  grid-template-columns:1fr 1fr;
  gap:10px;
}
button{
  border:none;
  border-radius:14px;
  padding:14px 8px;
  color:white;
  font-weight:900;
  font-size:13px;
}
button:active{
  transform:scale(.98);
}
.tegak{background:#004B87;}
.bongkok{background:#7C3AED;}
.on{background:#2FA84F;}
.off{background:#D32F2F;}
.reset{background:#008C9E;}
.dark{background:#0F766E;}
.footer{
  text-align:center;
  font-size:12px;
  color:#475569;
  margin-top:10px;
}
@media(max-width:480px){
  .grid{grid-template-columns:1fr;}
  .btns{grid-template-columns:1fr;}
}
</style>
</head>

<body>
<div class="wrap">

  <div class="header">
    <div class="logo">SORE</div>
    <div class="sub">Smart Posture Reminder</div>
  </div>

  <div class="card">
    <div id="statusBox" class="status">BELUM TRAINING</div>
    <div id="net" class="net">Menghubungkan...</div>
  </div>

  <div class="card">
    <div class="grid">
      <div class="box">
        <div class="label">Peringatan Sejak Reset</div>
        <div class="value"><span id="warning">0</span>x</div>
      </div>
      <div class="box">
        <div class="label">Durasi Tidak Sesuai</div>
        <div class="value"><span id="duration">0.0</span></div>
      </div>
      <div class="box">
        <div class="label">Sample Tegak</div>
        <div class="value" id="sampleTegak">0</div>
      </div>
      <div class="box">
        <div class="label">Sample Bongkok</div>
        <div class="value" id="sampleBongkok">0</div>
      </div>
    </div>
  </div>

  <div class="card">
    <div class="row"><b>Sudut pitch (Y)</b><span><span id="angle">0.00</span>&deg;</span></div>
    <div class="row"><b>Sudut roll (X)</b><span><span id="angleX">0.00</span>&deg;</span></div>
    <div class="row"><b>Baseline tegak pitch</b><span><span id="tegak">0.00</span>&deg;</span></div>
    <div class="row"><b>Baseline tegak roll</b><span><span id="tegakX">0.00</span>&deg;</span></div>
    <div class="row"><b>Baseline bongkok pitch</b><span><span id="bongkok">0.00</span>&deg;</span></div>
    <div class="row"><b>Deviasi pitch</b><span><span id="jarakPitch">0.00</span>&deg;</span></div>
    <div class="row"><b>Deviasi roll</b><span><span id="jarakRoll">0.00</span>&deg;</span></div>
    <div class="row"><b>Deviasi maksimum</b><span><span id="jarakTegak">0.00</span>&deg;</span></div>
    <div class="row"><b>Jarak ke baseline bongkok</b><span><span id="jarakBongkok">0.00</span>&deg;</span></div>
    <div class="row"><b>Durasi postur menyimpang</b><span><span id="bendTime">0.0</span> detik</span></div>
    <div class="row"><b>Motor getar</b><span id="vibration">OFF</span></div>
    <div class="row"><b>Sistem</b><span id="system">OFF</span></div>
  </div>

  <div class="card">
    <div class="btns">
      <button class="tegak" onclick="cmd('/train-tegak')">Simpan Tegak</button>
      <button class="bongkok" onclick="cmd('/train-bongkok')">Simpan Bongkok</button>
      <button class="on" onclick="cmd('/start')">Mulai Sistem</button>
      <button class="off" onclick="cmd('/off')">Matikan Sistem</button>
      <button class="reset" onclick="cmd('/reset-report')">Reset Laporan</button>
      <button class="dark" onclick="cmd('/reset-training')">Reset Training</button>
    </div>
  </div>

  <div class="footer">Dashboard SORE - ESP32 Web Monitoring</div>

</div>

<script>
async function cmd(url){
  document.getElementById("net").innerText = "Mengirim perintah...";
  try{
    await fetch(url + "?t=" + Date.now(), {cache:"no-store"});
    await updateData();
    document.getElementById("net").innerText = "Terhubung";
  }catch(e){
    document.getElementById("net").innerText = "Gagal kirim perintah";
  }
}

async function updateData(){
  try{
    let res = await fetch("/status?t=" + Date.now(), {cache:"no-store"});
    let d = await res.json();

    document.getElementById("net").innerText = "Terhubung";
    document.getElementById("statusBox").innerText = d.postureStatus;
    document.getElementById("warning").innerText = d.dailyWarningCount;
    document.getElementById("duration").innerText = d.totalBadMinutes;
    document.getElementById("sampleTegak").innerText = d.jumlahTegak;
    document.getElementById("sampleBongkok").innerText = d.jumlahBongkok;
    document.getElementById("angle").innerText = d.angleY;
    document.getElementById("angleX").innerText = d.angleX;
    document.getElementById("tegakX").innerText = d.baselineTegakX;
    document.getElementById("jarakPitch").innerText = d.jarakPitch;
    document.getElementById("jarakRoll").innerText = d.jarakRoll;
    document.getElementById("tegak").innerText = d.baselineTegakY;
    document.getElementById("bongkok").innerText = d.baselineBongkokY;
    document.getElementById("jarakTegak").innerText = d.jarakKeTegak;
    document.getElementById("jarakBongkok").innerText = d.jarakKeBongkok;
    document.getElementById("bendTime").innerText = d.currentBendDuration;
    document.getElementById("vibration").innerText = d.vibrationStatus;
    document.getElementById("system").innerText = d.systemActive;

    let box = document.getElementById("statusBox");
    box.style.background = d.statusBg;
    box.style.color = d.statusColor;

  }catch(e){
    document.getElementById("net").innerText = "Koneksi lambat / mencoba ulang";
  }
}

setInterval(updateData, 3000);
updateData();
</script>

</body>
</html>
)rawliteral";

// =======================
// SETUP
// =======================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println();
  Serial.println("====================================");
  Serial.println(" SORE Smart Posture Reminder");
  Serial.println(" ESP32 + MPU6050 + Vibration Module");
  Serial.println(" Light WiFi Dashboard + Bluetooth");
  Serial.println("====================================");

  // Bangunkan MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Atur akselerometer ke +/-2 g agar pembagi 16384 LSB/g valid.
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(1000);

  // Cek MPU6050
  Wire.beginTransmission(MPU_ADDR);
  byte error = Wire.endTransmission();

  if (error == 0) {
    Serial.println("MPU6050 terdeteksi.");
  } else {
    Serial.println("MPU6050 TIDAK terdeteksi!");
    Serial.println("Cek wiring:");
    Serial.println("VCC -> 3V3 ESP32");
    Serial.println("GND -> GND ESP32");
    Serial.println("SDA -> GPIO21");
    Serial.println("SCL -> GPIO22");

    while (true) {
      digitalWrite(VIBRATION_PIN, LOW);
      delay(1000);
    }
  }

  // WiFi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);

  Serial.println();
  Serial.println("WiFi Dashboard aktif.");
  Serial.print("Nama WiFi: ");
  Serial.println(apSSID);
  Serial.print("Password: ");
  Serial.println(apPassword);
  Serial.print("IP Dashboard: ");
  Serial.println(WiFi.softAPIP());

  // Bluetooth
  SerialBT.begin("SORE-Bluetooth");
  Serial.println("Bluetooth aktif: SORE-Bluetooth");

  // Web Routes
  server.on("/", handleDashboard);
  server.on("/status", handleStatusJson);
  server.on("/train-tegak", handleTrainTegakHP);
  server.on("/train-bongkok", handleTrainBongkokHP);
  server.on("/start", handleSystemOn);
  server.on("/off", handleSystemOff);
  server.on("/reset-report", handleResetData);
  server.on("/reset-training", handleResetTraining);
  server.onNotFound(handleNotFound);

  server.begin();

  tampilkanMenuSerial();
  lastLoopTime = millis();
}

// =======================
// LOOP
// =======================
void loop() {
  server.handleClient();
  handleSerialCommand();
  handleBluetoothCommand();

  // Jika sensor tidak terbaca 3 kali berturut-turut, matikan motor secara aman.
  if (!readMPU6050()) {
    if (sensorReadFailures < 255) sensorReadFailures++;
    if (sensorReadFailures >= 3) {
      sensorOK = false;
      systemActive = false;
      resetStatusGerak();
      postureStatus = "SENSOR ERROR";
    }
    lastLoopTime = millis();
    delay(100);
    return;
  }
  sensorReadFailures = 0;
  if (!sensorOK && postureStatus == "SENSOR ERROR") {
    postureStatus = (!tegakSudahDilatih || !bongkokSudahDilatih)
                    ? "BELUM TRAINING" : "SISTEM OFF";
  }
  sensorOK = true;
  calculateAngle();

  jarakPitch = fabs(angleY - baselineTegakY);
  jarakRoll = fabs(angleX - baselineTegakX);
  jarakKeTegak = max(jarakPitch, jarakRoll);
  jarakKeBongkok = max(fabs(angleY - baselineBongkokY),
                      fabs(angleX - baselineBongkokX));

  unsigned long now = millis();
  unsigned long deltaTime = now - lastLoopTime;
  lastLoopTime = now;

  if (!systemActive) {
    digitalWrite(VIBRATION_PIN, LOW);
    vibrationStatus = "OFF";
    if (postureStatus != "SENSOR ERROR" &&
        postureStatus != "TEGAK TERSIMPAN" &&
        postureStatus != "BONGKOK TERSIMPAN") {
      postureStatus = (!tegakSudahDilatih || !bongkokSudahDilatih)
                      ? "BELUM TRAINING" : "SISTEM OFF";
    }
    delay(100);
    return;
  }

  // Deteksi dua sumbu: batas dilampaui bila salah satu deviasi > 15 derajat.
  bool posisiMenyimpang = jarakKeTegak > bendThreshold;
  bool posisiSudahTegak = jarakKeTegak <= uprightReleaseThreshold;

  if (vibrationLockedOn) {
    if (posisiSudahTegak) {
      resetStatusGerak();
      postureStatus = "TEGAK";
      Serial.println("Postur kembali tegak; getar OFF");
    } else {
      totalBadPostureTime += deltaTime;
      digitalWrite(VIBRATION_PIN, HIGH);
      vibrationStatus = "ON";
      postureStatus = "GETAR AKTIF";
    }
  } else if (posisiMenyimpang) {
    totalBadPostureTime += deltaTime;
    if (!isBending) {
      isBending = true;
      bendStartTime = now;
      currentBendDuration = 0;
      Serial.println("Postur menyimpang terdeteksi");
    }
    currentBendDuration = (now - bendStartTime) / 1000.0;
    if ((now - bendStartTime) >= bendDelay) {
      vibrationLockedOn = true;
      digitalWrite(VIBRATION_PIN, HIGH);
      vibrationStatus = "ON";
      postureStatus = "GETAR AKTIF";
      if (!warningAlreadyCounted) {
        dailyWarningCount++;
        warningAlreadyCounted = true;
      }
      Serial.println("Getar ON; akan OFF setelah deviasi <= 8 derajat");
    } else {
      digitalWrite(VIBRATION_PIN, LOW);
      vibrationStatus = "OFF";
      postureStatus = "POSTUR MENYIMPANG";
    }
  } else {
    // Jika belum 3 detik dan posisi pulih, timer diulang dari nol.
    if (isBending) Serial.println("Postur kembali normal sebelum notifikasi");
    resetStatusGerak();
    postureStatus = "TEGAK";
  }

  // Dashboard diperbarui setiap 3 detik; motor diproses ~100 ms sekali.
  delay(100);
}

// =======================
// SERIAL COMMAND
// =======================
void handleSerialCommand() {
  if (Serial.available()) {
    char input = Serial.read();

    if (input == 'T' || input == 't') {
      simpanPosisiTegak("Arduino IDE");
    } else if (input == 'B' || input == 'b') {
      simpanPosisiBongkok("Arduino IDE");
    } else if (input == 'S' || input == 's') {
      mulaiSistem("Arduino IDE");
    } else if (input == 'O' || input == 'o') {
      matikanSistem("Arduino IDE");
    } else if (input == 'R' || input == 'r') {
      resetTraining();
    } else if (input == 'L' || input == 'l') {
      resetLaporan();
    } else if (input == 'D' || input == 'd') {
      kirimStatusSerial();
    }
  }
}

// =======================
// BLUETOOTH COMMAND
// =======================
void handleBluetoothCommand() {
  if (SerialBT.available()) {
    char inputBT = SerialBT.read();

    if (inputBT == 'T' || inputBT == 't') {
      simpanPosisiTegak("Bluetooth");
      kirimStatusBluetooth();
    } else if (inputBT == 'B' || inputBT == 'b') {
      simpanPosisiBongkok("Bluetooth");
      kirimStatusBluetooth();
    } else if (inputBT == 'S' || inputBT == 's') {
      mulaiSistem("Bluetooth");
      kirimStatusBluetooth();
    } else if (inputBT == 'O' || inputBT == 'o') {
      matikanSistem("Bluetooth");
      kirimStatusBluetooth();
    } else if (inputBT == 'R' || inputBT == 'r') {
      resetTraining();
      kirimStatusBluetooth();
    } else if (inputBT == 'L' || inputBT == 'l') {
      resetLaporan();
      kirimStatusBluetooth();
    } else if (inputBT == 'D' || inputBT == 'd') {
      kirimStatusBluetooth();
    }
  }
}

// =======================
// SIMPAN POSISI TEGAK
// =======================
void simpanPosisiTegak(String sumber) {
  if (!readMPU6050()) {
    Serial.println("Gagal menyimpan tegak: MPU6050 tidak terbaca.");
    return;
  }
  calculateAngle();

  totalTegakX += angleX;
  totalTegak += angleY;
  jumlahTegak++;

  baselineTegakX = totalTegakX / jumlahTegak;
  baselineTegakY = totalTegak / jumlahTegak;
  tegakSudahDilatih = true;

  systemActive = false;
  resetStatusGerak();
  postureStatus = "TEGAK TERSIMPAN";

  Serial.println();
  Serial.print("Posisi TEGAK disimpan dari ");
  Serial.println(sumber);
  Serial.print("Sample tegak: ");
  Serial.println(jumlahTegak);
  Serial.print("Angle Y: ");
  Serial.println(angleY);
  Serial.print("Baseline Tegak Y: ");
  Serial.println(baselineTegakY);
}

// =======================
// SIMPAN POSISI BONGKOK
// =======================
void simpanPosisiBongkok(String sumber) {
  if (!readMPU6050()) {
    Serial.println("Gagal menyimpan bongkok: MPU6050 tidak terbaca.");
    return;
  }
  calculateAngle();

  totalBongkokX += angleX;
  totalBongkok += angleY;
  jumlahBongkok++;

  baselineBongkokX = totalBongkokX / jumlahBongkok;
  baselineBongkokY = totalBongkok / jumlahBongkok;
  bongkokSudahDilatih = true;

  systemActive = false;
  resetStatusGerak();
  postureStatus = "BONGKOK TERSIMPAN";

  Serial.println();
  Serial.print("Posisi BONGKOK disimpan dari ");
  Serial.println(sumber);
  Serial.print("Sample bongkok: ");
  Serial.println(jumlahBongkok);
  Serial.print("Angle Y: ");
  Serial.println(angleY);
  Serial.print("Baseline Bongkok Y: ");
  Serial.println(baselineBongkokY);
}

// =======================
// MULAI SISTEM
// =======================
void mulaiSistem(String sumber) {
  if (!sensorOK) {
    postureStatus = "SENSOR ERROR";
    digitalWrite(VIBRATION_PIN, LOW);
    return;
  }
  if (!tegakSudahDilatih || !bongkokSudahDilatih || jumlahTegak == 0 || jumlahBongkok == 0) {
    systemActive = false;
    postureStatus = "BELUM TRAINING";
    vibrationStatus = "OFF";
    digitalWrite(VIBRATION_PIN, LOW);

    Serial.println();
    Serial.print("Sistem belum bisa dimulai dari ");
    Serial.println(sumber);
    Serial.println("Simpan posisi TEGAK dan BONGKOK dulu.");
    return;
  }

  systemActive = true;
  postureStatus = "TEGAK";
  vibrationStatus = "OFF";

  resetStatusGerak();
  lastLoopTime = millis();

  Serial.println();
  Serial.print("Sistem dimulai dari ");
  Serial.println(sumber);
}

// =======================
// MATIKAN SISTEM
// =======================
void matikanSistem(String sumber) {
  systemActive = false;
  resetStatusGerak();
  postureStatus = "SISTEM OFF";

  Serial.println();
  Serial.print("Sistem dimatikan dari ");
  Serial.println(sumber);
}

// =======================
// RESET STATUS GERAK
// =======================
void resetStatusGerak() {
  isBending = false;
  warningAlreadyCounted = false;
  vibrationLockedOn = false;
  bendStartTime = 0;
  currentBendDuration = 0;

  digitalWrite(VIBRATION_PIN, LOW);
  vibrationStatus = "OFF";
}

// =======================
// RESET TRAINING
// =======================
void resetTraining() {
  totalTegakX = 0;
  baselineTegakX = 0;
  totalBongkokX = 0;
  baselineBongkokX = 0;
  totalTegak = 0;
  jumlahTegak = 0;
  baselineTegakY = 0;
  tegakSudahDilatih = false;

  totalBongkok = 0;
  jumlahBongkok = 0;
  baselineBongkokY = 0;
  bongkokSudahDilatih = false;

  systemActive = false;
  resetStatusGerak();
  postureStatus = "BELUM TRAINING";

  Serial.println();
  Serial.println("Training posisi TEGAK dan BONGKOK di-reset.");
  tampilkanMenuSerial();
}

// =======================
// RESET LAPORAN
// =======================
void resetLaporan() {
  dailyWarningCount = 0;
  totalBadPostureTime = 0;
  // Jangan reset warningAlreadyCounted saat motor masih menyala:
  // satu episode penyimpangan tidak boleh dihitung dua kali.
  if (!isBending) currentBendDuration = 0;

  Serial.println();
  Serial.println("Laporan hari ini di-reset.");
}

// =======================
// MENU SERIAL
// =======================
void tampilkanMenuSerial() {
  Serial.println();
  Serial.println("===== MENU SERIAL MONITOR =====");
  Serial.println("T = simpan posisi TEGAK");
  Serial.println("B = simpan posisi BONGKOK");
  Serial.println("S = mulai sistem");
  Serial.println("O = matikan sistem");
  Serial.println("R = reset training");
  Serial.println("L = reset laporan hari ini");
  Serial.println("D = tampilkan data");
  Serial.println("================================");
  Serial.println();
}

// =======================
// DASHBOARD HANDLER
// =======================
void handleDashboard() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html", DASHBOARD_HTML);
}

// =======================
// STATUS JSON
// =======================
void handleStatusJson() {
  server.sendHeader("Cache-Control", "no-store");

  float totalBadMinutes = totalBadPostureTime / 60000.0;

  String statusColor = "#22C55E";
  String statusBg = "#DCFCE7";
  String statusIcon = "OK";

  if (postureStatus == "BELUM TRAINING") {
    statusColor = "#B45309";
    statusBg = "#FEF3C7";
    statusIcon = "!";
  } else if (postureStatus == "TEGAK TERSIMPAN") {
    statusColor = "#0284C7";
    statusBg = "#E0F2FE";
    statusIcon = "T";
  } else if (postureStatus == "BONGKOK TERSIMPAN") {
    statusColor = "#7C3AED";
    statusBg = "#EDE9FE";
    statusIcon = "B";
  } else if (postureStatus == "POSTUR MENYIMPANG") {
    statusColor = "#EA580C";
    statusBg = "#FFEDD5";
    statusIcon = "...";
  } else if (postureStatus == "GETAR AKTIF") {
    statusColor = "#DC2626";
    statusBg = "#FEE2E2";
    statusIcon = "ON";
  } else if (postureStatus == "SISTEM OFF") {
    statusColor = "#475569";
    statusBg = "#E2E8F0";
    statusIcon = "OFF";
  } else if (postureStatus == "SENSOR ERROR") {
    statusColor = "#DC2626";
    statusBg = "#FEE2E2";
    statusIcon = "!";
  }

  String json = "{";
  json += "\"postureStatus\":\"" + postureStatus + "\",";
  json += "\"dailyWarningCount\":" + String(dailyWarningCount) + ",";
  json += "\"totalBadMinutes\":\"" + String(totalBadMinutes, 1) + "\",";
  json += "\"jumlahTegak\":" + String(jumlahTegak) + ",";
  json += "\"jumlahBongkok\":" + String(jumlahBongkok) + ",";
  json += "\"angleY\":\"" + String(angleY, 2) + "\",";
  json += "\"angleX\":\"" + String(angleX, 2) + "\",";
  json += "\"baselineTegakX\":\"" + String(baselineTegakX, 2) + "\",";
  json += "\"jarakPitch\":\"" + String(jarakPitch, 2) + "\",";
  json += "\"jarakRoll\":\"" + String(jarakRoll, 2) + "\",";
  json += "\"baselineTegakY\":\"" + String(baselineTegakY, 2) + "\",";
  json += "\"baselineBongkokY\":\"" + String(baselineBongkokY, 2) + "\",";
  json += "\"jarakKeTegak\":\"" + String(jarakKeTegak, 2) + "\",";
  json += "\"jarakKeBongkok\":\"" + String(jarakKeBongkok, 2) + "\",";
  json += "\"currentBendDuration\":\"" + String(currentBendDuration, 1) + "\",";
  json += "\"vibrationStatus\":\"" + vibrationStatus + "\",";
  json += "\"systemActive\":\"" + String(systemActive ? "ON" : "OFF") + "\",";
  json += "\"statusColor\":\"" + statusColor + "\",";
  json += "\"statusBg\":\"" + statusBg + "\",";
  json += "\"statusIcon\":\"" + statusIcon + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// =======================
// WEB HANDLERS
// =======================
void handleTrainTegakHP() {
  simpanPosisiTegak("HP Dashboard");
  server.send(200, "text/plain", "OK");
}

void handleTrainBongkokHP() {
  simpanPosisiBongkok("HP Dashboard");
  server.send(200, "text/plain", "OK");
}

void handleSystemOn() {
  mulaiSistem("HP Dashboard");
  server.send(200, "text/plain", "OK");
}

void handleSystemOff() {
  matikanSistem("HP Dashboard");
  server.send(200, "text/plain", "OK");
}

void handleResetData() {
  resetLaporan();
  server.send(200, "text/plain", "OK");
}

void handleResetTraining() {
  resetTraining();
  server.send(200, "text/plain", "OK");
}

void handleNotFound() {
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// =======================
// STATUS SERIAL
// =======================
void kirimStatusSerial() {
  Serial.println();
  Serial.println("===== STATUS SORE =====");
  Serial.print("Status Postur: ");
  Serial.println(postureStatus);
  Serial.print("Sudut Sekarang: ");
  Serial.println(angleY);
  Serial.print("Baseline Tegak: ");
  Serial.println(baselineTegakY);
  Serial.print("Baseline Bongkok: ");
  Serial.println(baselineBongkokY);
  Serial.print("Jarak Tegak: ");
  Serial.println(jarakKeTegak);
  Serial.print("Jarak Bongkok: ");
  Serial.println(jarakKeBongkok);
  Serial.print("Peringatan Sejak Reset: ");
  Serial.println(dailyWarningCount);
  Serial.print("Motor Getar: ");
  Serial.println(vibrationStatus);
  Serial.print("Sistem: ");
  Serial.println(systemActive ? "ON" : "OFF");
  Serial.println("=======================");
}

// =======================
// STATUS BLUETOOTH
// =======================
void kirimStatusBluetooth() {
  float totalBadMinutes = totalBadPostureTime / 60000.0;

  SerialBT.println();
  SerialBT.println("===== SORE Smart Posture Reminder =====");
  SerialBT.print("Status Postur: ");
  SerialBT.println(postureStatus);
  SerialBT.print("Sudut Sekarang: ");
  SerialBT.print(angleY, 2);
  SerialBT.println(" derajat");
  SerialBT.print("Baseline Tegak: ");
  SerialBT.print(baselineTegakY, 2);
  SerialBT.println(" derajat");
  SerialBT.print("Baseline Bongkok: ");
  SerialBT.print(baselineBongkokY, 2);
  SerialBT.println(" derajat");
  SerialBT.print("Jarak ke Tegak: ");
  SerialBT.print(jarakKeTegak, 2);
  SerialBT.println(" derajat");
  SerialBT.print("Jarak ke Bongkok: ");
  SerialBT.print(jarakKeBongkok, 2);
  SerialBT.println(" derajat");
  SerialBT.print("Durasi Bongkok: ");
  SerialBT.print(currentBendDuration, 1);
  SerialBT.println(" detik");
  SerialBT.print("Peringatan Sejak Reset: ");
  SerialBT.print(dailyWarningCount);
  SerialBT.println("x");
  SerialBT.print("Total Durasi Tidak Sesuai: ");
  SerialBT.print(totalBadMinutes, 1);
  SerialBT.println(" menit");
  SerialBT.print("Motor Getar: ");
  SerialBT.println(vibrationStatus);
  SerialBT.print("Sistem: ");
  SerialBT.println(systemActive ? "ON" : "OFF");
  SerialBT.println("---------------------------------------");
  SerialBT.println("T = Simpan Tegak");
  SerialBT.println("B = Simpan Bongkok");
  SerialBT.println("S = Mulai Sistem");
  SerialBT.println("O = Matikan Sistem");
  SerialBT.println("R = Reset Training");
  SerialBT.println("L = Reset Laporan");
  SerialBT.println("D = Tampilkan Data");
  SerialBT.println("=======================================");
}

// =======================
// HITUNG SUDUT
// =======================
void calculateAngle() {
  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  angleX = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / PI;
  angleY = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
}

// =======================
// BACA MPU6050
// =======================
bool readMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)14, (uint8_t)true) != 14) return false;

  if (Wire.available() >= 14) {
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();

    TempRaw = Wire.read() << 8 | Wire.read();

    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();
    return true;
  }
  return false;
}
