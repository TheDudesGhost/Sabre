/*
 * projet.cpp - Code-Source de la version de base du projet Vision sur carte SABRE i.MX_6
 * R.Kachouri
 */
 
#include <stdio.h>
#include <stdlib.h>

#include "highgui.h"
#include "cv.h"

#define N_frames 500

#define addr_c(x,y,c) ((y)*step + (x)*channels + c)
#define addr(x,y) ((y)*width + (x))
 
int main() {
 
  int count_frames = 0;
  int i,j;
  
  // Dimensions et propriétés de l'image
  int size, height, width, step, depth, channels;

  // Déclaration des images 
  IplImage *im_in; // image d'entrée = trame de flux vidéo
  IplImage *im_1;  // image de résulat intermédiaire - conversion en niveau de gris
  
    // Pointeurs vers les données de l'image
    uchar *im_in_data;
    uchar *im_1_data;
  
    // Capture vidéo
    CvCapture *capture;
 
    // Ouvrir le flux vidéo
    capture = cvCreateCameraCapture(CV_CAP_ANY);
 
    // Ouverture de flux vidéo OK ?
    if (!capture) {
      printf("Problème d'ouverture du flux vidéo !!! \n");
      return 1;
    }
 
    // Création des fenêtres
    cvNamedWindow("Input", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Output", CV_WINDOW_AUTOSIZE);
	 
    // Positionnement des fenêtres
    cvMoveWindow("Input", 0,0);
    cvMoveWindow("Output", 0,700);
 
    // Lecture de la première trame
    im_in = cvQueryFrame(capture);
    
    // Initialisation des propriétés de l'image
    // size      = im_in->imageSize;
    height    = im_in->height;
    width     = im_in->width;
    step      = im_in->widthStep; // distance entre deux pixels
    channels  = im_in->nChannels;
	 im_in_data = (uchar *) im_in->imageData;
	 
    im_1 = cvCreateImage(cvSize(width,height),  IPL_DEPTH_8U, 1); 	// on crée une image en niveau de gris;
    im_1_data =(uchar *) im_1->imageData;
	 
    // Boucle de N_frames trames
    while(count_frames < N_frames) {
 
      // lecture d'une image
      im_in = cvQueryFrame(capture);
		 
      /********************************************************/
      /*               Chaîne de traitement                   */
      /********************************************************/
      // exemple de conversion RGB en niveau de gris
      for( i=0;i<height;i++) 
			for (j=0;j<width;j++)
			  {
				 // Gris = 0.3*R + 0.59*G + 0.11*B;
				 im_1_data[addr(j,i)] = 0.114*im_in_data[addr_c(j,i,0)] + 0.587*im_in_data[addr_c(j,i,1)] + 0.299*im_in_data[addr_c(j,i,2)];
			  } 

      /********************************************************/
     
      /********************************************************/
		 
		 
      
		 
       cvShowImage( "Input", im_in);
       cvShowImage( "Output", im_1);
       
       cvWaitKey(10);
 
       // Incrémentation
       count_frames++;
       
    }
	 
    // Libérer espace mémoire
    cvReleaseCapture(&capture);
    cvDestroyWindow("Input");
    cvDestroyWindow("Output");
    cvReleaseImage(&im_in);
    cvReleaseImage(&im_1);
 
    return 0;
 
}
