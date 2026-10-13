#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Aspar2 / Metin2 Mobile Icon Generator
Generates all required Android mipmap icons (square + circular round)
and iOS AppIcon variants from a single master 1024x1024 icon.

Source icon: android_ios/common/branding/icon_1024.png
"""

import os
import sys
from PIL import Image, ImageDraw

def find_project_root():
    current = os.path.dirname(os.path.abspath(__file__))
    while current and current != os.path.dirname(current):
        if os.path.exists(os.path.join(current, "android_ios")):
            return current
        current = os.path.dirname(current)
    # Default fallback: 3 levels up from extern/tools/IconGenerator
    return os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))

def create_circular_icon(img):
    """Creates a circular masked version of the icon with smooth edges."""
    size = img.size
    scale = 4
    mask_size = (size[0] * scale, size[1] * scale)
    mask = Image.new('L', mask_size, 0)
    draw = ImageDraw.Draw(mask)
    draw.ellipse((0, 0, mask_size[0] - 1, mask_size[1] - 1), fill=255)
    mask = mask.resize(size, Image.Resampling.LANCZOS)
    
    output = Image.new('RGBA', size, (0, 0, 0, 0))
    if img.mode != 'RGBA':
        img = img.convert('RGBA')
    output.paste(img, (0, 0))
    output.putalpha(mask)
    return output

def main():
    project_root = find_project_root()
    
    # Master source icon
    source_icon_path = os.path.join(project_root, "android_ios", "common", "branding", "icon_1024.png")
    if not os.path.exists(source_icon_path):
        source_icon_path = os.path.join(project_root, "android_ios", "ios", "AppIcon-1024.png")
        if not os.path.exists(source_icon_path):
            print(f"[HATA] Master icon bulunamadi: {source_icon_path}")
            print("Lutfen 1024x1024 boyutunda bir PNG dosyasini su yola koyun:")
            print("  android_ios/common/branding/icon_1024.png")
            sys.exit(1)

    print(f"Master icon yukleniyor: {source_icon_path}")
    master = Image.open(source_icon_path).convert("RGBA")
    if master.size != (1024, 1024):
        print(f"[UYARI] Icon boyutu 1024x1024 degil ({master.size[0]}x{master.size[1]}), 1024x1024'e olcekleniyor...")
        master = master.resize((1024, 1024), Image.Resampling.LANCZOS)

    # 1. Android Mipmap Sizes
    android_res_dir = os.path.join(project_root, "android_ios", "android", "app", "src", "main", "res")
    android_sizes = [
        (48, "mipmap-mdpi"),
        (72, "mipmap-hdpi"),
        (96, "mipmap-xhdpi"),
        (144, "mipmap-xxhdpi"),
        (192, "mipmap-xxxhdpi"),
    ]

    print("\n--- Android Ikonlari Uretiliyor ---")
    for size, folder in android_sizes:
        target_dir = os.path.join(android_res_dir, folder)
        os.makedirs(target_dir, exist_ok=True)
        
        square_img = master.resize((size, size), Image.Resampling.LANCZOS)
        square_path = os.path.join(target_dir, "ic_launcher.png")
        square_img.save(square_path, "PNG")
        
        round_img = create_circular_icon(square_img)
        round_path = os.path.join(target_dir, "ic_launcher_round.png")
        round_img.save(round_path, "PNG")
        
        print(f"  [OK] {folder}: ic_launcher.png & ic_launcher_round.png ({size}x{size})")

    # 2. iOS Icons
    ios_dir = os.path.join(project_root, "android_ios", "ios")
    ios_xcassets_dir = os.path.join(ios_dir, "Assets.xcassets", "AppIcon.appiconset")
    os.makedirs(ios_xcassets_dir, exist_ok=True)

    print("\n--- iOS Ikonlari Uretiliyor ---")
    master.save(os.path.join(ios_xcassets_dir, "AppIcon-1024.png"), "PNG")
    master.save(os.path.join(ios_dir, "AppIcon-1024.png"), "PNG")
    print("  [OK] AppIcon-1024.png (1024x1024)")

    ios_sizes = [
        (60, "AppIcon.png"),
        (120, "AppIcon@2x.png"),
        (180, "AppIcon@3x.png"),
        (120, "AppIcon-60@2x.png"),
        (152, "AppIcon-76@2x.png"),
        (60, "Icon-60.png"),
        (76, "Icon-76.png"),
    ]

    for size, filename in ios_sizes:
        target_path = os.path.join(ios_dir, filename)
        resized = master.resize((size, size), Image.Resampling.LANCZOS)
        resized.save(target_path, "PNG")
        print(f"  [OK] {filename} ({size}x{size})")

    print("\n[BASARILI] Tum Android ve iOS ikonlari basariyla olusturuldu!")

if __name__ == "__main__":
    main()
