# OCR toolkit
Simple pet project to learn something about [OCR|https://en.wikipedia.org/wiki/Optical_character_recognition] and some practice in C programming language

# Structure
In this project OCR was devided on three major steps:
* Preprocing - prepare input image to be recognized, apllying filters, [morphology | https://en.wikipedia.org/wiki/Mathematical_morphology] etc.
* Segmentation - finding areas which contains text
* Recognition - recognize text in found area
* Correction - recognition errors correction

For every step was writen separate library respectavly:
* ocr_imgproc
* ocr_segm
* ocr_recog
* ocr_correct

# Install
Just run `make` to build o-files
