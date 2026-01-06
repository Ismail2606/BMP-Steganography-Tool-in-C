#  BMP Image Steganography (C)

A **C-based steganography project** that hides a secret text file inside a **24-bit BMP image** using **Least Significant Bit (LSB)** encoding and can also extract the hidden data back.

---

## Project Overview

Steganography is the technique of hiding secret information inside another file such that its presence is not noticeable.
In this project, a **text file (.txt)** is hidden inside a **BMP image** without visibly altering the image.

The project supports:

* Encoding secret data into a BMP image
* Decoding secret data from the BMP image
* Capacity checking before encoding

---

## Features

* Supports **24-bit BMP images**
* LSB-based encoding technique
* Hides:

  * Magic string (for validation)
  * Secret file extension
  * Secret file size
  * Secret file data
* Capacity validation before encoding
* Error handling for invalid inputs
* Modular and readable C code

---

## Concepts Used

* File handling (`fopen`, `fread`, `fwrite`)
* Bitwise operations
* Pointers and structures
* Command-line arguments
* Binary file processing
* Modular programming

---


##  How It Works (LSB Technique)

* Each byte of secret data is broken into **8 bits**
* Each bit is stored in the **LSB of image bytes**
* Since LSB changes are visually unnoticeable, image quality remains intact

---

##  How to Run

###  Encoding

```bash
./stego -e source.bmp secret.txt output.bmp
```

###  Decoding

```bash
./stego -d output.bmp extracted.txt
```

---

##  Encoding Flow

1. Validate command-line arguments
2. Read BMP header
3. Check image capacity
4. Encode magic string
5. Encode secret file extension
6. Encode secret file size
7. Encode secret file data
8. Copy remaining image data

---

##  Decoding Flow

1. Validate magic string
2. Extract secret file extension
3. Extract secret file size
4. Decode secret file data
5. Write extracted file

---

##  Author

**Ismail Pasha**
Embedded Systems & C Programmer

---


⭐ If you find this project useful, don’t forget to star the repository!
