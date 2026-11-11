# 🚂 AmbaTrain: Design System

> **Tagline:** *"All the world's a station, and all the men and women merely passengers."*
> **Tema:** *A Fantasy Based on Reality*: dark, gothic, Shakespearean, dengan nuansa epik ala Final Fantasy.

---

## 1. Konsep Besar

### 1.1 Premis
Dunia nyata (stasiun, kota, rute kereta Indonesia) dilihat lewat lensa **dunia fantasi gelap**. Kota-kota asli tetap ada, tapi dibungkus seperti kerajaan dalam sebuah epos. Pengguna bukan sekadar "beli tiket", tapi **memulai sebuah perjalanan (quest)**.

| Dunia Nyata | Dunia AmbaTrain |
|---|---|
| Stasiun | **Gerbang** (Gate) |
| Kereta | **Lokomotif / Steed of Steel** |
| Kelas Eksekutif | **Sovereign** |
| Kelas Bisnis | **Noble** |
| Kelas Ekonomi | **Yeoman** |
| Kode booking | **Sigil** |
| E-ticket | **Royal Pass** / **Scroll of Passage** |
| Pembayaran | **Tribute** |
| Riwayat pesanan | **Chronicle** (Kronik Perjalanan) |
| Admin panel | **The Conductor's Chamber** |
| Login | **Enter the Realm** |
| Register | **Swear Thy Oath** |

> Rute dan nama kota **tetap nyata** (Makassar, Bandung, Yogyakarta, dll.) supaya aplikasi tetap fungsional dan tidak membingungkan. Fantasinya ada di **kemasan, bahasa, dan visual**, bukan di data.

### 1.2 Tiga Pilar Rasa
1. **Gothic Grandeur**: megah, kelam, arsitektural (lengkung runcing, kaca patri, ornamen besi tempa).
2. **Shakespearean Drama**: bahasa puitis, teatrikal, tragis-romantis.
3. **JRPG Cinematic**: UI bergaya menu game (frame berornamen, kristal bercahaya, partikel, transisi sinematik).

### 1.3 Referensi Mood (inspirasi, **bukan** menyalin aset)
- Final Fantasy (menu windows, kristal, serif elegan, aura mistis)
- Gothic cathedral & Victorian railway station
- Teater Globe / panggung Shakespeare (tirai, lilin, naskah)
- Bloodborne / Castlevania (palet kelam, aksen merah darah)

> ⚠️ **Catatan hak cipta:** Jangan pakai logo, karakter, musik, atau aset resmi Final Fantasy. Buat sendiri (SVG ornamen, ikon, ilustrasi) atau pakai aset berlisensi bebas (CC0/OFL).

---

## 2. Palet Warna

### 2.1 Warna Utama

| Token | Nama | Hex | Penggunaan |
|---|---|---|---|
| `--void` | Void Black | `#0A0A0F` | Background utama |
| `--obsidian` | Obsidian | `#12121A` | Surface / card |
| `--midnight` | Midnight Violet | `#1C1530` | Panel sekunder, header |
| `--crimson` | Blood Crimson | `#8B1A2B` | Aksen primer, CTA, bahaya |
| `--crimson-glow` | Ember | `#C42B45` | Hover CTA, highlight |
| `--gold` | Antique Gold | `#C9A24B` | Border ornamen, judul, harga |
| `--gold-light` | Candle Gold | `#E8D08A` | Teks aksen, ikon aktif |
| `--crystal` | Crystal Azure | `#5EC8E5` | Kristal, info, kursi dipilih (aksen FF) |
| `--bone` | Bone White | `#EDE6D6` | Teks utama |
| `--ash` | Ash Grey | `#9A94A6` | Teks sekunder, disabled |

### 2.2 Warna Fungsional (status)

| Status | Warna | Hex |
|---|---|---|
| Success / Confirmed | Emerald Dim | `#2F8F6B` |
| Warning / Held | Amber Flame | `#D9892B` |
| Error / Cancelled | Blood Crimson | `#8B1A2B` |
| Info / Selected | Crystal Azure | `#5EC8E5` |

### 2.3 Gradien Signature
```css
--grad-sky:     linear-gradient(180deg, #0A0A0F 0%, #1C1530 60%, #2A1B3D 100%);
--grad-gold:    linear-gradient(135deg, #8A6D2B 0%, #E8D08A 50%, #8A6D2B 100%);
--grad-crystal: radial-gradient(circle, #5EC8E5 0%, transparent 70%);
--grad-blood:   linear-gradient(135deg, #5C0F1B 0%, #C42B45 100%);
```

### 2.4 Aturan Penggunaan
- Dominasi **60% gelap** (void/obsidian), **30% ungu-malam**, **10% aksen** (gold/crimson/crystal).
- **Gold** untuk dekorasi & hierarki. **Crimson** untuk aksi penting. **Crystal** untuk interaktif/dipilih.
- Rasio kontras teks minimal **WCAG AA (4.5:1)**. `--bone` di atas `--void` sudah aman.

---

## 3. Tipografi

| Peran | Font | Fallback | Catatan |
|---|---|---|---|
| **Display / Logo / H1** | **Cinzel Decorative** | `Cinzel`, serif | Nuansa Romawi-gothic, megah |
| **Heading (H2-H4)** | **Cinzel** | Georgia, serif | Huruf kapital, letter-spacing lebar |
| **Body / Paragraf** | **Cormorant Garamond** atau **EB Garamond** | Georgia, serif | Elegan, mudah dibaca |
| **UI / Form / Tabel** | **Inter** atau **Lora** | system-ui | Keterbacaan tinggi untuk data |
| **Aksen "naskah"** | **IM Fell English** *(italic)* | serif | Kutipan Shakespeare, microcopy |
| **Angka tiket / kode** | **JetBrains Mono** | monospace | Sigil & nomor kursi |

Semua tersedia di **Google Fonts (OFL, gratis)**.

### Skala Tipografi
```
H1  3.5rem / 56px   Cinzel Decorative   tracking 0.08em
H2  2.25rem / 36px  Cinzel              tracking 0.06em
H3  1.5rem  / 24px  Cinzel              tracking 0.05em
Body 1.125rem/18px  Cormorant Garamond  line-height 1.6
UI   1rem   / 16px  Inter               line-height 1.5
Caption 0.875rem    Inter               color: --ash
```

> **Tips:** Teks dekoratif (Cinzel Decorative, IM Fell) hanya untuk judul & aksen. **Form, tabel jadwal, dan harga wajib pakai font UI yang jelas** agar usability tetap bagus.

---

## 4. Bahasa Visual

### 4.1 Elemen Ornamen
- **Lengkung runcing (pointed arch / ogive)** sebagai bingkai hero & modal.
- **Rose window** (kaca patri bundar) sebagai motif latar & loading spinner.
- **Border besi tempa**: garis gold tipis + sudut berornamen (corner flourish) pada setiap card.
- **Kristal bercahaya** sebagai ikon sentral (logo, kursi dipilih, tombol utama).
- **Roda gigi + jam kuno** sebagai motif steampunk-gothic untuk elemen kereta.
- **Gagak, mawar berduri, tengkorak**: dipakai hemat sebagai aksen di empty state/error.
- **Tekstur**: grain halus, vignette gelap di tepi layar, kabut ringan.

### 4.2 Logo AmbaTrain
- Wordmark **"AMBATRAIN"** dalam Cinzel Decorative, gradien gold.
- Simbol: **kristal bersayap di tengah rel yang menyatu ke cakrawala**, di dalam lengkung runcing.
- Varian: full (simbol + teks), simbol saja (favicon), mono gold, mono bone.

### 4.3 Ikonografi
- Gaya: **line icon tipis (1.5px)** warna gold, sudut sedikit runcing.
- Saran: **Lucide** / **Phosphor** sebagai dasar, lalu tint gold + glow tipis saat aktif.
- Ikon kustom (SVG): kristal, lokomotif, kursi, pedang-tiket, gulungan naskah.

---

## 5. Layout & Grid

- Grid **12 kolom**, max-width **1200px**, gutter 24px.
- Spasi berbasis **8px** (`8, 16, 24, 32, 48, 64, 96`).
- Border-radius **kecil/tajam (2-6px)**, hindari bentuk bulat berlebihan agar tetap gothic.
- Elevasi lewat **glow** (bukan shadow abu-abu): `box-shadow: 0 0 24px rgba(201,162,75,.15)`.

### Breakpoint
| Nama | Lebar |
|---|---|
| Mobile | ≤ 640px |
| Tablet | 641 - 1024px |
| Desktop | ≥ 1025px |

---

## 6. Komponen UI

### 6.1 Tombol
| Tipe | Gaya |
|---|---|
| **Primary** | Gradien crimson, border gold 1px, teks bone, huruf kapital Cinzel. Hover: glow ember + sedikit naik |
| **Secondary** | Transparan, border gold, teks gold. Hover: fill gold 10% |
| **Ghost** | Teks gold-light, underline ornamen saat hover |
| **Disabled** | Ash grey, tanpa glow |

### 6.2 Card ("Panel Menu ala JRPG")
- Background `--obsidian` dengan opacity 90% + `backdrop-filter: blur`.
- Border ganda tipis (gold luar, ungu dalam) + **corner flourish SVG** di 4 sudut.
- Header card pakai Cinzel + garis pemisah berornamen (◆───◆).

### 6.3 Form Input
- Background `--void`, border bawah gold (gaya perkamen/garis tulisan).
- Focus: border berubah crystal + glow biru tipis.
- Label kecil kapital berspasi lebar. Placeholder pakai IM Fell italic.
- Error: teks crimson + ikon mawar berduri kecil.

### 6.4 Navbar
- Transparan di atas hero, berubah solid `--void` saat scroll.
- Kiri: logo. Tengah: *Cari Rute, Kronik, Tentang*. Kanan: *Enter the Realm* (login).
- Garis ornamen gold tipis di bawah navbar.

### 6.5 Modal & Toast
- Modal berbentuk **lengkung runcing / kaca patri** dengan overlay gelap + blur.
- Toast muncul seperti **notifikasi quest** dari pojok kanan atas (slide + glow).

### 6.6 Badge Status Booking
| Status | Label tema | Warna |
|---|---|---|
| `held` | ⏳ *Awaiting Tribute* | Amber |
| `paid` | ✦ *Sealed* | Emerald |
| `cancelled` | ✕ *Forsaken* | Crimson |
| `expired` | ☠ *Faded to Dust* | Ash |

---

## 7. Halaman & Layout

### 7.1 Landing Page (Hero)
```
┌───────────────────────────────────────────────┐
│  [Logo]        Rute  Kronik  Tentang   [Masuk]│
│                                               │
│   Latar: stasiun gothic malam hari, kabut,    │
│   lokomotif uap bersinar kristal biru,        │
│   partikel debu/ember melayang                │
│                                               │
│        A M B A T R A I N                      │
│   "All the world's a station..."              │
│                                               │
│  ┌──────────── PANEL PENCARIAN ────────────┐  │
│  │ Dari [Gerbang Asal] → Ke [Gerbang Tuju] │  │
│  │ Tanggal [ ]   Penumpang [ ]  [⚔ MULAI] │  │
│  └─────────────────────────────────────────┘  │
│                                               │
│   Rute Populer  ·  Kelas Kereta  ·  Cerita    │
└───────────────────────────────────────────────┘
```

### 7.2 Hasil Pencarian Jadwal
- Daftar **kartu perjalanan**: nama kereta, jam berangkat → tiba, durasi (garis rel dengan titik), kelas, harga (gold, besar), sisa kursi.
- Filter sidebar: kelas, jam, harga (ala menu *equipment* JRPG).
- Garis rel ornamen horizontal menghubungkan jam berangkat dan tiba.

### 7.3 Pilih Kursi (Layar Signature ✨)
- Tampilan **denah gerbong dari atas** dengan gaya blueprint-gothic.
- Kursi sebagai **kristal / sigil kecil**:
  | State | Visual |
  |---|---|
  | Available | Outline gold, redup |
  | Selected | Kristal biru bersinar (pulse) |
  | Held (orang lain) | Amber, ikon jam pasir |
  | Booked | Gelap, retak, ikon ✕ |
- Panel kanan: ringkasan "party" (penumpang dipilih), total harga, tombol **Lanjut ke Tribute**.
- Tab gerbong ala tab menu game (*Carriage I, II, III*).

### 7.4 Data Penumpang & Pembayaran
- Form berformat **stepper/quest progress**: *Pilih Rute → Pilih Kursi → Data Diri → Tribute → Sealed*.
- Timer hitung mundur 15 menit tampil sebagai **jam pasir / kristal yang memudar**.
- Metode pembayaran sebagai kartu pilihan (simulasi).

### 7.5 Royal Pass (E-Ticket)
- Bentuk **gulungan perkamen / kartu bangsawan** dengan segel lilin crimson.
- Isi: Sigil (kode booking), rute, jam, kursi, nama penumpang, **QR code** dengan frame ornamen.
- Tombol *Unduh Scroll (PDF)*.
- Efek: reveal animasi segel pecah saat pertama dibuka.

### 7.6 Kronik (Riwayat Pesanan)
- Timeline vertikal bergaya **buku sejarah / quest log**, tiap entri memiliki badge status.

### 7.7 Login / Register
- Layout dua kolom: kiri ilustrasi (gerbang katedral), kanan form di panel kaca patri.
- Judul: **"Enter the Realm"** / **"Swear Thy Oath"**.

### 7.8 Panel Admin: The Conductor's Chamber
- Tema sama tapi **lebih fungsional**: sidebar gelap, tabel data jelas, aksen gold minimal.
- Prioritaskan keterbacaan dan efisiensi daripada dekorasi.

### 7.9 Empty State & Error
| Kondisi | Visual | Copy |
|---|---|---|
| Tidak ada jadwal | Gagak di atas rel kosong | *"The rails lie silent this day."* |
| 404 | Stasiun terbengkalai + kabut | *"Alas, this road leadeth nowhere."* |
| 500 | Lokomotif rusak berasap | *"The engine hath fallen. Our smiths are at work."* |
| Riwayat kosong | Buku kosong + lilin | *"Thy chronicle awaits its first page."* |

---

## 8. Copywriting & Microcopy

**Gaya:** puitis-teatrikal, tapi **tetap jelas**. Gabungkan Inggris klasik (aksen) dengan Bahasa Indonesia (informasi inti) agar pengguna tidak bingung.

| Konteks | Teks |
|---|---|
| Tombol cari | **Mulai Perjalanan** *(Begin the Journey)* |
| Tombol booking | **Segel Takdirmu** *(Seal Thy Fate)* |
| Tombol bayar | **Bayar Tribute** |
| Batal tiket | **Lepaskan Perjalanan** |
| Loading | *"Menyalakan tungku lokomotif..."* |
| Sukses booking | *"Takdirmu telah tersegel. Perjalanan menantimu."* |
| Kursi diambil | *"Kursi ini telah direbut takdir lain."* |
| Timer habis | *"Waktu telah habis. Kursimu kembali ke kabut."* |
| Validasi email | *"Surat elang tak dapat terkirim; periksa alamat emailmu."* |

**Kutipan Shakespeare** (domain publik) untuk dekorasi:
- *"All the world's a stage."*: As You Like It
- *"Parting is such sweet sorrow."*: Romeo and Juliet
- *"There is a tide in the affairs of men."*: Julius Caesar
- *"Journeys end in lovers meeting."*: Twelfth Night

> **Aturan emas:** informasi penting (harga, jam, kursi, error) **selalu ditulis jelas dulu**, copy puitis hanya sebagai pembungkus.

---

## 9. Motion & Animasi

| Elemen | Animasi | Durasi |
|---|---|---|
| Transisi halaman | Fade + sedikit slide, seperti tirai panggung | 400-600ms |
| Hover tombol | Glow tumbuh + naik 2px | 200ms |
| Kursi dipilih | Pulse kristal biru | 1.5s loop |
| Hero | Partikel ember/debu melayang, kabut bergerak lambat, lokomotif parallax | loop |
| Segel tiket | Segel lilin pecah → gulungan terbuka | 1.2s |
| Loading | Rose window berputar / roda gigi | loop |
| Toast | Slide-in dari kanan + glow | 300ms |

**Kecepatan & aksesibilitas**
- Easing: `cubic-bezier(0.22, 1, 0.36, 1)`.
- Hormati `prefers-reduced-motion`: matikan partikel & parallax.
- Animasi berat hanya di hero & momen spesial (sukses booking), **bukan** di form.

---

## 10. Audio (Opsional)

- Ambient: organ katedral / dentang jam / uap kereta (mute default, ada toggle 🔊).
- SFX: klik kristal, denting lonceng saat sukses, suara tiket distempel.
- Gunakan musik **royalty-free / CC0** (Pixabay, Freesound, Incompetech). **Jangan** pakai musik Final Fantasy.

---

## 11. Design Tokens (siap pakai)

### 11.1 CSS Variables
```css
:root {
  /* Colors */
  --void: #0A0A0F;
  --obsidian: #12121A;
  --midnight: #1C1530;
  --crimson: #8B1A2B;
  --crimson-glow: #C42B45;
  --gold: #C9A24B;
  --gold-light: #E8D08A;
  --crystal: #5EC8E5;
  --bone: #EDE6D6;
  --ash: #9A94A6;
  --success: #2F8F6B;
  --warning: #D9892B;

  /* Typography */
  --font-display: 'Cinzel Decorative', 'Cinzel', serif;
  --font-heading: 'Cinzel', Georgia, serif;
  --font-body: 'Cormorant Garamond', Georgia, serif;
  --font-ui: 'Inter', system-ui, sans-serif;
  --font-script: 'IM Fell English', serif;
  --font-mono: 'JetBrains Mono', monospace;

  /* Spacing & shape */
  --space-1: 8px;  --space-2: 16px; --space-3: 24px;
  --space-4: 32px; --space-6: 48px; --space-8: 64px;
  --radius: 4px;

  /* Effects */
  --glow-gold: 0 0 24px rgba(201, 162, 75, .18);
  --glow-crystal: 0 0 20px rgba(94, 200, 229, .35);
  --glow-crimson: 0 0 20px rgba(196, 43, 69, .35);
  --ease: cubic-bezier(0.22, 1, 0.36, 1);
}
```

### 11.2 Tailwind Config (kalau pakai Tailwind)
```js
// tailwind.config.js
module.exports = {
  theme: {
    extend: {
      colors: {
        void: '#0A0A0F', obsidian: '#12121A', midnight: '#1C1530',
        crimson: { DEFAULT: '#8B1A2B', glow: '#C42B45' },
        gold: { DEFAULT: '#C9A24B', light: '#E8D08A' },
        crystal: '#5EC8E5', bone: '#EDE6D6', ash: '#9A94A6',
      },
      fontFamily: {
        display: ['"Cinzel Decorative"', 'serif'],
        heading: ['Cinzel', 'serif'],
        body: ['"Cormorant Garamond"', 'serif'],
        ui: ['Inter', 'sans-serif'],
        script: ['"IM Fell English"', 'serif'],
      },
      boxShadow: {
        gold: '0 0 24px rgba(201,162,75,.18)',
        crystal: '0 0 20px rgba(94,200,229,.35)',
      },
    },
  },
}
```

---

## 12. Aset yang Perlu Disiapkan

| Aset | Sumber / Cara |
|---|---|
| Ilustrasi hero (stasiun gothic + lokomotif) | Generate AI (Midjourney/SD) atau Unsplash + edit; pastikan lisensi aman |
| Ornamen SVG (corner flourish, divider, rose window) | Buat di Figma/Inkscape, atau SVG gratis (SVGRepo, Freepik free) |
| Ikon | Lucide / Phosphor + kustom |
| Font | Google Fonts |
| Tekstur grain / kabut | Subtle Patterns, Transparent Textures (CC) |
| Partikel | `tsParticles` atau canvas sendiri |
| Audio | Pixabay / Freesound / Incompetech |

---

## 13. Aksesibilitas & Kinerja

- Kontras teks minimal **AA**; jangan taruh teks kecil di atas gambar tanpa overlay gelap.
- Semua gambar dekoratif punya `alt=""`, gambar bermakna punya alt jelas.
- Navigasi **keyboard** penuh (terutama pemilihan kursi); focus ring warna crystal.
- Denah kursi punya label teks (bukan hanya warna): ikon + nomor kursi.
- Kompres gambar (WebP/AVIF), lazy-load, batasi partikel di perangkat low-end.
- Pertimbangkan **toggle "Mode Sederhana"** untuk mematikan efek berat.

---

## 14. Rencana Implementasi Desain

| Tahap | Aktivitas |
|---|---|
| 1 | Finalisasi palet + font, buat **moodboard** & logo di Figma |
| 2 | Wireframe low-fi seluruh halaman (Section 7) |
| 3 | Buat **komponen dasar** (tombol, card, input, badge) + token CSS |
| 4 | Hi-fi halaman kunci: **Landing, Pilih Kursi, Royal Pass** |
| 5 | Implementasi frontend, hubungkan ke REST API C++ |
| 6 | Tambah motion, audio, polish |
| 7 | Uji kontras, responsif, dan performa |

---

## 15. Checklist Kualitas

- [ ] Tema konsisten di semua halaman
- [ ] Informasi penting tetap mudah dibaca (harga, jam, kursi)
- [ ] Denah kursi jelas & bisa dipakai di mobile
- [ ] Font dekoratif hanya untuk judul/aksen
- [ ] Tidak ada aset berhak cipta (FF, musik, gambar) tanpa lisensi
- [ ] `prefers-reduced-motion` dihormati
- [ ] Responsif mobile, tablet, desktop
- [ ] Halaman error & empty state sudah bertema

---

*AmbaTrain Design System v1.0: "Every journey is a tragedy or a triumph. Choose thy ticket."* 🌑✦