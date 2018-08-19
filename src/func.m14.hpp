#ifndef funcm14hpp
#define funcm14hpp
#include "includes/io/io.hpp"
#include "M14Defs.hpp"

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

int xyz = 1;
int readPlates(string y);
 
int readPlates(string y)
{
int result;
goto readPlates__nodeindex__;
beginreadPlates:;
;
return result;
{

using namespace std;

    char *outText;

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image = pixRead("testplate.jpg");
    api->SetImage(image);
    // Get OCR result

//	string result =  api->GetUTF8Text();

    outText = api->GetUTF8Text();
    printf("OCR output:\n%s %i", outText, strlen(outText));

	//cout << result.size() << " : " << result << endl;
    
    
    
	
	
	Boxa* boxes = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
	printf("Found %d textline image components.\n", boxes->n);
	for (int i = 0; i < boxes->n; i++)
	{
		BOX* box = boxaGetBox(boxes, i, L_CLONE);
		api->SetRectangle(box->x, box->y, box->w, box->h);
		char* ocrResult = api->GetUTF8Text();
		int conf = api->MeanTextConf();
		fprintf(stdout, "Box[%d]: x=%d, y=%d, w=%d, h=%d, confidence: %d, text: %s",
						i, box->x, box->y, box->w, box->h, conf, ocrResult);
	}
	
	
	// Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);

};
goto endreadPlates;
readPlates__nodeindex__:;
node.dataVectorReady=true;
switch (node.nodeNumber)
{
default: goto beginreadPlates;break;
}
endreadPlates:;
return result;
};

#endif 