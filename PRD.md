# 📜 PRD: AmbaTrain

**Product Requirements Document**

| | |
|---|---|
| **Nama Produk** | AmbaTrain |
| **Tagline** | *"All the world's a station, and all the men and women merely passengers."* |
| **Jenis** | Web app pemesanan tiket kereta api (tugas kuliah) |
| **Versi** | 1.0 (Draft) |
| **Backend** | C++17/20 + Crow (REST API) |
| **Database** | Supabase (PostgreSQL terkelola) |
| **Frontend** | HTML/CSS/JS (+ Tailwind), tema dark gothic Shakespearean ala Final Fantasy |
| **Dokumen terkait** | `desain.md` (Design System) |

---

## 1. Ringkasan Produk

AmbaTrain adalah website pemesanan tiket kereta api. Pengguna dapat mencari jadwal, memilih kursi pada denah gerbong, membuat pesanan, membayar (simulasi), lalu menerima e-ticket. Secara visual, aplikasi dibungkus dengan tema **"A Fantasy Based on Reality"**: kota dan rute tetap nyata, tetapi pengalamannya terasa seperti memulai sebuah quest dalam dunia fantasi gelap.

Nilai jual teknis proyek ini adalah **backend C++ yang menangani konsistensi data pemesanan kursi** (anti double-booking) di atas PostgreSQL milik Supabase.

---

## 2. Latar Belakang & Masalah

**Konteks akademik:** Proyek ini dibuat untuk tugas kuliah sehingga harus memperlihatkan penerapan konsep rekayasa perangkat lunak (analisis kebutuhan, perancangan basis data, arsitektur berlapis, REST API, keamanan, pengujian).

**Masalah yang dipecahkan oleh produk:**

1. Pemesanan tiket manual lambat dan rawan kesalahan.
2. Dua pengguna dapat memesan kursi yang sama pada saat bersamaan jika sistem tidak menangani konkurensi.
3. Pengguna butuh visualisasi kursi yang jelas, bukan sekadar form.
4. Pengalaman pemesanan umumnya generik. AmbaTrain menawarkan identitas visual yang berkesan.

---

## 3. Tujuan & Non-Tujuan

### 3.1 Tujuan (Goals)

| ID | Tujuan | Ukuran Keberhasilan |
|---|---|---|
| G1 | Pengguna dapat menyelesaikan alur pemesanan end-to-end | Alur cari → pilih kursi → bayar → tiket berjalan tanpa error |
| G2 | Tidak ada double-booking | 0 kursi ganda pada uji konkurensi (min. 50 request paralel) |
| G3 | Backend dibangun dengan C++ dan arsitektur berlapis | Struktur controller / service / repository terpenuhi |
| G4 | Data tersimpan aman di Supabase | RLS aktif, kredensial tidak bocor, password ter-hash |
| G5 | Desain konsisten dengan tema | Seluruh halaman mengikuti `desain.md` |
| G6 | Siap didemokan & didokumentasikan | README, ERD, API docs, laporan lengkap |

### 3.2 Non-Tujuan (Out of Scope)

- Integrasi payment gateway asli (Midtrans, dsb.). Pembayaran hanya **simulasi**.
- Integrasi dengan sistem KAI sungguhan (data jadwal adalah data **seed/dummy**).
- Aplikasi mobile native.
- Fitur loyalitas, voucher, dan promosi kompleks.
- Skala produksi (ribuan pengguna konkuren). Target cukup untuk demo dan uji beban ringan.

---

## 4. Target Pengguna (Persona)

### 4.1 Penumpang ("Traveler")
- **Siapa:** Mahasiswa/pekerja yang ingin memesan tiket cepat dari ponsel atau laptop.
- **Kebutuhan:** Cari jadwal, lihat harga jelas, pilih kursi, dapat tiket.
- **Pain point:** Takut kursi diambil orang lain saat proses bayar; UI membingungkan.

### 4.2 Admin ("Conductor")
- **Siapa:** Pengelola data kereta.
- **Kebutuhan:** Kelola stasiun, kereta, gerbong, jadwal, harga; melihat daftar pesanan.
- **Pain point:** Input data berulang dan rawan salah.

### 4.3 Dosen/Penguji (stakeholder akademik)
- **Kebutuhan:** Mudah menjalankan proyek, melihat arsitektur & dokumentasi yang rapi, mengevaluasi fitur inti.

---

## 5. Lingkup Fitur & Prioritas

Prioritas: **P0** = wajib (MVP), **P1** = penting, **P2** = nilai tambah.

### 5.1 Autentikasi & Akun
| ID | Kebutuhan | Prioritas |
|---|---|---|
| FR-01 | Pengguna dapat registrasi (nama, email, password) | P0 |
| FR-02 | Pengguna dapat login dan menerima JWT | P0 |
| FR-03 | Password disimpan sebagai hash (tidak plaintext) | P0 |
| FR-04 | Logout (hapus token di sisi klien) | P0 |
| FR-05 | Role: `user` dan `admin` dengan otorisasi endpoint | P0 |
| FR-06 | Edit profil & ganti password | P2 |

### 5.2 Pencarian Jadwal
| ID | Kebutuhan | Prioritas |
|---|---|---|
| FR-10 | Melihat daftar stasiun (untuk dropdown asal/tujuan) | P0 |
| FR-11 | Mencari jadwal berdasarkan asal, tujuan, tanggal, jumlah penumpang | P0 |
| FR-12 | Hasil menampilkan nama kereta, kelas, jam berangkat/tiba, durasi, harga, sisa kursi | P0 |
| FR-13 | Filter & sort (kelas, jam, harga) | P1 |

### 5.3 Pemilihan Kursi
| ID | Kebutuhan | Prioritas |
|---|---|---|
| FR-20 | Menampilkan denah kursi per gerbong beserta statusnya (available / held / booked) | P0 |
| FR-21 | Pengguna dapat memilih kursi sejumlah penumpang | P0 |
| FR-22 | Kursi yang dipilih **ditahan (hold) selama 15 menit** | P0 |
| FR-23 | Kursi yang sudah dihold/dipesan orang lain tidak dapat dipilih | P0 |
| FR-24 | Hold otomatis dilepas bila waktu habis atau pesanan dibatalkan | P0 |

### 5.4 Pemesanan & Pembayaran
| ID | Kebutuhan | Prioritas |
|---|---|---|
| FR-30 | Mengisi data penumpang (nama, no. identitas) | P0 |
| FR-31 | Sistem menghasilkan **kode booking (Sigil)** unik | P0 |
| FR-32 | Pembayaran simulasi (pilih metode → konfirmasi → status `paid`) | P0 |
| FR-33 | Timer hitung mundur pembayaran terlihat di UI | P1 |
| FR-34 | Pembatalan pesanan oleh pengguna (sebelum/ sesudah bayar sesuai aturan) | P0 |
| FR-35 | Pesanan otomatis `expired` bila tidak dibayar dalam 15 menit | P0 |

### 5.5 E-Ticket & Riwayat
| ID | Kebutuhan | Prioritas |
|---|---|---|
| FR-40 | Menampilkan e-ticket (Royal Pass) setelah pembayaran | P0 |
| FR-41 | Riwayat pesanan pengguna (Kronik) dengan status | P0 |
| FR-42 | QR code pada e-ticket | P1 |
| FR-43 | Unduh e-ticket sebagai PDF | P2 |
| FR-44 | Notifikasi email | P2 |

### 5.6 Panel Admin (The Conductor's Chamber)
| ID | Kebutuhan | Prioritas |
|---|---|---|
| FR-50 | CRUD stasiun | P1 |
| FR-51 | CRUD kereta, gerbong, dan kursi | P1 |
| FR-52 | CRUD jadwal & harga | P1 |
| FR-53 | Melihat seluruh pesanan & statusnya | P1 |
| FR-54 | Dashboard statistik sederhana (jumlah pesanan, pendapatan simulasi) | P2 |

---

## 6. User Stories

**Penumpang**
- Sebagai penumpang, saya ingin mencari kereta dari kota A ke kota B pada tanggal tertentu agar saya tahu pilihan yang tersedia.
- Sebagai penumpang, saya ingin melihat denah kursi agar saya bisa memilih posisi yang saya suka.
- Sebagai penumpang, saya ingin kursi saya ditahan selama proses pembayaran agar tidak diambil orang lain.
- Sebagai penumpang, saya ingin menerima e-ticket dengan kode booking agar saya bisa naik kereta.
- Sebagai penumpang, saya ingin membatalkan pesanan jika rencana berubah.
- Sebagai penumpang, saya ingin melihat riwayat perjalanan saya.

**Admin**
- Sebagai admin, saya ingin menambah jadwal & mengatur harga agar data selalu mutakhir.
- Sebagai admin, saya ingin memantau pesanan yang masuk.

---

## 7. Alur Pengguna Utama

### 7.1 Alur Pemesanan (Happy Path)

```
Landing → Cari Rute → Hasil Jadwal → Pilih Kursi (HOLD 15 menit)
      → Isi Data Penumpang → Bayar Tribute (simulasi)
      → Royal Pass (e-ticket) → Kronik
```

### 7.2 Alur Pemesanan (Edge Cases)

| Skenario | Perilaku Sistem |
|---|---|
| Kursi diambil orang lain saat dipilih | API menolak (409 Conflict), UI memberi pesan & refresh denah |
| Timer 15 menit habis | Status `expired`, kursi dilepas, UI mengarahkan ke pencarian |
| Pengguna menutup tab saat hold | Hold tetap berjalan sampai kedaluwarsa, lalu dilepas otomatis |
| Pembayaran dikonfirmasi dua kali | Idempotent: request kedua tidak mengubah apa pun |
| Pembatalan setelah bayar | Status `cancelled`, kursi dilepas |

### 7.3 Diagram Status Pesanan

```
            ┌─────────┐  bayar   ┌──────┐
 booking →  │  held   │ ───────► │ paid │
            └────┬────┘          └──┬───┘
        timeout  │   batal          │ batal
                 ▼     ▼            ▼
            ┌─────────┐        ┌───────────┐
            │ expired │        │ cancelled │
            └─────────┘        └───────────┘
```

---

## 8. Kebutuhan Non-Fungsional

| Kategori | Kebutuhan |
|---|---|
| **Konsistensi data** | Semua pemesanan kursi dilakukan dalam **transaksi DB** dengan row locking; ditambah partial unique index sebagai pengaman terakhir |
| **Keamanan** | Hash password (bcrypt/argon2 via libsodium), JWT ber-expiry, prepared statement, validasi input, CORS terbatas, HTTPS saat deploy |
| **Kinerja** | Pencarian jadwal < 500 ms (dataset seed); pemesanan < 1 s |
| **Ketersediaan** | Cukup untuk demo; tidak ada SLA produksi |
| **Skalabilitas** | Stateless API (JWT) sehingga bisa direplikasi; DB lewat connection pooler |
| **Keterpeliharaan** | Arsitektur berlapis, konfigurasi via environment variable, logging |
| **Kegunaan** | Responsif (mobile/tablet/desktop), navigasi keyboard pada denah kursi |
| **Aksesibilitas** | Kontras minimal WCAG AA, status kursi tidak hanya dibedakan warna |
| **Portabilitas** | Dapat dijalankan via Docker Compose (app C++) + koneksi ke Supabase |

---

## 9. Arsitektur Sistem

```
┌──────────────┐   HTTPS/JSON    ┌───────────────────────┐   libpqxx (SSL)   ┌─────────────────────┐
│   Browser    │ ──────────────► │  C++ REST API (Crow)  │ ────────────────► │  Supabase Postgres  │
│  (Frontend)  │ ◄────────────── │  controllers          │ ◄──────────────── │  (via pooler/direct)│
└──────────────┘                 │  services             │                   │  + pg_cron          │
                                 │  repositories         │                   └─────────────────────┘
                                 │  middleware (JWT,CORS)│
                                 └───────────────────────┘
```

**Prinsip penting:** Browser **tidak pernah** berbicara langsung ke database. Semua akses data melewati API C++. Ini sekaligus menjadi poin utama bahwa backend-nya memang C++.

### 9.1 Struktur Proyek

```
AmbaTrain/
├── CMakeLists.txt
├── vcpkg.json
├── .env.example
├── src/
│   ├── main.cpp
│   ├── controllers/
│   ├── services/
│   ├── repositories/
│   ├── models/
│   ├── middleware/
│   └── utils/
├── db/
│   ├── schema.sql
│   ├── seed.sql
│   └── migrations/
├── frontend/
├── docs/
├── Dockerfile
└── docker-compose.yml
```

### 9.2 Dependensi Utama

| Kebutuhan | Library |
|---|---|
| Web framework | Crow (alt: Drogon) |
| PostgreSQL client | libpqxx |
| JSON | nlohmann/json |
| JWT | jwt-cpp |
| Hashing | libsodium |
| Testing | GoogleTest |
| Build | CMake + vcpkg |

---

## 10. Integrasi Supabase

Supabase dipakai sebagai **PostgreSQL terkelola** (hosting DB, dashboard, backup, dan ekstensi). Backend C++ terhubung langsung ke Postgres menggunakan connection string standar.

### 10.1 Pilihan Koneksi

Supabase menyediakan beberapa jenis koneksi. Untuk backend C++ yang berjalan terus-menerus (persistent):

| Mode | Port | Catatan untuk AmbaTrain |
|---|---|---|
| **Direct connection** | 5432 | Hanya IPv6 secara default; IPv4 butuh add-on berbayar. Cocok jika jaringan kamu mendukung IPv6 |
| **Session pooler** (shared) | 5432 | **Rekomendasi default.** Mendukung IPv4, mempertahankan session state & prepared statement. Username berformat `postgres.[PROJECT-REF]` |
| **Transaction pooler** | 6543 | **Hindari** untuk libpqxx: tidak mendukung prepared statement dan session state hilang antar transaksi |

> ✅ **Keputusan:** gunakan **Session pooler (port 5432)**, atau Direct connection jika jaringan IPv6 tersedia. Ambil connection string dari Supabase Dashboard → tombol **Connect**.
> ⚠️ Karena detail koneksi Supabase bisa berubah, selalu cek ulang string di dashboard.

### 10.2 Konfigurasi Environment

```env
# .env (JANGAN di-commit!)
DATABASE_URL=postgresql://postgres.[PROJECT-REF]:[PASSWORD]@aws-[N]-[REGION].pooler.supabase.com:5432/postgres?sslmode=require
JWT_SECRET=ganti-dengan-secret-panjang-acak
JWT_EXPIRES_IN=3600
HOLD_MINUTES=15
CORS_ORIGIN=http://localhost:5500
PORT=8080
```

- Selalu gunakan `sslmode=require`.
- `.env` masuk `.gitignore`; sediakan `.env.example` tanpa nilai rahasia.
- Password database & JWT secret **tidak boleh** muncul di frontend atau repo publik.

### 10.3 Keputusan Autentikasi

| Opsi | Penjelasan | Keputusan |
|---|---|---|
| **A. Auth sendiri di C++** (tabel `users` + JWT + libsodium) | Backend C++ yang mengelola registrasi/login | ✅ **Dipilih** |
| B. Supabase Auth | Login via layanan Supabase; backend memverifikasi JWT | Alternatif/cadangan |

**Alasan memilih A:** tujuan tugas adalah menunjukkan kemampuan backend C++, sehingga logika autentikasi sebaiknya dibuat sendiri. Opsi B tetap bisa dijadikan pengembangan lanjutan.

### 10.4 Row Level Security (RLS)

- Aktifkan RLS pada **semua tabel** di schema `public`, karena tabel di schema `public` dapat diakses lewat API otomatis Supabase (PostgREST) jika RLS tidak diaktifkan.
- Backend C++ terhubung memakai role `postgres` sehingga **tidak terpengaruh RLS**. Keamanan aplikasi ditegakkan oleh middleware JWT di C++.
- Tanpa policy untuk role `anon`/`authenticated`, data **tidak bisa diakses lewat Supabase REST API**. Ini disengaja agar satu-satunya pintu masuk adalah API C++.
- Jangan pernah mengekspos `service_role` key atau password DB ke browser.

### 10.5 Pembebasan Hold Otomatis

Dua lapis pengaman:

1. **Query-time:** setiap query ketersediaan kursi mengabaikan hold yang `expires_at < now()`.
2. **Background cleanup:** gunakan **`pg_cron`** (ekstensi Supabase) atau thread di C++ yang berjalan tiap menit untuk mengubah status booking kedaluwarsa menjadi `expired` dan melepas kursinya.

```sql
-- contoh job pg_cron (aktifkan ekstensi dulu di Dashboard)
select cron.schedule(
  'expire-held-bookings', '* * * * *',
  $$ select expire_held_bookings(); $$
);
```

### 10.6 Migrasi & Seed
- Simpan `schema.sql`, `seed.sql`, dan migrasi bertingkat di folder `db/`.
- Jalankan lewat **SQL Editor** Supabase atau `psql`.
- Data seed: ±10 stasiun nyata, ±6 kereta, gerbong & kursi, jadwal 14 hari ke depan.

---

## 11. Model Data

### 11.1 Entitas

| Tabel | Kolom Utama |
|---|---|
| `users` | id (uuid), name, email (unique), password_hash, role, created_at |
| `stations` | id, code (unique), name, city |
| `trains` | id, name, class (`sovereign` / `noble` / `yeoman`) |
| `carriages` | id, train_id, number, seat_count |
| `seats` | id, carriage_id, seat_number |
| `schedules` | id, train_id, origin_id, destination_id, departure_at, arrival_at, price |
| `bookings` | id, code (unique), user_id, schedule_id, status, total_price, expires_at, created_at |
| `booking_passengers` | id, booking_id, schedule_id, seat_id, name, id_number, **seat_active** |
| `payments` | id, booking_id, method, status, amount, paid_at |

### 11.2 Relasi
```
users 1─∞ bookings ∞─1 schedules ∞─1 trains 1─∞ carriages 1─∞ seats
bookings 1─∞ booking_passengers ∞─1 seats
bookings 1─1 payments
schedules ∞─1 stations (origin & destination)
```

### 11.3 Pencegah Double Booking

Satu kursi pada satu jadwal hanya boleh dimiliki satu pesanan **aktif**. Pesanan yang dibatalkan/kedaluwarsa tidak boleh memblokir kursi, sehingga unique index dibuat **parsial**:

```sql
-- seat_active = true selama booking berstatus held/paid
create unique index uq_active_seat_per_schedule
  on booking_passengers (schedule_id, seat_id)
  where seat_active;
```

Alur pemesanan di service layer (dalam satu transaksi):

```
BEGIN;
  1. SELECT kursi yang diminta ... FOR UPDATE;  -- kunci baris
  2. Pastikan tidak ada booking aktif untuk kursi tsb
  3. INSERT bookings (status='held', expires_at = now() + 15 menit)
  4. INSERT booking_passengers (seat_active = true)
COMMIT;
-- Jika unique index dilanggar → rollback → API balas 409 Conflict
```

### 11.4 Indeks yang Disarankan
- `schedules (origin_id, destination_id, departure_at)` untuk pencarian.
- `bookings (user_id, created_at desc)` untuk riwayat.
- `bookings (status, expires_at)` untuk job kedaluwarsa.
- `booking_passengers (schedule_id) where seat_active` untuk denah kursi.

---

## 12. Spesifikasi API (Ringkas)

Base URL: `/api` · Format: JSON · Auth: `Authorization: Bearer <JWT>`

| Method | Endpoint | Auth | Deskripsi |
|---|---|---|---|
| GET | `/health` | - | Cek kesehatan service & koneksi DB |
| POST | `/auth/register` | - | Registrasi |
| POST | `/auth/login` | - | Login, mengembalikan JWT |
| GET | `/auth/me` | User | Profil pengguna |
| GET | `/stations` | - | Daftar stasiun |
| GET | `/schedules?from=&to=&date=&pax=` | - | Cari jadwal |
| GET | `/schedules/:id/seats` | - | Denah & status kursi |
| POST | `/bookings` | User | Buat booking + hold kursi |
| GET | `/bookings` | User | Riwayat pesanan |
| GET | `/bookings/:code` | User | Detail pesanan / e-ticket |
| POST | `/bookings/:id/pay` | User | Bayar (simulasi) |
| POST | `/bookings/:id/cancel` | User | Batalkan pesanan |
| `CRUD` | `/admin/stations`, `/admin/trains`, `/admin/schedules` | Admin | Manajemen data |
| GET | `/admin/bookings` | Admin | Semua pesanan |

### Format Error Konsisten
```json
{
  "error": {
    "code": "SEAT_UNAVAILABLE",
    "message": "Kursi 3B telah direbut takdir lain.",
    "details": { "seat": "3B" }
  }
}
```

| HTTP | Kasus |
|---|---|
| 400 | Validasi gagal |
| 401 | Token tidak ada/invalid |
| 403 | Role tidak cukup |
| 404 | Data tidak ditemukan |
| 409 | Konflik (kursi sudah diambil) |
| 410 | Booking kedaluwarsa |
| 500 | Kesalahan server |

### Contoh: `POST /api/bookings`
```json
// Request
{
  "schedule_id": 12,
  "passengers": [
    { "name": "Ayu Lestari", "id_number": "7371xxxxxxxxxxxx", "seat_id": 204 },
    { "name": "Budi Santoso", "id_number": "7371xxxxxxxxxxxx", "seat_id": 205 }
  ]
}

// Response 201
{
  "booking": {
    "id": 88,
    "code": "AMB-7K3QX9",
    "status": "held",
    "total_price": 640000,
    "expires_at": "2026-11-11T15:15:00Z"
  }
}
```

---

## 13. Persyaratan UI/UX

Mengacu penuh pada **`desain.md`**. Ringkasan yang mengikat produk:

- Tema: dark gothic Shakespearean + sentuhan JRPG (palet void / gold / crimson / crystal).
- Istilah tematik (Gerbang, Sigil, Tribute, Royal Pass, Kronik) dipakai di UI, tetapi **informasi inti (harga, jam, kursi, error) harus tetap jelas**.
- Halaman wajib: Landing, Hasil Jadwal, Pilih Kursi, Data Penumpang & Pembayaran, Royal Pass, Kronik, Login/Register, 404/500.
- Halaman opsional: Panel Admin.
- Animasi berat dibatasi pada hero & momen sukses; hormati `prefers-reduced-motion`.

---

## 14. Keamanan & Privasi

| Risiko | Mitigasi |
|---|---|
| SQL injection | Prepared statement / parameterized query di seluruh repository |
| Password bocor | Hash dengan salt (argon2/bcrypt), tidak pernah dilog |
| Pencurian token | JWT berumur pendek, secret dari env, HTTPS |
| Akses data orang lain (IDOR) | Setiap query booking memfilter `user_id` dari token |
| Brute-force login | Rate limiting sederhana per IP/email |
| XSS | Escape output di frontend, jangan `innerHTML` data pengguna |
| CORS terbuka | Whitelist origin frontend |
| Kredensial Supabase bocor | `.env` di `.gitignore`, rotasi password jika terlanjur ter-commit |
| Akses DB dari luar | RLS aktif, tanpa policy publik; hanya API C++ yang dipakai |
| Data pribadi (no. identitas) | Minimalkan penyimpanan; tampilkan sebagian (masking) di UI |

---

## 15. Strategi Pengujian

| Level | Cakupan | Alat |
|---|---|---|
| Unit | Validator, hashing, perhitungan harga, logika status | GoogleTest |
| Integrasi | Repository ↔ DB Supabase (project **terpisah** untuk testing) | GoogleTest + DB test |
| API | Semua endpoint, termasuk kasus error | Postman / Thunder Client |
| **Konkurensi** | 50 request paralel memesan kursi sama → tepat 1 sukses | Script (k6 / bash + curl / Python) |
| Kedaluwarsa | Hold lewat 15 menit → kursi kembali tersedia | Uji manual + set `HOLD_MINUTES=1` |
| UI | Alur utama di mobile & desktop | Manual + checklist |

### Kriteria Penerimaan Utama
- [ ] Pengguna baru dapat register, login, dan memesan tiket sampai mendapat e-ticket.
- [ ] Uji konkurensi: dari N request ke kursi yang sama, tepat satu berhasil, sisanya menerima 409.
- [ ] Booking yang tidak dibayar menjadi `expired` dan kursinya bisa dipesan lagi.
- [ ] Pengguna tidak dapat melihat atau mengubah pesanan milik pengguna lain.
- [ ] Tidak ada kredensial di repositori.
- [ ] Seluruh halaman utama responsif dan sesuai tema.

---

## 16. Metrik Keberhasilan

| Metrik | Target |
|---|---|
| Alur utama selesai tanpa bug kritis | 100% skenario uji lulus |
| Double-booking pada uji konkurensi | 0 |
| Waktu respons pencarian jadwal | < 500 ms |
| Cakupan endpoint terdokumentasi | 100% |
| Kelengkapan deliverable tugas | Semua item Bagian 19 terpenuhi |

---

## 17. Risiko & Mitigasi

| Risiko | Dampak | Mitigasi |
|---|---|---|
| Kurva belajar C++ web (Crow, libpqxx, vcpkg) | Jadwal molor | Mulai dari endpoint `/health`, bangun bertahap; siapkan boilerplate |
| Gagal konek ke Supabase (IPv6/IPv4, SSL) | Backend tidak bisa akses DB | Gunakan Session pooler, `sslmode=require`, cek username `postgres.[REF]` |
| Prepared statement bermasalah di Transaction pooler | Error saat query | Gunakan port 5432 (session/direct), bukan 6543 |
| Project free tier Supabase di-pause bila lama tidak aktif | Demo gagal | Aktifkan project sebelum demo; siapkan `pg_dump` & DB lokal cadangan |
| Race condition kursi | Data tidak konsisten | Transaksi + `FOR UPDATE` + partial unique index + uji konkurensi |
| Scope terlalu besar | Tidak selesai | Kerjakan P0 dulu; P1/P2 hanya jika waktu cukup |
| Aset visual berhak cipta | Masalah lisensi | Buat sendiri / aset berlisensi bebas; tidak memakai aset resmi Final Fantasy |
| Rahasia ter-commit ke Git | Kebocoran data | `.gitignore`, `.env.example`, rotasi kredensial |

---

## 18. Rencana Rilis & Timeline (±6 Minggu)

| Minggu | Milestone | Output |
|---|---|---|
| **1** | Fondasi | PRD final, ERD, project Supabase dibuat, `schema.sql`, setup CMake+vcpkg, `/health` terkoneksi ke DB |
| **2** | Auth & master data | Register/login/JWT, middleware, CRUD stasiun & kereta, seed |
| **3** | Jadwal | Pencarian jadwal, endpoint denah kursi, frontend landing + hasil |
| **4** | Booking inti | Hold kursi dengan locking, expiry (pg_cron), uji konkurensi, UI pilih kursi |
| **5** | Pembayaran & tiket | Bayar simulasi, batal, riwayat, Royal Pass + QR, polish tema |
| **6** | Finalisasi | Panel admin (P1), testing menyeluruh, Docker Compose, laporan, slide demo |

---

## 19. Deliverable Tugas

- [ ] Source code + `README.md` (cara build & run, termasuk setup Supabase)
- [ ] `PRD.md` (dokumen ini) dan `desain.md`
- [ ] ERD, Use Case Diagram, Sequence Diagram (alur booking), Activity/State Diagram
- [ ] `schema.sql` + `seed.sql`
- [ ] Dokumentasi API (Postman collection / OpenAPI)
- [ ] Laporan hasil uji (termasuk uji konkurensi)
- [ ] Demo (live / video)
- [ ] Opsional: `Dockerfile` + `docker-compose.yml`

---

## 20. Pertanyaan Terbuka

1. Apakah dosen mewajibkan fitur tertentu (misalnya admin panel, QR, PDF)?
2. Apakah deploy publik diperlukan, atau demo lokal sudah cukup?
3. Jaringan/ISP kamu mendukung IPv6? (menentukan Direct vs Session pooler)
4. Frontend memakai HTML/JS biasa atau framework (React)?
5. Berapa batas jumlah penumpang per booking (usulan: maksimal 4)?
6. Kebijakan pembatalan setelah pembayaran (refund simulasi atau tidak)?

---

## 21. Glosarium

| Istilah | Arti |
|---|---|
| **Gerbang** | Stasiun |
| **Sigil** | Kode booking |
| **Tribute** | Pembayaran |
| **Royal Pass** | E-ticket |
| **Kronik** | Riwayat pesanan |
| **Hold** | Penahanan kursi sementara (15 menit) |
| **RLS** | Row Level Security (kontrol akses tingkat baris di PostgreSQL) |
| **Pooler** | Perantara koneksi DB yang mengelola banyak koneksi klien |

---

*AmbaTrain PRD v1.0: "Better three hours too soon than a minute too late." Karena itu, jangan telat ngerjain.* 🚂✦