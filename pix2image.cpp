#include <pix2image.h>

//#include <photonfocus_camera.h>

using namespace cv; 
using namespace std; 

namespace POLPro
{

    std::vector<cv::Mat> raw2mat(const cv::Mat& origin)
  {
        // define the size of the output
	cv::Size output_size(origin.cols / 2, origin.rows / 2);

        // declare the vector containing the 4 angles images
        const int nb_angles = 4;
        std::vector<cv::Mat> output_img(nb_angles, cv::Mat::zeros
					(output_size, origin.type()));
	
        // copy the data in the new image
	// Efficient way 

        // for (int angle = 0; angle < nb_angles; ++angle)
        //     for (int row = 0; row < output_size.height; ++row)
        //         for (int col = 0; col < output_size.width; ++col)
        //         {
        //             int offset_row = angle / 2;
        //             int offset_col = angle % 2;
        //             output_img[angle].at<uchar>(row, col) = origin.at<uchar>(
        //                 2 * row + offset_row, 2 * col + offset_col);
        //         }

	// dummy way 
        int cols = origin.cols/2;
        int rows = origin.rows/2;
	cv::Mat I0 = cv::Mat(rows, cols, origin.type());
	cv::Mat I45 = cv::Mat(rows, cols, origin.type());
	cv::Mat I90 = cv::Mat(rows, cols, origin.type());
	cv::Mat I135 = cv::Mat(rows, cols, origin.type());

        for (int i=0; i<rows;i++){
	    for (int j=0; j<cols;j++){
	        I0.at<uchar>(i, j) = origin.at<uchar>(2*i, 2*j);
		I45.at<uchar>(i, j) = origin.at<uchar>(2*i+1, 2*j);
		I90.at<uchar>(i, j) = origin.at<uchar>(2*i, 2*j+1);
	        I135.at<uchar>(i, j) = origin.at<uchar>(2*i+1,2*j+1);
	    }
        }
	output_img[0] = I0; 
	output_img[1] = I45; 
	output_img[2] = I90; 
	output_img[3] = I135; 
	
	
        // Return the image
        return output_img;
    }

    std::vector<cv::Mat> compute_stokes(const std::vector<cv::Mat>& angles_img)
    {
        // define the number of images to have for Stokes
        const int nb_stokes_img = 3;
        // Create zeros images
        std::vector<cv::Mat> output_img(nb_stokes_img, cv::Mat::zeros(
                                            angles_img[0].size(), CV_32F));

        // compute the Stokes parameters maps
        // S0: add the different angles
        // for (auto it = angles_img.begin(); it != angles_img.end(); ++it)
        //     cv::add(output_img[0], *it, output_img[0], cv::noArray(),
        //             CV_32F);
        // output_img[0] /= 2.0;

	double min, max;
        Point idmin, idmax;

        add(angles_img[0], angles_img[2], output_img[0], noArray(), CV_32F);
	add(angles_img[1], output_img[0], output_img[0], noArray(), CV_32F);
        add(angles_img[3], output_img[0], output_img[0], noArray(), CV_32F);
        
	output_img[0] = output_img[0] / 2.0;

	minMaxLoc(output_img[0], &min, &max, &idmin, &idmax) ;
        cout <<"min max s0: " << min << " " << max << std::endl;

        // S1: subtract angles 0 and 90
        cv::subtract(angles_img[0], angles_img[2], output_img[1],
                     cv::noArray(), CV_32F);
        // S2: subtract angles 45 and 135
        cv::subtract(angles_img[1], angles_img[3], output_img[2],
                     cv::noArray(), CV_32F);

        return output_img;
    }

    std::vector<cv::Mat> compute_stokes(const cv::Mat& origin)
    {
        // refactor the raw image
        std::vector<cv::Mat> angles_img = raw2mat(origin);

        return compute_stokes(angles_img);
    }

    std::vector<cv::Mat> compute_polar_params(
        const std::vector<cv::Mat>& origin)
    {
        std::vector<cv::Mat> stokes_img;
        // Check if we have the original data or the stokes
        if (origin.size() == 4)
        {
            stokes_img = compute_stokes(origin);
        } else {
            stokes_img = origin;
        }

        // define the number of maps
        const int nb_params = 3;
        // create the zeros images
        std::vector<cv::Mat> output_img(nb_params, cv::Mat::zeros(
                                            stokes_img[0].size(), CV_32F));

        // compute the polar coordinate in degrees
        cv::cartToPolar(stokes_img[1], stokes_img[2],
                        output_img[0], output_img[1],
                        true);
        // normalize the maps
	// dop
        output_img[0] /= stokes_img[0];
        // aop
	output_img[1] *= 0.5;
        // copy s0
        stokes_img[0].copyTo(output_img[2]);

        return output_img;
    }

    std::vector<cv::Mat> compute_polar_params(const cv::Mat& origin)
    {
        // compute the Stokes' parameters
        std::vector<cv::Mat> stokes_img = compute_stokes(origin);

        return compute_polar_params(stokes_img);
    }


//////////////////////

    void imshow(std::vector<cv::Mat> img, bool as_hsv=false, 
		bool as_stokes=true)
    {
	int cols = img[0].rows; 
	int rows = img[0].cols; 

        // through an error if there is not 3d img and hsv is turned on
        if ((img.size() != 3) && as_hsv)
            throw std::invalid_argument("img needs to be a 3 channels images"
                                        " if you need hsv support");
	
        // Declarig the appropriate size depeding on the image size
	
	if ((img.size() == 3) && as_stokes && !as_hsv){

	    // create the zeros images
	    cv::Mat output_img(img[0].rows*2, img[0].cols*2, CV_8UC1);
	    cv::Mat s; 
	    s = img[0]; 
    	   s = s/2.0 ; 
	   img[1] = (img[1]+255.0)/2.0; 
	   img[2] = (img[2]+255.0)/2.0; 
	   for (int i = 0;  i <=2; ++i){
	       img[i].convertTo(img[i], CV_8UC1); 
	       }		
					       

	    s.copyTo(output_img(cv::Rect(0, 0, 
					      s.cols, s.rows)));
	    img[1].copyTo(output_img(cv::Rect(rows, 0, 
					      img[1].cols, img[1].rows)));
	    img[2].copyTo(output_img(cv::Rect(0, cols, 
					      img[2].cols, img[2].rows)));
	    

	    imshow("Stokes-params", output_img); 


	}else if ((img.size() == 3) && !as_stokes){
	    
	    // define the number of maps
	    const int nb_params = 3;
	    // create the zeros images
	    std::vector<cv::Mat> output_img(nb_params, cv::Mat::zeros(
                                            img[0].size(), CV_8UC1));
	    img[0] = img[0]*255 ;
	    img[2] = img[2]/2; 
	    for (int i = 0; i <=2; ++i){
	       img[i].convertTo(img[i], CV_8UC1); 
	       img[i].copyTo(output_img[i]); 
	    }
	    // for (auto it = img.begin(); it != img.end(); ++it)
	    // 	output_img[it] = img[it].ConvertTo(img[it], CV_8UC1); 
	  

	    if (as_hsv){
		// merge the vector into a single matrix
		cv::Mat img_hsv; 
		merge(output_img, img_hsv); 

		// convert from bgr to hsl
		cv::Mat HSL; 
		cvtColor(img_hsv, HSL, CV_HLS2BGR);
    
		// show the hsv image 
		imshow("hsv image", img_hsv);  
		// DoP image 
		imshow("DoP", output_img[0]); 
		// AoP image
		imshow("AoP", output_img[1]); 
	    
	    }else{
		  
		// DoP image 
		imshow("DoP", output_img[0]); 
		// AoP image
		imshow("AoP", output_img[1]); 
	    
	    }
        
    		 
	}else if (img.size() ==4){
	    // these images are already in 8 bit and does not require conversion
	    // create the zeros images
	    cv::Mat output_img(img[0].rows*2, img[0].cols*2, CV_8UC1);
	   
	    img[0].copyTo(output_img(cv::Rect(0, 0, 
					      img[0].cols, img[0].rows)));
	    img[1].copyTo(output_img(cv::Rect(rows, 0, 
					      img[1].cols, img[1].rows)));
	    img[2].copyTo(output_img(cv::Rect(0, cols, 
					      img[2].cols, img[2].rows)));
	    img[3].copyTo(output_img(cv::Rect(rows, cols, 
					      img[3].cols, img[3].rows)));


	    imshow("parsed_image", output_img); 
	    	    
	}else{
	    throw std::invalid_argument("img needs to be a 3 or 4 channels"); 
	}

	

    }
}


					       
int main( int argc, char** argv )
{
    if( argc != 2)
    {
	cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    Mat image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);   // Read the file

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

   
    std::vector<cv::Mat> angle_image = POLPro::raw2mat(image); 
    cout << "image size \t" << angle_image.size()<< std::endl; 
   
    std::vector<cv::Mat> stokes_images = POLPro::compute_stokes(image);
    // std::vector<cv::Mat> polar_images = 
    //     POLPro::compute_polar_params(stokes_images);
   
    POLPro::imshow(stokes_images); 
    // POLPro::imshow(polar_images, true, false); 
    POLPro::imshow(angle_image); 
    //imshow("parsed image", angle_image); 
   waitKey(0); 
   return 0 ; 
   
}
