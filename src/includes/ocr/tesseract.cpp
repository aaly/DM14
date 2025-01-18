// Copyright (c) 2015, <Abdallah Aly> <aaly90@gmail.com>
//
// Part of Distributed Mission14 programming language
//
// See file "license" for license

#include "tesseract.hpp"

Tesseract::Tesseract() { api = new tesseract::TessBaseAPI(); }

int Tesseract::setLanguage(const string &lang) {
  // Initialize tesseract-ocr with English, without specifying tessdata path
  if (api->Init(NULL, lang.c_str())) {
    return 0;
  }

  return 0;
}

string Tesseract::readImage(const string &path) {
  // Open input image with leptonica library
  Pix *image = pixRead(path.c_str());

  if (!image) {
    cout << "ERRORRRRRRRRRRRRR" << endl;
    return "";
  }

  api->SetImage(image);

  // Get OCR result
  char *result = api->GetUTF8Text();

  if (strlen(result)) {
    string ret(result);
    free(result);
    return ret;
  } else {
    return "";
  }

  // outText = api->GetUTF8Text();
  // printf("OCR output:\n%s %i", outText, strlen(outText));

  Boxa *boxes =
      api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
  printf("Found %d textline image components.\n", boxes->n);
  for (int i = 0; i < boxes->n; i++) {
    BOX *box = boxaGetBox(boxes, i, L_CLONE);
    api->SetRectangle(box->x, box->y, box->w, box->h);
    char *ocrResult = api->GetUTF8Text();
    int conf = api->MeanTextConf();
    fprintf(stdout, "Box[%d]: x=%d, y=%d, w=%d, h=%d, confidence: %d, text: %s",
            i, box->x, box->y, box->w, box->h, conf, ocrResult);
  }

  // Destroy used object and release memory
  pixDestroy(&image);
  delete image;
}
Tesseract::~Tesseract() {
  api->End();
  delete api;
}
