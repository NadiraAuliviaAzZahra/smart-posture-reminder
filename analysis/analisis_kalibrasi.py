#!/usr/bin/env python3
"""Analisis kalibrasi SORE berbasis CSV hasil pengukuran nyata.

Kolom wajib: axis,reference_deg,mpu_deg,repeat.
Program ini tidak membuat data contoh atau hasil simulasi.
"""
import csv
import math
import statistics
import sys
from collections import defaultdict
from pathlib import Path

HEADER = {"axis", "reference_deg", "mpu_deg", "repeat"}


def baca_data(path):
    kelompok = defaultdict(list)
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        pembaca = csv.DictReader(f)
        if not HEADER.issubset(set(pembaca.fieldnames or [])):
            raise ValueError(f"Kolom wajib: {', '.join(sorted(HEADER))}")
        for baris_ke, row in enumerate(pembaca, start=2):
            try:
                axis = row["axis"].strip().lower()
                if axis not in {"pitch", "roll"}:
                    raise ValueError("axis harus pitch atau roll")
                ref = float(row["reference_deg"])
                mpu = float(row["mpu_deg"])
                if not (math.isfinite(ref) and math.isfinite(mpu)):
                    raise ValueError("nilai sudut harus bilangan terhingga")
                ulangan = int(row["repeat"])
                if ulangan < 1:
                    raise ValueError("repeat harus >= 1")
                kelompok[axis].append(mpu - ref)
            except (ValueError, TypeError, KeyError) as e:
                raise ValueError(f"Baris {baris_ke}: {e}") from e
    if not kelompok:
        raise ValueError("Tidak ada data pengukuran aktual dalam CSV.")
    return kelompok


def tampilkan(kelompok):
    for axis in ("pitch", "roll"):
        errors = kelompok.get(axis, [])
        if not errors:
            print(f"{axis}: belum tersedia data; tidak dihitung.")
            continue
        n = len(errors)
        mae = sum(abs(e) for e in errors) / n
        rmse = math.sqrt(sum(e * e for e in errors) / n)
        sd = statistics.stdev(errors) if n >= 2 else float("nan")
        galat_terbesar = max(abs(e) for e in errors)
        print(f"{axis.upper()} | n={n} | MAE={mae:.3f}° | RMSE={rmse:.3f}° | "
              f"SD error={sd:.3f}° | Maksimum absolut={galat_terbesar:.3f}°")
        print("  Target maksimum ≤2°:", "TERPENUHI" if galat_terbesar <= 2 else "BELUM TERPENUHI")
        print("  Target SD error <0,5°:",
              "TERPENUHI" if n >= 2 and sd < 0.5 else "BELUM DAPAT DIPASTIKAN / TIDAK TERPENUHI")
    print("Catatan: keterpenuhan target lain (nol, respons getar, stabilitas) harus diuji terpisah.")


def main():
    if len(sys.argv) != 2:
        print("Cara pakai: python analysis/analisis_kalibrasi.py data/hasil_kalibrasi.csv")
        return 2
    path = Path(sys.argv[1])
    if not path.is_file():
        print(f"File tidak ditemukan: {path}", file=sys.stderr)
        return 2
    try:
        tampilkan(baca_data(path))
    except ValueError as error:
        print(f"Data tidak valid: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
