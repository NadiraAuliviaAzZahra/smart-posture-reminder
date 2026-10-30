# Prosedur Kalibrasi dan Pengujian SORE (Rencana)

## Persiapan

1. Periksa pemasangan sensor pada punggung bagian atas dan keselarasan sumbu sensor dengan digital inclinometer.
2. Catat tipe ESP32, modul MPU6050, motor/driver, kondisi baterai atau catu daya, serta perangkat lunak yang dipakai.
3. Catat identitas alat pembanding dan status kalibrasinya. Jika belum terkalibrasi, tulis keterbatasan tersebut.
4. Aktifkan alat dan simpan baseline posisi tegak melalui dashboard/Serial.

## Pengukuran statis

1. Uji setiap sumbu pitch dan roll secara terpisah.
2. Gunakan sudut referensi **0°, 10°, 20°, 30°, 40°**.
3. Pada setiap sudut, tunggu pembacaan stabil, lalu catat nilai inclinometer dan pembacaan MPU6050.
4. Lakukan **5 pengulangan** pada setiap sudut di setiap sumbu sehingga rencana jumlah data **2 sumbu × 5 sudut × 5 pengulangan = 50 baris**.
5. Masukkan hasil nyata ke CSV dengan header seperti `data/template_kalibrasi.csv`. Jangan membuat data contoh yang disajikan sebagai hasil.
6. Analisis menggunakan `analysis/analisis_kalibrasi.py`.

## Uji notifikasi

- Latih baseline tegak dan baseline bongkok.
- Uji deviasi mendekati ambang (misalnya 14°, 16°) dan durasi kurang/lebih dari 3 detik.
- Catat waktu nyata mulai kondisi pemicu dan waktu nyata motor mulai bergetar; hitung selisih setelah durasi 3 detik terpenuhi untuk evaluasi target **≤ 500 ms**.
- Uji peringatan tetap aktif sampai deviasi kembali **≤8°**.
- Ulangi pengujian pada perubahan sudut pitch dan roll secara terpisah.

## Uji stabilitas

Jalankan sistem selama 8 jam pada kondisi uji yang terdokumentasi. Catat restart, hang, terputusnya koneksi, kehilangan pembacaan, dan kondisi sumber daya.

## Target (bukan hasil)

- Galat sudut maksimum: ≤2°.
- Standar deviasi error: <0,5°.
- Pengembalian nol: dalam rentang ±1°.
- Respons aktivasi motor: ≤500 ms setelah kondisi >15° berlangsung 3 detik.
- Stabilitas: 8 jam tanpa restart/hang atau kehilangan data pada kondisi uji yang didefinisikan.

**Catatan:** Estimasi sudut pada firmware saat ini berasal dari akselerometer, bukan fusion giroskop–akselerometer. Dokumentasikan keterbatasan dan hasil sebenarnya.
