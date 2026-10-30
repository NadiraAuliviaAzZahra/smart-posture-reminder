# Firmware ESP32

Buka kode utama di:

`SORE_Smart_Posture_Reminder/SORE_Smart_Posture_Reminder.ino`

Firmware Arduino menggunakan `Wire.h`, `WiFi.h`, `WebServer.h`, dan `BluetoothSerial.h` (bagian dari ekosistem Arduino ESP32).

**Board:** ESP32 dengan Bluetooth Classic/SPP.  
**Baud Serial:** 115200.  
**SDA / SCL:** GPIO 21 / GPIO 22.  
**Driver getar:** GPIO 25.  
**Akses dashboard:** Wi-Fi `SORE-Monitor` dan alamat `http://192.168.4.1`.

**Sebelum upload:** ganti password Access Point pada file `.ino`, periksa wiring, pilih board yang sesuai, lalu lakukan Verify/Upload di Arduino IDE.

Perubahan dibanding kode awal:
- Deviasi postur menggunakan pitch dan roll, tidak hanya satu sumbu.
- Motor dinyalakan setelah >15° selama 3 detik, mati setelah deviasi kembali ≤8°.
- Durasi dan peringatan dilaporkan sejak reset, bukan otomatis harian.
- Kegagalan pembacaan sensor berulang mematikan motor dan pemantauan.
- Dashboard menampilkan nilai dua sumbu.

**Belum diverifikasi:** kompilasi Arduino sebenarnya, stabilitas, akurasi, maupun ketepatan waktu motor pada perangkat fisik.
