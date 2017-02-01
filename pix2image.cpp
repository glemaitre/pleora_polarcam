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
	imshow("original imahe", origin);
        // declare the vector containing the 4 angles images
        const int nb_angles = 4;
	std::vector<cv::Mat> output_img(nb_angles);                           
        for (auto it = output_img.begin(); it != output_img.end(); ++it)
            *it = cv::Mat::zeros(output_size, CV_8U);

        // copy the data in the new image
        for (int angle = 0; angle < nb_angles; ++angle){
	    int offset_row = angle / 2;
	    int offset_col = angle % 2;
	    cout<< "offset_row " << offset_row << "  offset_col" << offset_col 
		<< std::endl; 

	    for (int row = 0; row < origin.rows/2; ++row){
                for (int col = 0; col < origin.cols/2; ++col){
		    output_img[angle].at<uchar>(row, col) = origin.at<uchar>(
                        2 * row + offset_row, 2 * col + offset_col); 
		}
	    }
	}		
        // Return the image
        return output_img;
    }

    std::vector<cv::Mat> compute_stokes(const std::vector<cv::Mat>& angles_img)
    {
        // define the number of images to have for Stokes
        const int nb_stokes_img = 3;
        // Create zeros images
        std::vector<cv::Mat> output_img(nb_stokes_img);
	for (auto it = output_img.begin(); it != output_img.end(); ++it)
	    *it = cv::Mat::zeros(angles_img[0].size(), CV_32F);

        // compute the Stokes parameters maps
	// S0: add the different angles
        for (auto it = angles_img.begin(); it != angles_img.end(); ++it)
            cv::add(output_img[0], *it, output_img[0], cv::noArray(),
                    CV_32F);
        output_img[0] /= 2.0;
    	minmax(output_img[0], "s0");  
	

       // S1: subtract angles 0 and 90
	cv::subtract(angles_img[0], angles_img[2], output_img[1],
                     cv::noArray(), CV_32F);

	minmax(output_img[1], "s1"); 
	// S2: subtract angles 45 and 135
        cv::subtract(angles_img[1], angles_img[3], output_img[2],
                     cv::noArray(), CV_32F);
	minmax(output_img[2], "s2"); 

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
        std::vector<cv::Mat> output_img(nb_params);
	for (auto it = output_img.begin(); it != output_img.end(); ++it)
	    *it = cv::Mat::zeros(stokes_img[0].size(), CV_32F);


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



    int minmax (cv::Mat img, std::string s)
    {
	double min, max;
	Point idmin, idmax;
	minMaxLoc(img, &min, &max, &idmin, &idmax) ;
        
	cout <<"min max " + s + " : " << min << " " << max << std::endl;
    }

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

	    // create the zeros output image
	    cv::Mat output_img = cv::Mat::zeros
		(img[0].rows*2, img[0].cols*2, CV_8UC1);
	    
	    // conversion of CV_32F to CV_8UC1 
	    img[0] /= 2.0;
	    img[1] = (img[1]+255.0)/2.0; 
	    img[2] = (img[2]+255.0)/2.0; 
	    for (int i = 0;  i <=2; ++i){
		img[i].convertTo(img[i], CV_8UC1); 
	    }		
					       
	    // coping the data to the output image 
	    img[0].copyTo(output_img(cv::Rect(0, 0, 
					      img[0].cols, img[0].rows)));
	    img[1].copyTo(output_img(cv::Rect(rows, 0, 
					      img[1].cols, img[1].rows)));
	    img[2].copyTo(output_img(cv::Rect(0, cols, 
					      img[2].cols, img[2].rows)));
	    

	    imshow("Stokes-params", output_img); 


	}else if ((img.size() == 3) && !as_stokes){
	    
	   
	    // create the zeros output image
	    cv::Mat output_img = cv::Mat::zeros
		(img[0].rows*2, img[0].cols*2, CV_8UC1);
	    
	    // conversion from CV_32F tp CV_8UC1
	    img[0] = img[0]*255 ;
	    img[2] = img[2]/2; 
	    for (int i = 0; i <=2; ++i){
	       img[i].convertTo(img[i], CV_8UC1); 
	    }

            // coping the data to the output image 
	    img[0].copyTo(output_img(cv::Rect(0, 0, 
					      img[0].cols, img[0].rows)));
	    img[1].copyTo(output_img(cv::Rect(rows, 0, 
					      img[1].cols, img[1].rows)));
	    img[2].copyTo(output_img(cv::Rect(0, cols, 
					      img[2].cols, img[2].rows)));
	    

	    imshow("polar_params", output_img); 
	    

	    if (as_hsv){
		// merge the vector into a single matrix
		cv::Mat img_hsv; 
		merge(img, img_hsv); 

		// convert from bgr to hsl
		cv::Mat HSL; 
		cvtColor(img_hsv, HSL, CV_HLS2BGR);
    
		// show the hsv image 
		imshow("hsv image", img_hsv);  
	    
	    }else{
		  
		// DoP image 
		imshow("DoP", img[0]); 
		// AoP image
		imshow("AoP", img[1]); 
	    
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

    // parsed image from original image 
    std::vector<cv::Mat> angle_image = POLPro::raw2mat(image); 
    
    // Stokes parameters 
    std::vector<cv::Mat> stokes_images = POLPro::compute_stokes(image);
    
    // polar componnets
    std::vector<cv::Mat> polar_images = 
	POLPro::compute_polar_params(stokes_images); 

    //showing the paramters

    POLPro::imshow(angle_image); 

    POLPro::imshow(stokes_images); 

    POLPro::imshow(polar_images, false, false); 
    
   waitKey(0); 
   return 0 ; 
   
}
