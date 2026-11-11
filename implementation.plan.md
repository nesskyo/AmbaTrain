# 🚂 AmbaTrain — Comprehensive Implementation Plan

> **Product:** AmbaTrain — Web Pemesanan Tiket Kereta Api Gothic Shakespearean  
> **Tagline:** *"All the world's a station, and all the men and women merely passengers."*  
> **Tech Stack:** C++17/20 (Crow Framework) + Supabase (PostgreSQL 15+) + Tailwind CSS / Vanilla JS  
> **Dokumen Terkait:** [PRD.md](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/PRD.md), [desain.md](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/desain.md), [query.md](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/query.md)

---

## 📑 Daftar Isi
1. [Ringkasan Eksekutif & Sasaran Teknis](#1-ringkasan-eksekutif--sasaran-teknis)
2. [Arsitektur Sistem & Direktori Proyek](#2-arsitektur-sistem--direktori-proyek)
3. [Spesifikasi Dependensi & Build System](#3-spesifikasi-dependensi--build-system)
4. [Rencana Fase Kerja (6 Minggu / 6 Fase)](#4-rencana-fase-kerja-6-minggu--6-fase)
   - [Fase 1: Fondasi Basis Data & Boilerplate C++](#fase-1-fondasi-basis-data--boilerplate-c)
   - [Fase 2: Autentikasi & Manajemen Data Master](#fase-2-autentikasi--manajemen-data-master)
   - [Fase 3: Pencarian Jadwal & Visualisasi Denah Kursi](#fase-3-pencarian-jadwal--visualisasi-denah-kursi)
   - [Fase 4: Mesin Pemesanan Inti & Anti Double-Booking](#fase-4-mesin-pemesanan-inti--anti-double-booking)
   - [Fase 5: Pembayaran Simulasi, Royal Pass & Kronik](#fase-5-pembayaran-simulasi-royal-pass--kronik)
   - [Fase 6: Panel Admin, Pengujian Konkurensi & Finalisasi](#fase-6-panel-admin-pengujian-konkurensi--finalisasi)
5. [Strategi Anti Double-Booking & Desain Transaksi](#5-strategi-anti-double-booking--desain-transaksi)
6. [Format Kontrak API & Error Envelope](#6-format-kontrak-api--error-envelope)
7. [Rencana Pengujian & Script Uji Konkurensi](#7-rencana-pengujian--script-uji-konkurensi)
8. [Matriks Deliverable Akademik](#8-matriks-deliverable-akademik)

---

## 1. Ringkasan Eksekutif & Sasaran Teknis

AmbaTrain adalah aplikasi web pemesanan tiket kereta api bertema **"A Fantasy Based on Reality"** (dark gothic Shakespearean ala Final Fantasy) dengan data stasiun dan rute nyata di Indonesia. Nilai jual utama teknis proyek ini adalah **backend berbasis C++ murni yang menggaransi konsistensi data pemesanan kursi (anti double-booking)** di atas basis data PostgreSQL terkelola Supabase.

### Metrik Keberhasilan Kunci (Success Metrics):
| ID | Metrik | Target | Metode Pengujian |
|---|---|---|---|
| M1 | End-to-end booking flow | 100% lulus tanpa hambatan | Uji manual & integrasi API |
| M2 | Double booking pada beban paralel | **0 kursi ganda** | 50 request konkuren paralel memesan kursi sama |
| M3 | Latensi pencarian jadwal | < 500 ms (dataset seed) | Benchmark k6 / ab |
| M4 | Arsitektur berlapis backend | Strict Separation of Concerns | Controller -> Service -> Repository |
| M5 | Keamanan data Supabase | RLS aktif, credentials terisolasi | Audit PostgREST direct access (harus 401/404) |

---

## 2. Arsitektur Sistem & Direktori Proyek

### 2.1 Diagram Arsitektur Berlapis (Layered Architecture)

```
┌─────────────────────────────────────────────────────────────┐
│                       Browser (Frontend)                    │
│    HTML5 + Tailwind CSS (Void, Obsidian, Crimson, Gold)     │
└──────────────────────────────┬──────────────────────────────┘
                               │ HTTPS / REST JSON + Bearer JWT
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                 AmbaTrain Backend (C++ Crow)                │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ Middleware: CORSHandler, AuthMiddleware (JWT), Logger │  │
│  └───────────────────────────┬───────────────────────────┘  │
│                              ▼                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ Controllers: Auth, Stations, Schedules, Bookings, ... │  │
│  └───────────────────────────┬───────────────────────────┘  │
│                              ▼                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ Services: BookingEngine, Hashing, ExpiryWorker, ...   │  │
│  └───────────────────────────┬───────────────────────────┘  │
│                              ▼                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ Repositories: Prepared Statements via libpqxx         │  │
│  └───────────────────────────┬───────────────────────────┘  │
│                              ▼                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ DatabasePool: Thread-safe libpqxx Connection Pool     │  │
│  └───────────────────────────┬───────────────────────────┘  │
└──────────────────────────────┼──────────────────────────────┘
                               │ SSL Session Pooler (Port 5432)
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                 Supabase (PostgreSQL 15+)                   │
│   • Partial Unique Index (uq_active_seat_per_schedule)      │
│   • Triggers: Auto updated_at & Auto-release expired seats  │
│   • RLS Enabled: PostgREST API Blocked for Public           │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Struktur File Proyek

```
AmbaTrain/
├── CMakeLists.txt                # Root CMake build script
├── vcpkg.json                    # vcpkg manifest file
├── .env.example                  # Template variabel lingkungan
├── .gitignore                    # Mengabaikan build/, .env, vcpkg_installed/
├── Dockerfile                    # Multi-stage Docker build
├── docker-compose.yml            # Deployment container
├── PRD.md                        # Product Requirements Document
├── desain.md                     # Design System Document
├── query.md                      # Database queries & Supabase guide
├── implementation-plan.md        # Dokumen master rencana implementasi
├── db/
│   ├── schema.sql                # DDL Schema, Index, Triggers & RLS
│   ├── seed.sql                  # Data awal stasiun, kereta, jadwal 14 hari
│   └── migrations/               # Skrip migrasi berkala
├── src/
│   ├── main.cpp                  # Entry point Crow server
│   ├── config/
│   │   ├── Config.hpp            # Parser environment variables
│   │   └── DatabasePool.hpp      # Connection pooler libpqxx
│   ├── models/                   # Data Structs & nlohmann JSON serializers
│   │   ├── User.hpp
│   │   ├── Station.hpp
│   │   ├── Train.hpp
│   │   ├── Schedule.hpp
│   │   ├── Booking.hpp
│   │   └── Payment.hpp
│   ├── repositories/             # Layer interaksi langsung ke database
│   │   ├── UserRepository.hpp
│   │   ├── StationRepository.hpp
│   │   ├── ScheduleRepository.hpp
│   │   ├── SeatRepository.hpp
│   │   ├── BookingRepository.hpp
│   │   └── PaymentRepository.hpp
│   ├── services/                 # Layer logika bisnis & transaksi
│   │   ├── AuthService.hpp
│   │   ├── ScheduleService.hpp
│   │   ├── BookingService.hpp    # Engine locking & anti double-booking
│   │   └── ExpiryWorker.hpp      # Thread pembebasan kursi kedaluwarsa
│   ├── controllers/              # Layer router & HTTP handler
│   │   ├── AuthController.hpp
│   │   ├── StationController.hpp
│   │   ├── ScheduleController.hpp
│   │   ├── BookingController.hpp
│   │   ├── PaymentController.hpp
│   │   └── AdminController.hpp
│   ├── middleware/               # Middleware layer
│   │   ├── CorsMiddleware.hpp
│   │   └── AuthGuard.hpp
│   └── utils/                    # Helper & utilitas
│       ├── CryptoUtils.hpp       # Password hashing (Argon2 / libsodium)
│       ├── JwtUtils.hpp          # JWT signer & parser
│       └── ResponseUtils.hpp     # Standard JSON envelope formatter
├── frontend/                     # UI Web Gothic Shakespearean
│   ├── index.html                # Landing page & pencarian rute
│   ├── schedules.html            # Hasil jadwal & filter kelas kereta
│   ├── seats.html                # Denah kursi kristal interaktif
│   ├── checkout.html             # Input data penumpang & timer tribute
│   ├── ticket.html               # Royal Pass (e-ticket) & QR Code
│   ├── chronicle.html            # Riwayat pesanan buku sejarah
│   ├── auth.html                 # Login & Register (Enter the Realm)
│   ├── admin.html                # The Conductor's Chamber
│   ├── css/
│   │   ├── style.css             # Desain gothic & custom variables
│   │   └── tailwind.css
│   └── js/
│       ├── api.js                # Wrapper fetch HTTP client & JWT storage
│       ├── app.js
│       └── seatmap.js            # Interaktivitas denah gerbong
├── scripts/                      # Script pengujian & utility
│   ├── test_concurrency.py       # Script uji 50 request paralel
│   └── seed_supabase.sh          # Runner psql untuk setup otomatis
└── tests/                        # GoogleTest unit & integration tests
    ├── AuthTest.cpp
    ├── BookingConcurrencyTest.cpp
    └── ScheduleTest.cpp
```

---

## 3. Spesifikasi Dependensi & Build System

### 3.1 `vcpkg.json` (Package Manifest)
```json
{
  "name": "ambatrain",
  "version-string": "1.0.0",
  "dependencies": [
    "crow",
    "libpqxx",
    "nlohmann-json",
    "jwt-cpp",
    "libsodium",
    "gtest"
  ]
}
```

### 3.2 `CMakeLists.txt` (Build Configuration)
```cmake
cmake_minimum_required(VERSION 3.20)
project(AmbaTrain CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(libsodium CONFIG REQUIRED)

include_directories(src)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(ambatrain ${SOURCES})

target_link_libraries(ambatrain PRIVATE
    Crow::Crow
    libpqxx::pqxx
    nlohmann_json::nlohmann_json
    jwt-cpp::jwt-cpp
    libsodium::libsodium
)

# Enable Testing
enable_testing()
find_package(GTest CONFIG REQUIRED)
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(ambatrain_tests ${TEST_SOURCES})
    target_link_libraries(ambatrain_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        libpqxx::pqxx
        nlohmann_json::nlohmann_json
        jwt-cpp::jwt-cpp
        libsodium::libsodium
    )
    add_test(NAME AllTests COMMAND ambatrain_tests)
endif()
```

### 3.3 Variabel Lingkungan (`.env.example`)
```env
# Koneksi Database Supabase (Wajib Session Pooler Port 5432)
DATABASE_URL=postgresql://postgres.[PROJECT-REF]:[PASSWORD]@aws-0-[REGION].pooler.supabase.com:5432/postgres?sslmode=require

# Konfigurasi Server
PORT=8080
SERVER_HOST=0.0.0.0
CORS_ORIGIN=http://localhost:5500

# Keamanan JWT
JWT_SECRET=super-secret-gothic-sigil-key-change-this-in-production-123456
JWT_EXPIRES_IN=86400

# Aturan Bisnis
HOLD_MINUTES=15
```

---

## 4. Rencana Fase Kerja (6 Minggu / 6 Fase)

### Fase 1: Fondasi Basis Data & Boilerplate C++ (Minggu 1) [STATUS: SELESAI ✅]
- [x] **DB Setup:** Berkas [`db/schema.sql`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/db/schema.sql) siap dieksekusi di Supabase SQL Editor.
- [x] **Seed Data:** Berkas [`db/seed.sql`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/db/seed.sql) siap dieksekusi untuk mengisi 10 stasiun, 6 kereta 3 kelas, dan jadwal dinamis 14 hari.
- [x] **Config Parser:** Terbuat di [`src/config/Config.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/config/Config.hpp) untuk mem-parsing environment variables dari `.env`.
- [x] **Database Connection Pool:** Terbuat di [`src/config/DatabasePool.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/config/DatabasePool.hpp) membungkus koneksi `pqxx::connection` secara thread-safe menggunakan antrean dan scoped RAII guard.
- [x] **Health & Master Endpoints:** Terbuat di [`src/main.cpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/main.cpp) mengimplementasikan `GET /api/health` (dengan pengukuran latensi round-trip DB dan status pool) serta `GET /api/stations`.
- [x] **CORS & Response Utility:** Terbuat di [`src/middleware/CorsMiddleware.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/middleware/CorsMiddleware.hpp) dan [`src/utils/ResponseUtils.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/utils/ResponseUtils.hpp) untuk format JSON envelope standar PRD.
- [x] **Data Models:** Terbuat di `src/models/` ([`Station.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Station.hpp), [`Train.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Train.hpp), [`User.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/User.hpp), [`Schedule.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Schedule.hpp), [`Booking.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Booking.hpp), [`Payment.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Payment.hpp)).

### Fase 2: Autentikasi & Manajemen Data Master (Minggu 2) [STATUS: SELESAI ✅]
- [x] **Hashing Engine ([`CryptoUtils.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/utils/CryptoUtils.hpp)):** Mendukung `libsodium` (Argon2id) dan portable SHA-256 salted hash fallback.
- [x] **JWT Provider ([`JwtUtils.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/utils/JwtUtils.hpp)):** Utilitas penandatanganan dan verifikasi HS256 dengan claims `user_id`, `role`, `email`, dan `exp`.
- [x] **Auth Guard & Middleware ([`AuthGuard.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/middleware/AuthGuard.hpp)):** Ekstraksi header `Bearer <token>`, validasi claims, dan otorisasi role (`user` vs `admin`).
- [x] **Repository Layer:** Terbuat [`UserRepository.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/repositories/UserRepository.hpp) & [`StationRepository.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/repositories/StationRepository.hpp).
- [x] **Service Layer:** Terbuat [`AuthService.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/services/AuthService.hpp) menangani logika bisnis registrasi, login, dan profil.
- [x] **Controller Layer & Endpoints:** Terbuat [`AuthController.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/controllers/AuthController.hpp) & [`StationController.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/controllers/StationController.hpp):
  - `POST /api/auth/register`: Validasi format email, cek duplikasi, hash password, kembalikan JWT.
  - `POST /api/auth/login`: Verifikasi password hash, kembalikan JWT.
  - `GET /api/auth/me`: Endpoint terproteksi mengembalikan profil pengelana yang sedang aktif.
  - `GET /api/stations`: Daftar gerbang stasiun terurut.
- [x] **Integration:** Seluruh route terkoneksi di [`src/main.cpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/main.cpp) dan script uji otomatis [`scripts/test_auth.py`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/scripts/test_auth.py).

### Fase 3: Pencarian Jadwal & Visualisasi Denah Kursi (Minggu 3) [STATUS: SELESAI ✅]
- [x] **Schedule Search API ([`ScheduleRepository.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/repositories/ScheduleRepository.hpp) & [`ScheduleController.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/controllers/ScheduleController.hpp)):**
  - Parameter query: `from`, `to`, `date`, `pax`.
  - Agregasi data kereta (Sovereign, Noble, Yeoman), jam berangkat, jam tiba, durasi, harga.
  - Kalkulasi sisa kursi real-time (mengurangkan total kursi dengan kursi aktif pada booking `paid` dan `held` belum kedaluwarsa).
- [x] **Seat Layout API ([`SeatRepository.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/repositories/SeatRepository.hpp)):**
  - `GET /api/schedules/:id/seats`: Mengelompokkan seluruh gerbong dan kursi dengan status dinamis (`available`, `held` 15 menit, `booked`).
- [x] **Service Layer Validation ([`ScheduleService.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/services/ScheduleService.hpp)):**
  - Validasi jarak rute (`from != to`), format tanggal ISO8601, batas maksimal rombongan (`1 <= pax <= 4`).
- [x] **Frontend Interaktif Bergaya Gothic JRPG:**
  - [`frontend/index.html`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/frontend/index.html): Hero stasiun gothic, form pencarian rute, kartu hasil jadwal responsif.
  - [`frontend/seats.html`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/frontend/seats.html): Signature interactive carriage blueprint dengan kursi kristal bercahaya, lorong gerbong, tab gerbong, dan kalkulator harga party.
- [x] **Automated Testing:** Skrip pengujian otomatis di [`scripts/test_schedules_seats.py`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/scripts/test_schedules_seats.py).

### Fase 4: Mesin Pemesanan Inti & Anti Double-Booking (Minggu 4)
- [ ] **Booking Engine Transaction (`POST /api/bookings`):**
  - Menerima payload: `schedule_id` dan array `passengers` (`name`, `id_number`, `seat_id`).
  - Membuka transaksi database terisolasi: `BEGIN;`
  - Mengeksekusi `SELECT id FROM seats WHERE id IN (...) FOR UPDATE;` untuk mengunci baris kursi dari balapan transaksi konkuren.
  - Memvalidasi apakah kursi masih kosong pada jadwal tersebut. Jika sudah diambil, lakukan `ROLLBACK;` dan kembalikan status `409 Conflict`.
  - Membuat baris baru pada tabel `bookings` dengan `status = 'held'`, kode unik Sigil (`AMB-XXXXXX`), dan `expires_at = NOW() + INTERVAL '15 minutes'`.
  - Memasukkan seluruh data penumpang ke `booking_passengers` dengan `seat_active = TRUE`.
  - Menangani error jika Partial Unique Index `uq_active_seat_per_schedule` dilanggar secara atomik.
  - `COMMIT;` dan mengembalikan respons `201 Created` beserta Sigil dan timer.
- [ ] **Background Expiry Worker (`ExpiryWorker.hpp`):**
  - Menjalankan background thread `std::thread` di C++ yang setiap 60 detik memanggil `SELECT public.expire_held_bookings();`.
  - Mengubah booking kedaluwarsa menjadi `expired`, memicu trigger database yang otomatis melepaskan kursi (`seat_active = FALSE`).

### Fase 5: Pembayaran Simulasi, Royal Pass & Kronik (Minggu 5)
- [ ] **Payment API (`POST /api/bookings/:id/pay`):**
  - Validasi bahwa pemesanan masih berstatus `held` dan `expires_at > NOW()`.
  - Memperbarui status booking menjadi `paid` (Sealed).
  - Menyimpan data transaksi ke tabel `payments` dengan metode yang dipilih (`tribute_crystal`, `qris`, `bank_transfer`).
- [ ] **Cancellation API (`POST /api/bookings/:id/cancel`):**
  - Mengubah status booking menjadi `cancelled` (Forsaken).
  - Database trigger otomatis membebaskan kursi seketika.
- [ ] **Royal Pass E-Ticket API (`GET /api/bookings/:code`):**
  - Menampilkan detail lengkap tiket (Sigil code, kereta, rute, waktu, daftar penumpang & nomor kursi, status segel, dan data representasi QR code).
- [ ] **Kronik Perjalanan (`GET /api/bookings`):**
  - Mengambil seluruh riwayat pemesanan milik user yang sedang terotentikasi, diurutkan dari yang terbaru.
- [ ] **Frontend Polishing:** Desain tampilan gulungan perkamen Royal Pass, segel lilin crimson, dan quest log riwayat perjalanan.

### Fase 6: Panel Admin, Pengujian Konkurensi & Finalisasi (Minggu 6)
- [ ] **The Conductor's Chamber (Admin Panel):**
  - `GET /api/admin/dashboard`: Statistik ringkasan (total pesanan, total revenue simulasi, pesanan lunas, hold aktif).
  - `GET /api/admin/bookings`: Daftar seluruh pesanan dari seluruh penumpang.
  - CRUD Stasiun, Kereta, dan Jadwal (khusus role `admin`).
- [ ] **Uji Konkurensi Paralel (50 Thread):**
  - Menjalankan skrip pengujian beban paralel untuk membuktikan nol double-booking.
- [ ] **Dockerization:** Membuat `Dockerfile` multi-stage dan `docker-compose.yml`.
- [ ] **Dokumentasi & Laporan Akhir:** Export koleksi Postman, pembuatan diagram ERD, sequence diagram alur booking, dan slide presentasi demo tugas kuliah.

---

## 5. Strategi Anti Double-Booking & Desain Transaksi

AmbaTrain menerapkan **Defense-in-Depth** untuk menjamin integritas data kursi:

```
[ Incoming Booking Request ]
             │
             ▼
[ 1. Service Layer Validation ]
  • Validasi format input, schedule_id valid, jumlah pax <= kuota
             │
             ▼
[ 2. Database Transaction: BEGIN ]
             │
             ▼
[ 3. Row-Level Pessimistic Locking ]
  • SELECT id FROM seats WHERE id IN (...) FOR UPDATE;
  • Mencegah pembacaan kotor dan serialisasi akses kursi antar-thread
             │
             ▼
[ 4. Active Booking Check ]
  • Pastikan tidak ada booking_passengers aktif untuk jadwal & kursi tsb
  • Jika terdeteksi ada yang mendahului -> ROLLBACK & balas HTTP 409 Conflict
             │
             ▼
[ 5. Insert Bookings & Booking Passengers ]
  • Insert bookings (status = 'held', expires_at = now() + 15 min)
  • Insert booking_passengers (seat_active = true)
             │
             ▼
[ 6. Ultimate Guard: Partial Unique Index ]
  • INDEX: uq_active_seat_per_schedule ON (schedule_id, seat_id) WHERE seat_active = TRUE
  • Jika terjadi race-condition microsecond, constraint ini menjamin transaksi kedua gagal!
             │
             ▼
[ 7. Transaction: COMMIT ] -> HTTP 201 Created
```

---

## 6. Format Kontrak API & Error Envelope

Seluruh respons kegagalan API AmbaTrain mengikuti format error envelope konsisten sesuai **PRD Bagian 12**:

### 6.1 Format Error Standar
```json
{
  "error": {
    "code": "SEAT_UNAVAILABLE",
    "message": "Kursi 3B telah direbut takdir lain.",
    "details": {
      "seat_id": 204,
      "seat_number": "3B"
    }
  }
}
```

### 6.2 Pemetaan Kode HTTP
| Status HTTP | Kode Error | Skenario |
|---|---|---|
| `400 Bad Request` | `INVALID_INPUT` | Validasi form gagal (format email salah, NIK kosong) |
| `401 Unauthorized` | `UNAUTHORIZED` | Token JWT tidak disertakan atau telah kedaluwarsa |
| `403 Forbidden` | `FORBIDDEN` | Pengguna biasa mencoba mengakses endpoint Admin |
| `404 Not Found` | `NOT_FOUND` | Jadwal, stasiun, atau booking tidak ditemukan |
| `409 Conflict` | `SEAT_UNAVAILABLE` | Kursi telah diambil/ditahan oleh pengelana lain |
| `410 Gone` | `BOOKING_EXPIRED` | Batas waktu 15 menit habis saat hendak membayar |
| `500 Server Error` | `INTERNAL_ERROR` | Kesalahan internal server C++ atau koneksi database |

---

## 7. Rencana Pengujian & Script Uji Konkurensi

Sesuai **PRD Bagian 15**, pembuktian keberhasilan anti double-booking dilakukan melalui uji konkurensi minimal 50 request simultan.

### 7.1 Script Pengujian Konkurensi (`scripts/test_concurrency.py`)
### Fase 1: Fondasi Basis Data & Boilerplate C++ (Minggu 1) [STATUS: SELESAI ✅]
- [x] **DB Setup:** Berkas [`db/schema.sql`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/db/schema.sql) siap dieksekusi di Supabase SQL Editor.
- [x] **Seed Data:** Berkas [`db/seed.sql`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/db/seed.sql) siap dieksekusi untuk mengisi 10 stasiun, 6 kereta 3 kelas, dan jadwal dinamis 14 hari.
- [x] **Config Parser:** Terbuat di [`src/config/Config.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/config/Config.hpp) untuk mem-parsing environment variables dari `.env`.
- [x] **Database Connection Pool:** Terbuat di [`src/config/DatabasePool.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/config/DatabasePool.hpp) membungkus koneksi `pqxx::connection` secara thread-safe menggunakan antrean dan scoped RAII guard.
- [x] **Health & Master Endpoints:** Terbuat di [`src/main.cpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/main.cpp) mengimplementasikan `GET /api/health` (dengan pengukuran latensi round-trip DB dan status pool) serta `GET /api/stations`.
- [x] **CORS & Response Utility:** Terbuat di [`src/middleware/CorsMiddleware.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/middleware/CorsMiddleware.hpp) dan [`src/utils/ResponseUtils.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/utils/ResponseUtils.hpp) untuk format JSON envelope standar PRD.
- [x] **Data Models:** Terbuat di `src/models/` ([`Station.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Station.hpp), [`Train.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Train.hpp), [`User.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/User.hpp), [`Schedule.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Schedule.hpp), [`Booking.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Booking.hpp), [`Payment.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/models/Payment.hpp)).

### Fase 2: Autentikasi & Manajemen Data Master (Minggu 2) [STATUS: SELESAI ✅]
- [x] **Hashing Engine ([`CryptoUtils.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/utils/CryptoUtils.hpp)):** Mendukung `libsodium` (Argon2id) dan portable SHA-256 salted hash fallback.
- [x] **JWT Provider ([`JwtUtils.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/utils/JwtUtils.hpp)):** Utilitas penandatanganan dan verifikasi HS256 dengan claims `user_id`, `role`, `email`, dan `exp`.
- [x] **Auth Guard & Middleware ([`AuthGuard.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/middleware/AuthGuard.hpp)):** Ekstraksi header `Bearer <token>`, validasi claims, dan otorisasi role (`user` vs `admin`).
- [x] **Repository Layer:** Terbuat [`UserRepository.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/repositories/UserRepository.hpp) & [`StationRepository.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/repositories/StationRepository.hpp).
- [x] **Service Layer:** Terbuat [`AuthService.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/services/AuthService.hpp) menangani logika bisnis registrasi, login, dan profil.
- [x] **Controller Layer & Endpoints:** Terbuat [`AuthController.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/controllers/AuthController.hpp) & [`StationController.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/controllers/StationController.hpp):
  - `POST /api/auth/register`: Validasi format email, cek duplikasi, hash password, kembalikan JWT.
  - `POST /api/auth/login`: Verifikasi password hash, kembalikan JWT.
  - `GET /api/auth/me`: Endpoint terproteksi mengembalikan profil pengelana yang sedang aktif.
  - `GET /api/stations`: Daftar gerbang stasiun terurut.
- [x] **Integration:** Seluruh route terkoneksi di [`src/main.cpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/main.cpp) dan script uji otomatis [`scripts/test_auth.py`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/scripts/test_auth.py).

### Fase 3: Pencarian Jadwal & Visualisasi Denah Kursi (Minggu 3) [STATUS: SELESAI ✅]
- [x] **Schedule Search API ([`ScheduleRepository.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/repositories/ScheduleRepository.hpp) & [`ScheduleController.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/controllers/ScheduleController.hpp)):**
  - Parameter query: `from`, `to`, `date`, `pax`.
  - Agregasi data kereta (Sovereign, Noble, Yeoman), jam berangkat, jam tiba, durasi, harga.
  - Kalkulasi sisa kursi real-time (mengurangkan total kursi dengan kursi aktif pada booking `paid` dan `held` belum kedaluwarsa).
- [x] **Seat Layout API ([`SeatRepository.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/repositories/SeatRepository.hpp)):**
  - `GET /api/schedules/:id/seats`: Mengelompokkan seluruh gerbong dan kursi dengan status dinamis (`available`, `held` 15 menit, `booked`).
- [x] **Service Layer Validation ([`ScheduleService.hpp`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/src/services/ScheduleService.hpp)):**
  - Validasi jarak rute (`from != to`), format tanggal ISO8601, batas maksimal rombongan (`1 <= pax <= 4`).
- [x] **Frontend Interaktif Bergaya Gothic JRPG:**
  - [`frontend/index.html`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/frontend/index.html): Hero stasiun gothic, form pencarian rute, kartu hasil jadwal responsif.
  - [`frontend/seats.html`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/frontend/seats.html): Signature interactive carriage blueprint dengan kursi kristal bercahaya, lorong gerbong, tab gerbong, dan kalkulator harga party.
- [x] **Automated Testing:** Skrip pengujian otomatis di [`scripts/test_schedules_seats.py`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/scripts/test_schedules_seats.py).

### Fase 4: Mesin Pemesanan Inti & Anti Double-Booking (Minggu 4)
- [ ] **Booking Engine Transaction (`POST /api/bookings`):**
  - Menerima payload: `schedule_id` dan array `passengers` (`name`, `id_number`, `seat_id`).
  - Membuka transaksi database terisolasi: `BEGIN;`
  - Mengeksekusi `SELECT id FROM seats WHERE id IN (...) FOR UPDATE;` untuk mengunci baris kursi dari balapan transaksi konkuren.
  - Memvalidasi apakah kursi masih kosong pada jadwal tersebut. Jika sudah diambil, lakukan `ROLLBACK;` dan kembalikan status `409 Conflict`.
  - Membuat baris baru pada tabel `bookings` dengan `status = 'held'`, kode unik Sigil (`AMB-XXXXXX`), dan `expires_at = NOW() + INTERVAL '15 minutes'`.
  - Memasukkan seluruh data penumpang ke `booking_passengers` dengan `seat_active = TRUE`.
  - Menangani error jika Partial Unique Index `uq_active_seat_per_schedule` dilanggar secara atomik.
  - `COMMIT;` dan mengembalikan respons `201 Created` beserta Sigil dan timer.
- [ ] **Background Expiry Worker (`ExpiryWorker.hpp`):**
  - Menjalankan background thread `std::thread` di C++ yang setiap 60 detik memanggil `SELECT public.expire_held_bookings();`.
  - Mengubah booking kedaluwarsa menjadi `expired`, memicu trigger database yang otomatis melepaskan kursi (`seat_active = FALSE`).

### Fase 5: Pembayaran Simulasi, Royal Pass & Kronik (Minggu 5)
- [ ] **Payment API (`POST /api/bookings/:id/pay`):**
  - Validasi bahwa pemesanan masih berstatus `held` dan `expires_at > NOW()`.
  - Memperbarui status booking menjadi `paid` (Sealed).
  - Menyimpan data transaksi ke tabel `payments` dengan metode yang dipilih (`tribute_crystal`, `qris`, `bank_transfer`).
- [ ] **Cancellation API (`POST /api/bookings/:id/cancel`):**
  - Mengubah status booking menjadi `cancelled` (Forsaken).
  - Database trigger otomatis membebaskan kursi seketika.
- [ ] **Royal Pass E-Ticket API (`GET /api/bookings/:code`):**
  - Menampilkan detail lengkap tiket (Sigil code, kereta, rute, waktu, daftar penumpang & nomor kursi, status segel, dan data representasi QR code).
- [ ] **Kronik Perjalanan (`GET /api/bookings`):**
  - Mengambil seluruh riwayat pemesanan milik user yang sedang terotentikasi, diurutkan dari yang terbaru.
- [ ] **Frontend Polishing:** Desain tampilan gulungan perkamen Royal Pass, segel lilin crimson, dan quest log riwayat perjalanan.

### Fase 6: Panel Admin, Pengujian Konkurensi & Finalisasi (Minggu 6)
- [ ] **The Conductor's Chamber (Admin Panel):**
  - `GET /api/admin/dashboard`: Statistik ringkasan (total pesanan, total revenue simulasi, pesanan lunas, hold aktif).
  - `GET /api/admin/bookings`: Daftar seluruh pesanan dari seluruh penumpang.
  - CRUD Stasiun, Kereta, dan Jadwal (khusus role `admin`).
- [ ] **Uji Konkurensi Paralel (50 Thread):**
  - Menjalankan skrip pengujian beban paralel untuk membuktikan nol double-booking.
- [ ] **Dockerization:** Membuat `Dockerfile` multi-stage dan `docker-compose.yml`.
- [ ] **Dokumentasi & Laporan Akhir:** Export koleksi Postman, pembuatan diagram ERD, sequence diagram alur booking, dan slide presentasi demo tugas kuliah.

---

## 5. Strategi Anti Double-Booking & Desain Transaksi

AmbaTrain menerapkan **Defense-in-Depth** untuk menjamin integritas data kursi:

```
[ Incoming Booking Request ]
             │
             ▼
[ 1. Service Layer Validation ]
  • Validasi format input, schedule_id valid, jumlah pax <= kuota
             │
             ▼
[ 2. Database Transaction: BEGIN ]
             │
             ▼
[ 3. Row-Level Pessimistic Locking ]
  • SELECT id FROM seats WHERE id IN (...) FOR UPDATE;
  • Mencegah pembacaan kotor dan serialisasi akses kursi antar-thread
             │
             ▼
[ 4. Active Booking Check ]
  • Pastikan tidak ada booking_passengers aktif untuk jadwal & kursi tsb
  • Jika terdeteksi ada yang mendahului -> ROLLBACK & balas HTTP 409 Conflict
             │
             ▼
[ 5. Insert Bookings & Booking Passengers ]
  • Insert bookings (status = 'held', expires_at = now() + 15 min)
  • Insert booking_passengers (seat_active = true)
             │
             ▼
[ 6. Ultimate Guard: Partial Unique Index ]
  • INDEX: uq_active_seat_per_schedule ON (schedule_id, seat_id) WHERE seat_active = TRUE
  • Jika terjadi race-condition microsecond, constraint ini menjamin transaksi kedua gagal!
             │
             ▼
[ 7. Transaction: COMMIT ] -> HTTP 201 Created
```

---

## 6. Format Kontrak API & Error Envelope

Seluruh respons kegagalan API AmbaTrain mengikuti format error envelope konsisten sesuai **PRD Bagian 12**:

### 6.1 Format Error Standar
```json
{
  "error": {
    "code": "SEAT_UNAVAILABLE",
    "message": "Kursi 3B telah direbut takdir lain.",
    "details": {
      "seat_id": 204,
      "seat_number": "3B"
    }
  }
}
```

### 6.2 Pemetaan Kode HTTP
| Status HTTP | Kode Error | Skenario |
|---|---|---|
| `400 Bad Request` | `INVALID_INPUT` | Validasi form gagal (format email salah, NIK kosong) |
| `401 Unauthorized` | `UNAUTHORIZED` | Token JWT tidak disertakan atau telah kedaluwarsa |
| `403 Forbidden` | `FORBIDDEN` | Pengguna biasa mencoba mengakses endpoint Admin |
| `404 Not Found` | `NOT_FOUND` | Jadwal, stasiun, atau booking tidak ditemukan |
| `409 Conflict` | `SEAT_UNAVAILABLE` | Kursi telah diambil/ditahan oleh pengelana lain |
| `410 Gone` | `BOOKING_EXPIRED` | Batas waktu 15 menit habis saat hendak membayar |
| `500 Server Error` | `INTERNAL_ERROR` | Kesalahan internal server C++ atau koneksi database |

---

## 7. Rencana Pengujian & Script Uji Konkurensi

Sesuai **PRD Bagian 15**, pembuktian keberhasilan anti double-booking dilakukan melalui uji konkurensi minimal 50 request simultan.

### 7.1 Script Pengujian Konkurensi (`scripts/test_concurrency.py`)
```python
import concurrent.futures
import requests
import json
import sys

API_BASE = "http://localhost:8080/api"
SCHEDULE_ID = 1
TARGET_SEAT_ID = 5  # Kursi yang akan diperebutkan secara bersamaan

def authenticate_user(email, password):
    res = requests.post(f"{API_BASE}/auth/login", json={"email": email, "password": password})
    if res.status_code == 200:
        return res.json()["token"]
    raise Exception("Gagal login user uji")

def attempt_booking(worker_id, token):
    headers = {"Authorization": f"Bearer {token}", "Content-Type": "application/json"}
    payload = {
        "schedule_id": SCHEDULE_ID,
        "passengers": [
            {
                "name": f"Contender #{worker_id}",
                "id_number": f"73710000000000{worker_id:02d}",
                "seat_id": TARGET_SEAT_ID
            }
        ]
    }
    res = requests.post(f"{API_BASE}/bookings", json=payload, headers=headers)
    return res.status_code

def main():
    print(f"🚀 Memulai Uji Konkurensi: 50 Request paralel memperebutkan Seat ID {TARGET_SEAT_ID}...")
    token = authenticate_user("traveler@ambatrain.com", "Password123!")

    with concurrent.futures.ThreadPoolExecutor(max_workers=50) as executor:
        futures = [executor.submit(attempt_booking, i, token) for i in range(1, 51)]
        results = [f.result() for f in concurrent.futures.as_completed(futures)]

    success_count = results.count(201)
    conflict_count = results.count(409)
    other_count = len(results) - (success_count + conflict_count)

    print("\n--- HASIL PENGUJIAN KONKURENSI ---")
    print(f"Total Request Dikirim : 50")
    print(f"Berhasil (201 Created) : {success_count}")
    print(f"Konflik (409 Conflict) : {conflict_count}")
    print(f"Lainnya                : {other_count}")

    if success_count == 1 and conflict_count == 49:
        print("\n✅ PENGUJIAN LULUS SEMPURNA: 0 Double Booking! Konsistensi transaksi C++ terbukti aman.")
        sys.exit(0)
    else:
        print("\n❌ PENGUJIAN GAGAL: Terjadi double booking atau inkonsistensi status!")
        sys.exit(1)

if __name__ == "__main__":
    main()
```

---

## 8. Matriks Deliverable Akademik

Sesuai **PRD Bagian 19**, berikut adalah checklist dokumen dan luaran yang siap diserahkan untuk tugas kuliah:

| Deliverable | Status | Lokasi Berkas |
|---|---|---|
| **Product Requirements Document (PRD)** | ✅ Selesai | [`PRD.md`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/PRD.md) |
| **Design System & Style Guide** | ✅ Selesai | [`desain.md`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/desain.md) |
| **Database Schema DDL & Index** | ✅ Selesai | [`db/schema.sql`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/db/schema.sql) |
| **Database Seed Data (14 Hari)** | ✅ Selesai | [`db/seed.sql`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/db/seed.sql) |
| **Katalog Query Supabase & C++** | ✅ Selesai | [`query.md`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/query.md) |
| **Master Implementation Plan** | ✅ Selesai | [`implementation-plan.md`](file:///c:/Users/MY%20VICTUS/Desktop/AmbaTrain/implementation-plan.md) |
| **Build Configuration (CMake + vcpkg)** | ⏳ Siap Dibuat | `CMakeLists.txt`, `vcpkg.json` |
| **Source Code Backend C++** | ⏳ Siap Dibuat | `src/` (Controllers, Services, Repositories) |
| **Frontend Gothic Shakespearean** | ⏳ Siap Dibuat | `frontend/` (HTML, CSS, JS) |
| **Script Pengujian Konkurensi** | ⏳ Siap Dibuat | `scripts/test_concurrency.py` |
| **Containerization (Docker)** | ⏳ Siap Dibuat | `Dockerfile`, `docker-compose.yml` |
| **Laporan & Dokumentasi API** | ⏳ Siap Dibuat | `docs/` |
