# LearningEsp-Now
Diketahui tiga buah ESP terkoneksi melalui protokol ESP-NOW (lihat gambar terlampir). Angka yang ada dibawah setiap ESP adalah MAC addressnya. 

Di ESP-1 terdapat LM35 dan Heater, sedangkan di ESP-2 terdapat LDR dan LED. Buat program di ESP-0, ESP-1 dan ESP-2 untuk mendukung cara kerja berikut. Setiap 5 detik, ESP-0 membaca nilai LM35 (dari ESP-1) dan membaca nilai LDR (dari ESP-2). Berdasarkan data LM35 yang dibaca oleh ESP-0, 

Jika LM35&lt;40.0 maka ESP-0 mengirim data ke ESP-1 supaya Heater1 hidup 100% atau datanya 255, jika LM35>60.0 maka ESP-0 mengirim data ke ESP-1 supaya Heater1 mati 100% atau datanya 0, jika LM35 antara 40 dan 60 maka data untuk Heater1 adalah proporsional antara 255-0, contohnya ketika LM35=50 maka data=127 (lihat gambar di lampiran). 

Berdasarkan data LDR yang dibaca ESP-0, jika LDR&lt;100 maka ESP-0 mengirim data ke ESP-2 supaya LED on, jika LDR>800 maka ESP-0 mengirim data ke ESP-2 supaya LED off. Jadi yang melakukan pembandingan data LDR dengan 100 atau 800 bukan program di ESP-2 tapi di ESP-0, ESP-2 hanya menerima perintah on/off dari ESP-0.

![Topologi](https://lh3.googleusercontent.com/5VGlLC7PIXVjxJNTY-zBuXnpMAlIkXTAtx-Fgv63ca7QwNPKFTgELjIjQIN8Fqt5a2jT-wWln78IyOY=w1920-h961g)
![Topologi](https://drive.google.com/file/d/1pPtX7PnNtYONymdXjKJUYEd4LqxJ6TjG/view?usp=drive_web&authuser=0)
