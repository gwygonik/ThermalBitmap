# ThermalBitmap
Code to create bitmaps row-by-row on an Arduino for output on a thermal printer

This particular example uses Perlin noise to create a pleasing design across the print.

The core function to pay attention to is the setBitPixel() method:

```
void setBitPixel(uint8_t *pxlBytes, int pxlIndex, short width) {
  for (int i=1;i<=width;i++) {
    int tmpIndex = pxlIndex - (floor(width/2)) + i; // center the width around desired pixel (use odd numbers)
    int byteIndex = floor(tmpIndex/8); // e.g.: 33 / 8 = 4.125 -> floor 4
    int bitIndex = floor(tmpIndex%8); // 3.g.: 33 % 8 = 1

    // clamp to left/right bounds
    if (byteIndex < 0) byteIndex = 0;
    if (bitIndex < 0) bitIndex = 0;
    if (byteIndex > 383) byteIndex = 383;
    if (bitIndex > 7) bitIndex = 7;

    // finally, set the appropriate bit in the byte
    bitSet(pxlBytes[byteIndex], 7-bitIndex);
  }
}
```

This will let you work in absolute pixel values (0-383 across a row) as it will automatically determine the correct byte and bit to set, along with dealing with pixel width.

Running this on an Arduino UNO hooked into a stock, unmodified Adafruit thermal printer should yield a print like this:

