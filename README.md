# SORE — Smart Posture Reminder

Proyek **SORE (Smart Posture Reminder)** dikembangkan untuk memantau perubahan kemiringan tubuh bagian atas dan memberikan peringatan getar berdasarkan sudut postur. Perangkat menggunakan **ESP32**, **MPU6050**, motor getar melalui driver, serta dashboard lokal yang dapat diakses melalui Wi-Fi dan kontrol Bluetooth Serial.

> **Status:** Kode firmware telah disusun dan diperiksa sintaks C++ secara statis. Kompilasi untuk board ESP32, pengujian langsung, kalibrasi menggunakan alat pembanding, dan validasi kinerja **belum dikonfirmasi**. Jangan menganggap angka target di bawah sebagai hasil eksperimen.

## Fitur firmware

- Menghitung estimasi sudut pitch (Y) dan roll (X) dari data **akselerometer** MPU6050.
- Menyimpan baseline posisi **tegak** dan **bongkok** melalui proses training manual.
- Mendeteksi postur menyimpang ketika **salah satu** deviasi pitch atau roll terhadap baseline tegak **lebih dari 15°** selama **3 detik terus-menerus**.
- Mengaktifkan motor getar setelah waktu tersebut, lalu mematikannya setelah deviasi kembali **≤ 8°** (histeresis).
- Menampilkan dashboard Wi-Fi dengan status, sudut, baseline, jumlah peringatan sejak reset, dan durasi postur menyimpang.
- Menyediakan kontrol via Bluetooth Serial dan Serial Monitor; mematikan motor saat pembacaan MPU6050 gagal berulang kali.

**Catatan:** Register giroskop juga dibaca tetapi *belum* digunakan untuk sensor fusion/filter komplementer. Perhitungan sudut saat ini berbasis akselerometer; ini perlu ditulis jujur pada slide metode.

## Komponen dan koneksi

| Perangkat | Koneksi |
|---|---|
| MPU6050 VCC | ESP32 3V3 (sesuai modul yang digunakan) |
| MPU6050 GND | ESP32 GND |
| MPU6050 SDA | ESP32 GPIO 21 |
| MPU6050 SCL | ESP32 GPIO 22 |
| Input driver motor getar | ESP32 GPIO 25 |
| Catu daya motor | Menyesuaikan driver dan spesifikasi motor |

**Jangan sambungkan motor getar DC secara langsung ke GPIO ESP32.** Gunakan modul driver/transistor dan proteksi yang sesuai. Periksa skema pada `hardware/README.md`.

## Kompilasi dan penggunaan

1. Gunakan **ESP32 klasik yang mendukung Bluetooth Classic/SPP**, misalnya varian ESP32-WROOM-32, bukan ESP32-C3/S3 yang tidak mendukung `BluetoothSerial` Classic SPP dengan cara yang sama.
2. Instal **Arduino IDE** dan board support **esp32 by Espressif Systems**.
3. Buka `firmware/SORE_Smart_Posture_Reminder/SORE_Smart_Posture_Reminder.ino`.
4. **Ganti** nilai `apPassword = "GANTI_PASSWORD_AP_2026"` dengan sandi Wi-Fi lokal minimal 8 karakter milik kelompok sebelum upload. Jangan mengunggah sandi pribadi yang digunakan pada jaringan lain.
5. Pilih board dan port ESP32 yang benar. Jalankan **Verify** lalu **Upload** di Arduino IDE.
6. Buka Serial Monitor pada baud **115200**. Hubungkan HP ke Wi-Fi **SORE-Monitor** menggunakan sandi yang sudah diganti.
7. Buka **http://192.168.4.1** di browser.
8. Pada posisi tegak, tekan **Simpan Tegak**; pada posisi condong yang dipilih, tekan **Simpan Bongkok**; kemudian tekan **Mulai Sistem**.
9. Uji sudut terhadap alat referensi dan dokumentasikan hasilnya; jangan memakai contoh visual atau grafik simulasi sebagai data aktual.

### Kontrol Serial / Bluetooth

| Perintah | Fungsi |
|---|---|
| `T` | Simpan baseline tegak |
| `B` | Simpan baseline bongkok |
| `S` | Mulai pemantauan |
| `O` | Matikan pemantauan |
| `R` | Reset training |
| `L` | Reset laporan sejak perangkat menyala |
| `D` | Tampilkan status |

Bluetooth Classic dipublikasikan dengan nama **SORE-Bluetooth**. Dashboard diperbarui sekitar tiap 3 detik; interval tampilan ini **bukan** waktu respons motor.

## Rencana kalibrasi dan evaluasi

- Referensi yang **direncanakan**: digital inclinometer.
- Titik pengukuran: punggung bagian atas/torakal; orientasi pemasangan harus dibuat konsisten.
- Sudut referensi: **0°, 10°, 20°, 30°, 40°**.
- Pengulangan: **5 kali per sudut, per sumbu** (total direncanakan 50 pengukuran untuk pitch dan roll).
- Metrik: galat absolut, MAE, RMSE, standar deviasi error, uji respons getar, dan uji stabilitas.
- Target rancangan: galat maksimum ≤ 2°, standar deviasi error < 0,5°, pengembalian ke nol ±1°, respons motor ≤ 500 ms setelah syarat 3 detik terpenuhi, dan operasi tanpa gangguan selama 8 jam.

Target tersebut **belum dibuktikan** oleh kode atau dokumentasi ini. Lihat `calibration/prosedur_kalibrasi.md` dan `analysis/README.md` untuk prosedur dan analisis.

## Struktur repositori

```text
smart-posture-reminder/
├── README.md
├── hardware/
├── firmware/
│   └── SORE_Smart_Posture_Reminder/
│       └── SORE_Smart_Posture_Reminder.ino
├── calibration/
├── data/
│   └── template_kalibrasi.csv
├── analysis/
│   └── analisis_kalibrasi.py
├── dashboard/
├── documentation/
└── manuscript/
```

## Progres dan batasan

**Tersedia dalam repositori:** kode firmware, rancangan dokumentasi hardware, prosedur pengujian, template data kosong, dan skrip analisis. **Belum tersedia:** bukti hasil kalibrasi, data pengujian aktual, verifikasi board, pengujian klinis, atau klaim pencegahan kelainan tulang belakang. Dokumentasi progres dan bukti pengujian perlu ditambahkan sesuai yang benar-benar dilakukan kelompok.

## Tim

- Muharrahmi
- Nadira Aulivia Az-Zahra
- Nahdia Shofi Rahayu

Pembimbing: Nurul Maulidiyah, S.Si., M.Sc.  
Mata kuliah: Perancangan Sistem Biomedis II.

## Publikasi yang direncanakan

Judul sementara: *Design and Performance Evaluation of an MPU6050–ESP32 Wearable Posture Reminder*.  
Target jurnal: [Electrician: Jurnal Rekayasa dan Teknologi Elektro](https://electrician.unila.ac.id/index.php/ojs/).  
Seluruh klaim hasil harus didasarkan pada pengujian yang benar-benar telah dilaksanakan.
