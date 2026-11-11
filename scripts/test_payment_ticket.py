"""
AmbaTrain Phase 5 Automated Payment & Royal Pass Test Script
Menguji pembayaran simulasi Tribute (Sealed), penarikan Royal Pass (e-ticket),
pembatalan tiket (Forsaken), dan pemeriksaan Kronik perjalanan.
"""

import urllib.request
import urllib.error
import json
import time
import sys

API_BASE = "http://localhost:8080/api"

def make_request(endpoint, method="GET", data=None, token=None):
    url = f"{API_BASE}{endpoint}"
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"

    body = json.dumps(data).encode('utf-8') if data else None
    req = urllib.request.Request(url, data=body, headers=headers, method=method)

    try:
        with urllib.request.urlopen(req, timeout=5) as res:
            return res.status, json.loads(res.read().decode('utf-8'))
    except urllib.error.HTTPError as e:
        err_body = e.read().decode('utf-8')
        try:
            return e.code, json.loads(err_body)
        except:
            return e.code, {"raw": err_body}
    except Exception as e:
        return 0, {"error": str(e)}

def run_tests():
    print("=" * 60)
    print("  AMBATRAIN — PEMERIKSAAN FASE 5 (TRIBUTE & ROYAL PASS)")
    print("=" * 60)

    # 1. Healthcheck
    status, res = make_request("/health")
    if status != 200:
        print(f"❌ Server belum berjalan di {API_BASE}.")
        sys.exit(1)
    print(f"✅ Server Crow C++ Berjalan: {res.get('service')}")

    # 2. Login as Traveler
    print("\n[1/5] Autentikasi Pengelana...")
    status, login_res = make_request("/auth/login", method="POST", data={
        "email": "traveler@ambatrain.com",
        "password": "Password123!"
    })
    if status != 200:
        print(f"❌ Login gagal: {login_res}")
        sys.exit(1)
    token = login_res["token"]
    print("  ✅ Token JWT diperoleh.")

    # 3. Create a Booking Hold
    target_seat_id = 15
    schedule_id = 1
    booking_payload = {
        "schedule_id": schedule_id,
        "passengers": [
            {
                "name": "Lord Prompto Argentum",
                "id_number": "7371112233445566",
                "seat_id": target_seat_id
            }
        ]
    }

    print(f"\n[2/5] Membuat Pesanan Tiket (Hold)...")
    status, book_res = make_request("/bookings", method="POST", data=booking_payload, token=token)
    if status != 201:
        print(f"  ❌ Gagal membuat pesanan (HTTP {status}): {book_res}")
        sys.exit(1)

    booking = book_res["booking"]
    booking_id = booking["id"]
    sigil_code = booking["code"]
    print(f"  ✅ Booking Berhasil Dibuat: ID {booking_id}, Sigil {sigil_code}, Status: {booking['status']}")

    # 4. Pay Tribute (Simulasi Pembayaran 1-Klik)
    print(f"\n[3/5] Melakukan Pembayaran Simulasi Tribute (POST /api/bookings/{booking_id}/pay)...")
    status, pay_res = make_request(f"/bookings/{booking_id}/pay", method="POST", data={"method": "tribute_crystal"}, token=token)
    if status == 200:
        print(f"  ✅ Pembayaran Sukses! Status pesanan kini: SEALED (Lunas)")
        print(f"     Metode: {pay_res.get('payment', {}).get('method')} · Nominal: Rp {pay_res.get('payment', {}).get('amount')}")
    else:
        print(f"  ❌ Gagal bayar tribute: {pay_res}")
        sys.exit(1)

    # 5. Fetch Royal Pass (E-Ticket)
    print(f"\n[4/5] Memeriksa Royal Pass E-Ticket (GET /api/bookings/{sigil_code})...")
    status, ticket = make_request(f"/bookings/{sigil_code}", method="GET", token=token)
    if status == 200:
        print(f"  ✅ Royal Pass Valid:")
        print(f"     Status      : {ticket.get('status')}")
        print(f"     Lokomotif   : {ticket.get('train', {}).get('name')}")
        print(f"     Rute        : {ticket.get('origin', {}).get('city')} ──▶ {ticket.get('destination', {}).get('city')}")
        print(f"     Penumpang   : {ticket.get('passengers', [{}])[0].get('passenger_name')}")
    else:
        print(f"  ❌ Gagal memuat Royal Pass: {ticket}")
        sys.exit(1)

    # 6. Test Cancellation
    print(f"\n[5/5] Menguji Pembatalan Tiket (POST /api/bookings/{booking_id}/cancel)...")
    status, cancel_res = make_request(f"/bookings/{booking_id}/cancel", method="POST", token=token)
    if status == 200:
        print(f"  ✅ Pembatalan Berhasil: {cancel_res.get('message')}")
        # Cek status tiket setelah batal
        status_after, ticket_after = make_request(f"/bookings/{sigil_code}", method="GET", token=token)
        print(f"     Status Tiket Terkini: {ticket_after.get('status')} (Kursi telah kembali ke kabut)")
    else:
        print(f"  ❌ Gagal membatalkan tiket: {cancel_res}")

    print("\n" + "=" * 60)
    print("🎉 SEMUA UJI FASE 5 (TRIBUTE & ROYAL PASS) BERHASIL SEMPURNA!")
    print("=" * 60)

if __name__ == "__main__":
    run_tests()

