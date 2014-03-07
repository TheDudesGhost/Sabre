/*
 * projet.cpp - Code-Source de la version de base du projet Vision sur carte SABRE i.MX_6
 * R.Kachouri
 */

#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <atomic>
#include <future>

#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <math.h>
#include <iostream>

#define N_frames 100

#define addr_c(x,y,c) ((y)*step + (x)*channels + c)
#define addr(x,y) ((y)*width + (x))
#define value(x) ((x > 90) ? 255 : 0)

// Sobel1 : sans optimisation
// Average sobel filter duration: 178.184ms
void sobel(int width, int height, uchar* data_in, uchar* data_out, int y_start, int y_stop)
{
    int i,j;
    int tmp_v;
    int tmp_h;

    for (i=y_start; i<y_stop;i++)
        for(j=1;j<width-1;j++){
            tmp_v =     -1*data_in[addr(j-1,i-1)] -2*data_in[addr(j-1,i)] -1*data_in[addr(j-1,i+1)]
                +1*data_in[addr(j+1,i-1)] +2*data_in[addr(j+1,i)] +1*data_in[addr(j+1,i+1)];
            tmp_h =     -1*data_in[addr(j-1,i-1)] -2*data_in[addr(j,i-1)] -1*data_in[addr(j+1,i-1)]
                +1*data_in[addr(j-1,i+1)] +2*data_in[addr(j,i+1)] +1*data_in[addr(j+1,i+1)];

            data_out[addr(j,i)] = value( sqrt( pow(tmp_h,2) + pow(tmp_v,2) ) );
        }
}
// Sobel2 : Approx sqrt -> abs
// Average sobel filter duration: 57.1648ms
void sobel2(int width, int height, uchar* data_in, uchar* data_out, int y_start, int y_stop)
{
    int i,j;
    int tmp_v;
    int tmp_h;

    for (i=y_start; i<y_stop;i++)
        for(j=1;j<width-1;j++){
            tmp_v =     -1*data_in[addr(j-1,i-1)] -2*data_in[addr(j-1,i)] -1*data_in[addr(j-1,i+1)]
                +1*data_in[addr(j+1,i-1)] +2*data_in[addr(j+1,i)] +1*data_in[addr(j+1,i+1)];
            tmp_h =     -1*data_in[addr(j-1,i-1)] -2*data_in[addr(j,i-1)] -1*data_in[addr(j+1,i-1)]
                +1*data_in[addr(j-1,i+1)] +2*data_in[addr(j,i+1)] +1*data_in[addr(j+1,i+1)];

            data_out[addr(j,i)] = value( abs(tmp_h) + abs(tmp_v) );
        }
}
// Sobel 3 : 4 acces memoire en moins par iteration
// Average sobel filter duration: 47.6487ms
void sobel3(int width, int height, uchar* data_in, uchar* data_out, int y_start, int y_stop)
{
    int i,j;
    int tmp_v;
    int tmp_h;
    uchar a,b,c,d;


    for (i=y_start; i<y_stop;i++)
        for(j=1;j<width-1;j++){
			a = data_in[addr(j-1,i-1)];
			b = data_in[addr(j+1,i-1)];
			c = data_in[addr(j-1,i+1)];
			d = data_in[addr(j+1,i+1)];

            tmp_v =     - a -2*data_in[addr(j-1,i)] - c + b +2*data_in[addr(j+1,i)] + d;
            tmp_h =     - a -2*data_in[addr(j,i-1)] - b + c +2*data_in[addr(j,i+1)] + d;

            data_out[addr(j,i)] = value( abs(tmp_h) + abs(tmp_v) );
        }
}
// Sobel 4 : Prog dynamique
// Average sobel filter duration: 37.9785ms
void sobel4(int width, int height, uchar* data_in, uchar* data_out, int y_start, int y_stop)
{
    int i,j;
    int tmp_v;
    int tmp_h;
    uchar a,b,c,d;
 	uchar h1,h2,v1,v2,tmp;



	for (i=y_start; i<y_stop;i++){
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
}
// sort_median : optimisation tri diagonal
// Average median filter duration: 207.333ms
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


uchar qselect(uchar *v, int len, int k)
{
	#define SWAP(a, b) { tmp = v[a]; v[a] = v[b]; v[b] = tmp; }
	int i, st, tmp;
	for (st = i = 0; i < len - 1; i++) {
		if (v[i] > v[len-1]) continue;
			SWAP(i, st);
			st++;
	}
	SWAP(len-1, st);
	return k == st	?v[st]
			:st > k	? qselect(v, st, k)
			: qselect(v + st, len - st, k - st);
}

// Sort_median : quick select
// 350 ms
uchar sort_median2(uchar data[9]){
	return qselect(&data[0],9,4);
}

// Median 1 : Sans optimisation
// Average median filter duration: 319.333ms
uchar* median_filter(int width, int height, uchar* in, uchar* out, int y_start, int y_stop)
{
    uchar tmp[9];
    int i,j,k,l,m;
    for (i=y_start; i<y_stop;i++)
        for(j=1;j<width-1;j++)
        {
            m = 0;
            for(k=-1; k<2; k++)
                for(l=-1;l<2;l++,m++){
                    tmp[m] = in[addr(j+k,i+l)];
                }
            out[addr(j,i)] = sort_median(tmp);
        }
}
// Median 2 : Prog dynamique
// Average median filter duration: 229.277ms
void median_filter2(int width, int height, uchar* in, uchar* out, int y_start, int y_stop)
{
    uchar t[9];
    int i,j;
    for (i=y_start; i<y_stop;i++){
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
}

// Median 3 : Prog dynamique + deroulage de boucle
// Average median filter duration: 199.34ms
void median_filter3(int width, int height, uchar* in, uchar* out, int y_start, int y_stop)
{
    uchar t[9]; // Premiere boucle
	uchar n[9]; // Seconde Boucle
    int i,j;
    for (i=y_start; i<y_stop;i++){
		t[0]=in[addr(0,i-1)]; t[1]=in[addr(1,i-1)]; //t[2]=in[addr(2,i-1)];
		t[3]=in[addr(0, i )]; t[4]=in[addr(1, i )]; //t[5]=in[addr(2, i )];
		t[6]=in[addr(0,i+1)]; t[7]=in[addr(1,i+1)]; //t[8]=in[addr(2,i+1)];

		n[0]=t[1]; n[1]=t[2]; //n[2]=in[addr(3,i-1)];
		n[3]=t[4]; n[4]=t[5]; //n[5]=in[addr(3, i )];
        n[6]=t[7]; n[7]=t[8]; //n[8]=in[addr(3,i+1)];

		for(j=1;j<width-2;j=j+2){
			t[2] = in[addr(j+1,i-1)];
			t[5] = in[addr(j+1, i )];
			t[8] = in[addr(j+1,i+1)];

			n[2] = in[addr(j+2,i-1)];
			n[5] = in[addr(j+2, i )];
        	n[8] = in[addr(j+2,i+1)];


       		out[addr( j ,i)] = sort_median(t);
			out[addr(j+1,i)] = sort_median(n);

			t[0]=n[1]; t[1]=n[2];
			t[3]=n[4]; t[4]=n[5];
			t[6]=n[7]; t[7]=n[8];

			n[0]=t[1]; n[1]=t[2];
			n[3]=t[4]; n[4]=t[5];
        	n[6]=t[7]; n[7]=t[8];
        }
	}
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

    const unsigned cores = std::min(std::thread::hardware_concurrency(), (unsigned)1);
    std::cout << "Using " << cores << " cores." << std::endl;
    std::vector<std::future<void>> results(cores);

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

        // Median
        int64 median_start = cv::getTickCount();
        for(unsigned i = 0; i < cores; i++) {
            int h = height - 2;
            int y_start = i * h / cores + 1,
                y_stop = (i + 1) * h / cores + 1;
            results[i] = std::async(std::launch::async, &median_filter3, width, height, im_1_data, im_2_data, y_start, y_stop);
        }
        for(unsigned i = 0; i < cores; i++) {
            results[i].wait();
        }
        median_time += (double)(cv::getTickCount() - median_start) / cv::getTickFrequency();

        sobel(width, height, im_1_data, im_3_data, 1, height - 1); // Sobel

        // Median + Sobel
        int64 sobel_start = cv::getTickCount();
        for(unsigned i = 0; i < cores; i++) {
            int h = height - 2;
            int y_start = i * h / cores + 1,
                y_stop = (i + 1) * h / cores + 1;
            results[i] = std::async(std::launch::async, &sobel4, width, height, im_2_data, im_4_data, y_start, y_stop);
        }
        for(unsigned i = 0; i < cores; i++) {
            results[i].wait();
        }
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
