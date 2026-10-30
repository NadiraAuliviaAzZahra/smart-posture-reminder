# Dashboard web

Dashboard HTML, CSS, dan JavaScript saat ini **tertanam langsung di firmware** pada variabel `DASHBOARD_HTML` (PROGMEM), bukan sebuah aplikasi terpisah.

- Wi-Fi (Access Point ESP32): `SORE-Monitor`
- URL: `http://192.168.4.1`
- Endpoint status: `/status`
- Kendali lokal: `/train-tegak`, `/train-bongkok`, `/start`, `/off`, `/reset-report`, `/reset-training`

Pembaruan tampilan dilakukan dengan polling setiap 3 detik. Tidak perlu hosting eksternal untuk mengakses dashboard dari perangkat yang terhubung ke Access Point ESP32. **Ganti password AP sebelum penggunaan.**
