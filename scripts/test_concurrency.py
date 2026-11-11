"""
AmbaTrain Concurrency Test Harness
Simulates 50 simultaneous parallel HTTP requests booking the exact same seat.
Verifies that the Anti Double-Booking locking mechanism guarantees:
  - Exactly 1 request receives HTTP 201 Created
  - The remaining 49 requests receive HTTP 409 Conflict
"""

import concurrent.futures
import requests
import json
import sys

API_BASE = "http://localhost:8080/api"
SCHEDULE_ID = 1
TARGET_SEAT_ID = 5  # Seat ID being contested

def authenticate_user(email, password):
    res = requests.post(f"{API_BASE}/auth/login", json={"email": email, "password": password})
    if res.status_code == 200:
        return res.json()["token"]
    raise Exception("Gagal login user uji. Pastikan server C++ dan seed database sudah berjalan.")

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
    try:
        res = requests.post(f"{API_BASE}/bookings", json=payload, headers=headers, timeout=5)
        return res.status_code
    except Exception as e:
        return f"ERROR: {str(e)}"

def main():
    print(f"🚀 Memulai Uji Konkurensi: 50 Request paralel memperebutkan Seat ID {TARGET_SEAT_ID}...")
    try:
        token = authenticate_user("traveler@ambatrain.com", "Password123!")
    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)

    with concurrent.futures.ThreadPoolExecutor(max_workers=50) as executor:
        futures = [executor.submit(attempt_booking, i, token) for i in range(1, 51)]
        results = [f.result() for f in concurrent.futures.as_completed(futures)]

    success_count = results.count(201)
    conflict_count = results.count(409)
    other_count = len(results) - (success_count + conflict_count)

    print("\n" + "="*50)
    print("HASIL PENGUJIAN KONKURENSI AMBATRAIN")
    print("="*50)
    print(f"Total Request Dikirim : 50")
    print(f"Berhasil (201 Created) : {success_count}")
    print(f"Konflik (409 Conflict) : {conflict_count}")
    print(f"Lainnya/Error          : {other_count}")

    if success_count == 1 and conflict_count == 49:
        print("\n✅ PENGUJIAN LULUS SEMPURNA: 0 Double Booking! Konsistensi transaksi C++ terbukti aman.")
        sys.exit(0)
    else:
        print("\n❌ PENGUJIAN GAGAL: Terjadi double booking atau inkonsistensi penanganan konkurensi!")
        sys.exit(1)

if __name__ == "__main__":
    main()

