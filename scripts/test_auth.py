"""
AmbaTrain Phase 2 Automated Auth & Master Data Test Script
Menguji alur registrasi, login JWT, proteksi AuthGuard /api/auth/me, dan /api/stations.
"""

import urllib.request
import urllib.error
import json
import time
import random
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
    print("  AMBATRAIN — PEMERIKSAAN FASE 2 (AUTH & MASTER DATA)")
    print("=" * 60)

    # 1. Healthcheck
    status, res = make_request("/health")
    if status != 200:
        print(f"❌ Server belum berjalan di {API_BASE}. Pastikan binary telah dijalankan.")
        print("   (Jalankan server terlebih dahulu: ./ambatrain)")
        sys.exit(1)
    print(f"✅ Server Crow C++ Berjalan: {res.get('service')} - Status: {res.get('status')}")

    # 2. Register New User
    unique_id = int(time.time() * 1000) % 100000
    test_user = {
        "name": f"Knight Galahad #{unique_id}",
        "email": f"galahad_{unique_id}@ambatrain.com",
        "password": "OathOfGalahad123!"
    }

    print(f"\n[1/4] Menguji Registrasi: {test_user['email']}...")
    status, res = make_request("/auth/register", method="POST", data=test_user)
    if status == 201:
        print(f"  ✅ Registrasi Berhasil! Token JWT diterima.")
        token = res.get("token")
    else:
        print(f"  ❌ Registrasi Gagal (HTTP {status}): {res}")
        sys.exit(1)

    # 3. Login
    print(f"\n[2/4] Menguji Login...")
    login_data = {"email": test_user["email"], "password": test_user["password"]}
    status, res = make_request("/auth/login", method="POST", data=login_data)
    if status == 200:
        print(f"  ✅ Login Berhasil! Token valid.")
        token = res.get("token")
    else:
        print(f"  ❌ Login Gagal: {res}")
        sys.exit(1)

    # 4. Get Profile (Protected)
    print(f"\n[3/4] Menguji AuthGuard (GET /api/auth/me)...")
    status, res = make_request("/auth/me", method="GET", token=token)
    if status == 200:
        print(f"  ✅ Profil berhasil divalidasi: Nama '{res.get('name')}', Role '{res.get('role')}'")
    else:
        print(f"  ❌ Akses profil gagal: {res}")
        sys.exit(1)

    # 5. Stations Master Data
    print(f"\n[4/4] Menguji Master Data Stasiun (GET /api/stations)...")
    status, res = make_request("/stations", method="GET")
    if status == 200:
        print(f"  ✅ Daftar stasiun diterima: Total {len(res)} stasiun tercatat.")
    else:
        print(f"  ❌ Gagal memuat stasiun: {res}")
        sys.exit(1)

    print("\n" + "=" * 60)
    print("🎉 SEMUA UJI FASE 2 BERHASIL SEMPURNA!")
    print("=" * 60)

if __name__ == "__main__":
    run_tests()

