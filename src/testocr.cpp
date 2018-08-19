 
#include<tesseract/baseapi.h>
#include<leptonica/allheaders.h>
#include<string>
#include<iostream>
usingnamespacestd;intmain(){char*outText;tesseract::TessBaseAPI*api=newtesseract::TessBaseAPI();//Initializetesseract-ocrwithEnglish,withoutspecifyingtessdatapathif(api->Init(NULL,"eng")){fprintf(stderr,"Could 
not initialize 
tesseract.\n");exit(1);}//OpeninputimagewithleptonicalibraryPix*image=pixRead("testplate.jpg");api->SetImage(image);//GetOCRresult//stringresult=api->GetUTF8Text();outText=api->GetUTF8Text();printf("OCR 
output:\n%s %i",outText,strlen(outText));//cout<<result.size()<<" : 
"<<result<<endl;Boxa*boxes=api->GetComponentImages(tesseract::RIL_TEXTLINE,true,NULL,NULL);printf("Found 
%d textline image 
components.\n",boxes->n);for(inti=0;i<boxes->n;i++){BOX*box=boxaGetBox(boxes,i,L_CLONE);api->SetRectangle(box->x,box->y,box->w,box->h);char*ocrResult=api->GetUTF8Text();intconf=api->MeanTextConf();fprintf(stdout,"Box[%d]: 
x=%d, y=%d, w=%d, h=%d, confidence: %d, text: 
%s",i,box->x,box->y,box->w,box->h,conf,ocrResult);}//Destroyusedobjectandreleasememoryapi->End();delete[]outText;pixDestroy(&image);return0;}
