# Analisis kalibrasi

Gunakan Python 3 standar (tidak membutuhkan paket tambahan).

1. Isi `data/template_kalibrasi.csv` dengan data pengukuran **aktual** atau simpan sebagai file baru, misalnya `data/hasil_kalibrasi.csv`.
2. Jalankan dari folder utama repositori:

```bash
python analysis/analisis_kalibrasi.py data/hasil_kalibrasi.csv
```

Skrip menghitung MAE, RMSE, standar deviasi sampel error, serta galat absolut maksimum untuk masing-masing sumbu. Skrip menolak CSV tanpa pengukuran dan tidak mengisi data palsu. Hasil muncul di terminal; grafik dapat dibuat setelah data nyata tersedia.

Pengujian respons motor dan stabilitas delapan jam memerlukan dokumentasi hasil terpisah.
