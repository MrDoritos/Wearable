#!/bin/python3

import freetype
from PIL import Image, ImageDraw
import sys, os

# --- Configuration ---
FONT_PATH = "arial.ttf"  # IMPORTANT: Replace with the actual path to your .ttf font file
FONT_WIDTH = 8
FONT_HEIGHT = 0
#CHARACTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{};':\",./<>?|\\"
#CHARACTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#CHARACTERS = ''.join([chr(x) for x in range(ord('a'), ord('z')+1)])
CHARACTERS = ''.join([chr(x) for x in range(0,256)])
ATLAS_PADDING = 0       # Padding between characters in the atlas

def create_font_atlas(font_path, font_width, font_height, characters, padding=2):
    """
    Creates a font atlas texture from a given font and set of characters.

    Args:
        font_path (str): Path to the TrueType font file (.ttf).
        font_size (int): The desired font size in pixels.
        characters (str): A string containing all characters to include in the atlas.
        padding (int): Padding in pixels between each character in the atlas.

    Returns:
        tuple: A tuple containing:
            - PIL.Image: The generated font atlas image.
            - dict: A dictionary where keys are characters and values are dictionaries
                    containing 'uv' (normalized texture coordinates), 'width', 'height',
                    'advance_x', 'bearing_x', and 'bearing_y' for each character.
    """
    try:
        face = freetype.Face(font_path)
        face.set_pixel_sizes(font_width, font_height)
        #face.set_char_size(font_width, font_height)
    except Exception as e:
        print(f"Error loading font or setting size: {e}")
        print(f"Please ensure '{font_path}' is a valid font file and freetype-py is installed.")
        return None, None

    char_metrics = {}
    max_char_width = 0
    max_char_height = 0
    total_width_needed = 0
    num_chars = len(characters)

    # First pass: Calculate dimensions for the atlas
    # We'll use a simple row-by-row packing strategy for this example.
    # A more advanced packer could optimize space better.
    for char_code in characters:
        face.load_char(char_code, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_LIGHT)
        glyph = face.glyph

        char_width = glyph.bitmap.width
        char_height = glyph.bitmap.rows

        max_char_width = max(max_char_width, char_width)
        max_char_height = max(max_char_height, char_height)

        total_width_needed += (char_width + padding)

    # Estimate atlas dimensions (simple row layout)
    # For a more robust solution, consider a bin-packing algorithm.
    atlas_width = total_width_needed + padding
    atlas_height = max_char_height + 2 * padding # Add padding top/bottom
    atlas_dim=128
    max_char_dim = max(max_char_height, max_char_width)
    atlas_area = (max_char_dim ** 2) * num_chars
    while True:
        if atlas_dim ** 2 >= atlas_area:
            break
        atlas_dim *= 2

    print(f'atlas size {atlas_dim}x{atlas_dim}')
    print(f'box_size {max_char_width}x{max_char_height}')
    atlas_width=atlas_height=atlas_dim


    # Create a new RGBA image (with alpha channel for transparency)
    atlas_image = Image.new('RGBA', (atlas_width, atlas_height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(atlas_image)

    current_x = padding
    current_y = padding

    # Second pass: Render characters to the atlas and store metrics
    for char_code in characters:
        face.load_char(char_code, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_LIGHT)
        glyph = face.glyph
        bitmap = glyph.bitmap

        # Convert freetype bitmap to PIL Image
        # Freetype bitmaps are typically grayscale (luminance), so we need to convert to RGBA
        # The `FT_LOAD_TARGET_LIGHT` flag often results in anti-aliased glyphs.
        # We'll treat the bitmap as an alpha channel.
        if bitmap.width > 0 and bitmap.rows > 0 and (bitmap.width * bitmap.rows) <= len(bitmap.buffer):
            # Create a new PIL image from the freetype bitmap data
            # Freetype bitmap data is usually a flat byte array
            # We'll create a single-channel 'L' (luminance) image first
            print(bitmap.width, bitmap.rows, len(bitmap.buffer))
            char_img = Image.frombytes(
                'L',
                (bitmap.width, bitmap.rows),
                bytes(bitmap.buffer) + bytes([0])
            )
            # Convert 'L' to 'RGBA' so we can paste it onto the atlas with transparency
            # The 'L' channel becomes the alpha channel, and color is set to white (255,255,255)
            WHITE=(255,255,255,0)
            BLACK=(0,0,0,255)
            char_img_rgba = Image.new('RGBA', char_img.size, WHITE)
            char_img_rgba.putalpha(char_img)
        else:
            # Handle empty glyphs (e.g., space character)
            char_img_rgba = Image.new('RGBA', (0, 0), (0, 0, 0, 0))

        char_width,char_height=char_img_rgba.size
        bearing_x,bearing_y=glyph.bitmap_left,glyph.bitmap_top

        #offset_x = current_x + max_char_width - char_width + glyph.bitmap_left
        #offset_y = current_y + max_char_height - char_height + glyph.bitmap_top
        #offset_x = current_x + (max_char_width - (char_width - bearing_x))
        #offset_y = current_y + (max_char_height - (char_height - bearing_y))
        #offset_x = current_x + bearing_x
        #offset_y = current_y + bearing_y - char_height
        #offset_x = (current_x - bearing_x) + (max_char_width - (char_width))
        offset_x = current_x + bearing_x
        offset_y = current_y + (max_char_height - bearing_y) - 1

        # Paste the character onto the atlas
        if char_img_rgba.size[0] > 0 and char_img_rgba.size[1] > 0:
            atlas_image.paste(char_img_rgba, (offset_x, offset_y), char_img_rgba)

        # Store metrics
        # UV coordinates are normalized (0.0 to 1.0)
        uv_left = current_x / atlas_width
        uv_top = current_y / atlas_height
        uv_right = (current_x + char_img_rgba.size[0]) / atlas_width
        uv_bottom = (current_y + char_img_rgba.size[1]) / atlas_height

        char_metrics[char_code] = {
            'uv': (uv_left, uv_top, uv_right, uv_bottom),
            'width': char_img_rgba.size[0],
            'height': char_img_rgba.size[1],
            'advance_x': glyph.advance.x / 64.0,  # Convert from 1/64th pixel units to pixels
            'advance_y': glyph.advance.y / 64.0,
            'bearing_x': glyph.bitmap_left,
            'bearing_y': glyph.bitmap_top,
        }

        # Move to the next position for the next character
        current_x += max_char_width + padding
        if current_x + max_char_width > atlas_width:
            current_x = 0
            current_y += max_char_height

    return atlas_image, char_metrics

# --- Main execution ---
if __name__ == "__main__":
    argc = len(sys.argv)

    if argc < 3:
        print('pack.py (font file) (font size)')
        sys.exit(1)

    FONT_PATH,FONT_WIDTH=os.path.expanduser(sys.argv[1]),int(sys.argv[2])
    FONT_HEIGHT=FONT_WIDTH
    if argc > 3:
        FONT_HEIGHT=int(sys.argv[3])

    atlas, metrics = create_font_atlas(FONT_PATH, FONT_WIDTH, FONT_HEIGHT, CHARACTERS, ATLAS_PADDING)

    if atlas and metrics:
        output_filename = os.path.basename(FONT_PATH) + f'_{FONT_WIDTH}x{FONT_HEIGHT}.png'
        atlas.save(output_filename)
        print(f"\nFont atlas saved to '{output_filename}'")
        print("\nCharacter Metrics (UVs, dimensions, advance, bearing):")
        for char, data in metrics.items():
            print(f"  '{char}': {data}")
    else:
        print("\nFailed to create font atlas. Please check the font path and error messages above.")

