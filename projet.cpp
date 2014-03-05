/*
 * projet.cpp - Code-Source de la version de base du projet Vision sur carte SABRE i.MX_6
 * R.Kachouri
 */

#include <stdio.h>
#include <stdlib.h>

#include "highgui.h"
#include "cv.h"
#include <math.h>
#include <iostream>

#define N_frames 100

#define addr_c(x,y,c) ((y)*step + (x)*channels + c)
#define addr(x,y) ((y)*width + (x))
#define value(x) ((x > 90) ? 255 : 0)

// Sobel1 : sans optimisation
// Average sobel filter duration: 178.184ms
uchar* sobel(int width, int height, uchar* data_in, uchar* data_out)
{
    int i,j;
    int tmp_v;
    int tmp_h;

    for (i=1; i<height-1;i++)
        for(j=1;j<width-1;j++){
            tmp_v =     -1*data_in[addr(j-1,i-1)] -2*data_in[addr(j-1,i)] -1*data_in[addr(j-1,i+1)]
                +1*data_in[addr(j+1,i-1)] +2*data_in[addr(j+1,i)] +1*data_in[addr(j+1,i+1)];
            tmp_h =     -1*data_in[addr(j-1,i-1)] -2*data_in[addr(j,i-1)] -1*data_in[addr(j+1,i-1)]
                +1*data_in[addr(j-1,i+1)] +2*data_in[addr(j,i+1)] +1*data_in[addr(j+1,i+1)];

            data_out[addr(j,i)] = value( sqrt( pow(tmp_h,2) + pow(tmp_v,2) ) );
        }
    return data_out;
}
// Sobel2 : Approx sqrt -> abs
// Average sobel filter duration: 57.1648ms
uchar* sobel2(int width, int height, uchar* data_in, uchar* data_out)
{
    int i,j;
    int tmp_v;
    int tmp_h;

    for (i=1; i<height-1;i++)
        for(j=1;j<width-1;j++){
            tmp_v =     -1*data_in[addr(j-1,i-1)] -2*data_in[addr(j-1,i)] -1*data_in[addr(j-1,i+1)]
                +1*data_in[addr(j+1,i-1)] +2*data_in[addr(j+1,i)] +1*data_in[addr(j+1,i+1)];
            tmp_h =     -1*data_in[addr(j-1,i-1)] -2*data_in[addr(j,i-1)] -1*data_in[addr(j+1,i-1)]
                +1*data_in[addr(j-1,i+1)] +2*data_in[addr(j,i+1)] +1*data_in[addr(j+1,i+1)];

            data_out[addr(j,i)] = value( abs(tmp_h) + abs(tmp_v) );
        }
    return data_out;
}
// Sobel 3 : 4 acces memoire en moins par iteration
// Average sobel filter duration: 47.6487ms
uchar* sobel3(int width, int height, uchar* data_in, uchar* data_out)
{
    int i,j;
    int tmp_v;
    int tmp_h;
    uchar a,b,c,d;
 

    for (i=1; i<height-1;i++)
        for(j=1;j<width-1;j++){
			a = data_in[addr(j-1,i-1)];
			b = data_in[addr(j+1,i-1)];
			c = data_in[addr(j-1,i+1)];
			d = data_in[addr(j+1,i+1)];

            tmp_v =     - a -2*data_in[addr(j-1,i)] - c + b +2*data_in[addr(j+1,i)] + d;
            tmp_h =     - a -2*data_in[addr(j,i-1)] - b + c +2*data_in[addr(j,i+1)] + d;

            data_out[addr(j,i)] = value( abs(tmp_h) + abs(tmp_v) );
        }
    return data_out;
}
// Sobel 4 : Prog dynamique
// 
uchar* sobel4(int width, int height, uchar* data_in, uchar* data_out)
{
    int i,j;
    int tmp_v;
    int tmp_h;
    uchar a,b,c,d;
 	uchar h1,h2,v1,v2,tmp;
	
	
	
	for (i=1; i<height-1;i++){
		a = data_in[addr(0,i-1)];
		c = data_in[addr(0,i+1)];
    	h1 = data_in[addr(1,i-1)];
		h2 = data_in[addr(1,i+1)];
		v1 = data_in[addr(0,i)];
		tmp = data_in[addr(1,i)];
        
		for(j=1;j<width-1;j++){
			b = data_in[addr(j+1,i-1)];
			v2 = data_in[addr(j+1,i)];
			d = data_in[addr(j+1,i+1)];
            // Sobel
			tmp_v = - a -2*v1 - c + b +2*v2 + d;
            tmp_h = - a -2*h1 - b + c +2*h2 + d;
            data_out[addr(j,i)] = value( abs(tmp_h) + abs(tmp_v) );
			//update valeurs
			a = h1; c = h2;
			h1 = b; h2 = d;
			tmp = v2; v1 = tmp;

        }
	}
    return data_out;
}
// sort_median : optimisation tri diagonal
// Average median filter duration: 319.333ms
uchar sort_median(uchar data[9]){
    int i,j;
    uchar tmp;
    for(i=0; i<9;i=i+3) {
        if (data[i]>data[i+1])
            std::swap(data[i],data[i+1]);
        if (data[i+1]>data[i+2])
            std::swap(data[i],data[i+1]);
        if (data[i]>data[i+1])
            std::swap(data[i],data[i+1]);
    }
    for(i=0; i<3;i++) {
        if (data[i]>data[i+3])
            std::swap(data[i],data[i+3]);
        if (data[i+3]>data[i+6])
            std::swap(data[i+3],data[i+6]);
        if (data[i]>data[i+3])
            std::swap(data[i],data[i+3]);
    }
    
    if (data[2]>data[4])
            std::swap(data[2],data[4]);
    if (data[4]>data[6])
            std::swap(data[4],data[6]);
    if (data[2]>data[4])
            std::swap(data[2],data[4]);   

    return data[4];
}


// Median 1 : Sans optimisation
// Average median filter duration: 319.333ms
uchar* median_filter(int width, int height, uchar* in, uchar* out)
{
    uchar tmp[9];
    int i,j,k,l,m;
    for (i=1; i<height-1;i++)
        for(j=1;j<width-1;j++)
        {
            m = 0;
            for(k=-1; k<2; k++)
                for(l=-1;l<2;l++,m++){
                    tmp[m] = in[addr(j+k,i+l)];
                }
            out[addr(j,i)] = sort_median(tmp);
        }
    return out;
}
// Median 2 : Prog dynamique
// Average median filter duration: 229.277ms
uchar* median_filter2(int width, int height, uchar* in, uchar* out)
{
    uchar t[9];
    int i,j;
    for (i=1; i<height-1;i++){
		t[0]=in[addr(0,i-1)]; t[1]=in[addr(1,i-1)]; t[2]=in[addr(2,i-1)];
		t[3]=in[addr(0, i )]; t[4]=in[addr(1, i )]; t[5]=in[addr(2, i )];
		t[6]=in[addr(0,i+1)]; t[7]=in[addr(1,i+1)]; t[8]=in[addr(2,i+1)];

        for(j=1;j<width-1;j++){
			t[2] = in[addr(j+1,i-1)];
			t[5] = in[addr(j+1, i )];
			t[8] = in[addr(j+1,i+1)];

       		out[addr(j,i)] = sort_median(t);
			t[0]=t[1]; t[1]=t[2]; 
			t[3]=t[4]; t[4]=t[5]; 
			t[6]=t[7]; t[7]=t[8]; 

        }
	}
    return out;
}




int main() {

    int count_frames = 0;
    int i,j;

    // Dimensions et propriétés de l'image
    int size, height, width, step, depth, channels;

    // Déclaration des images
    IplImage *im_in; // image d'entrée = trame de flux vidéo
    IplImage *im_1;  // image de résulat intermédiaire - conversion en niveau de gris
    IplImage *im_2; // résultat de Sobel
    IplImage *im_3;
    IplImage *im_4;

    // Pointeurs vers les données de l'image
    uchar *im_in_data;
    uchar *im_1_data;
    uchar *im_2_data;
    uchar *im_3_data;
    uchar *im_4_data;

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

    im_1 = cvCreateImage(cvSize(width,height),  IPL_DEPTH_8U, 1);     // on crée une image en niveau de gris;
    im_2 = cvCreateImage(cvSize(width,height),  IPL_DEPTH_8U, 1);     // on crée une image en niveau de gris;
    im_3 = cvCreateImage(cvSize(width,height),  IPL_DEPTH_8U, 1);     // on crée une image en niveau de gris;
    im_4 = cvCreateImage(cvSize(width,height),  IPL_DEPTH_8U, 1);     // on crée une image en niveau de gris;
    im_1_data =(uchar *) im_1->imageData;
    im_2_data = (uchar *) im_2->imageData;
    im_3_data = (uchar *) im_3->imageData;
    im_4_data = (uchar *) im_4->imageData;

    double sobel_time = 0.0;
    double median_time = 0.0;

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
                im_1_data[addr(j,i)] =   //  0.114*im_in_data[addr_c(j,i,0)] +
                   /* 0.587* */im_in_data[addr_c(j,i,1)]; // +
                   // 0.299*im_in_data[addr_c(j,i,2)];
            }

        int64 median_start = cv::getTickCount();
        im_2_data = median_filter2(width, height, im_1_data, im_2_data); // Median
        median_time += (double)(cv::getTickCount() - median_start) / cv::getTickFrequency();


        int64 sobel_start = cv::getTickCount();
        im_4_data = sobel4(width, height, im_2_data, im_4_data); // Median + Sobel
        sobel_time += (double)(cv::getTickCount() - sobel_start) / cv::getTickFrequency();
        /********************************************************/

        /********************************************************/

         
       	cvShowImage( "Noir & Blanc", im_1);
       	cvShowImage( "Median", im_2);
		cvShowImage( "Median & Sobel", im_4);

        cvWaitKey(10);

        // Incrémentation
        count_frames++;

    }

    median_time /= N_frames;
    sobel_time /= N_frames;
    std::cout << "Average median filter duration: " << 1000.0 * median_time << "ms" << std::endl;
    std::cout << "Average sobel filter duration: " << 1000.0 * sobel_time << "ms" << std::endl;

    // Libérer espace mémoire
    cvReleaseCapture(&capture);
    cvDestroyWindow("Input");
    cvDestroyWindow("Output");
    cvDestroyWindow("Sobel");
    cvReleaseImage(&im_in);
    cvReleaseImage(&im_1);
    cvReleaseImage(&im_2);

    return 0;
}
