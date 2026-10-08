import os
import struct
import subprocess
import time
import sys

script_dir = os.path.dirname(os.path.abspath(__file__))
converter = os.path.join(script_dir, "convert_gr2.exe")
data_pack_dir = r"D:\01NewFiles\Data_Pack"

if len(sys.argv) > 1:
    data_pack_dir = sys.argv[1]

print("==================================================")
print(f"  Granny BitKnit -> Oodle1 Converter")
print(f"  Target: {data_pack_dir}")
print("==================================================")

start_time = time.time()
scanned = 0
found = []

print("Scanning for BitKnit / BitKnit2 files...")

for root, dirs, files in os.walk(data_pack_dir):
    for f in files:
        if f.lower().endswith(".gr2"):
            scanned += 1
            fp = os.path.join(root, f)
            try:
                with open(fp, "rb") as fp_in:
                    data = fp_in.read(128)
                if len(data) >= 72:
                    sec_off = struct.unpack("<I", data[44:48])[0]
                    sec_cnt = struct.unpack("<I", data[48:52])[0]
                    with open(fp, "rb") as fp_full:
                        fp_full.seek(32 + sec_off)
                        sec_bytes = fp_full.read(44 * sec_cnt)
                    has_bk = False
                    for i in range(sec_cnt):
                        fmt = struct.unpack("<I", sec_bytes[i*44 : i*44+4])[0]
                        if fmt in (3, 4):
                            has_bk = True
                            break
                    if has_bk:
                        found.append(fp)
            except Exception:
                pass

print(f"Scanned {scanned} GR2 files.")
if not found:
    print("Harika! Tum dosyalar zaten uyumlu. Donusturulecek BitKnit dosyasi bulunamadi.")
    sys.exit(0)

print(f"Donusturulecek {len(found)} adet BitKnit dosyasi bulundu. Donusturuluyor...")

converted = 0
failed = 0

for i, fp in enumerate(found, 1):
    res = subprocess.run([converter, "oodle1", fp, fp], capture_output=True, text=True)
    if res.returncode == 0:
        converted += 1
    else:
        failed += 1
        print(f"HATA [{i}/{len(found)}]: {fp}")

elapsed = time.time() - start_time
print(f"\nIslem Tamamlandi! ({elapsed:.2f} saniye)")
print(f"Basariyla Donusturulen: {converted}")
print(f"Basarisiz: {failed}")
