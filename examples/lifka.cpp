/* Copyright 2017 
 * 
 * Compile with:
g++ lifka.cpp -lcrn -o lifka
 *
 * \Author: 
 * 	- Javier Izquierdo Vera - javierizquierdovera@gmail.com
 */

#include <CRNBlock.h>
#include <CRNImage/CRNImageGray.h>
#include <CRNFeature/CRNFeatureSet.h>
#include <CRNFeature/CRNFeatureExtractorProfile.h>
#include <CRNFeature/CRNFeatureExtractorProjection.h>
#include <CRNFeature/CRNBlockTreeExtractorTextLinesFromProjection.h>
#include <CRNAI/CRNBasicClassify.h>
#include <CRNUtils/CRNTimer.h>
#include <CRNIO/CRNIO.h>

#define VERSION "1.0"
#define NAME "lifka"



void savePNG(crn::SImage img, std::string name){

	name.append(".png");
	img->SavePNG(name);
}

void scaleToSize(crn::SImage img, size_t width, size_t height){
	img->ScaleToSize(width, height);
}

bool scale(crn::SImage img, char* img_path, size_t width, size_t height){

	bool error = false;

	std::string name = img_path;
	name.append("_");
	name.append(std::to_string(width));
	name.append("x");
	name.append(std::to_string(height));

	scaleToSize(img, width, height);

	savePNG(img, name);

	return error;
}

crn::SBlock getBlockFromImg(crn::SImage img){
	return crn::Block::New(img);
}

crn::SImage getImgFromPath(char* img_path){
	return crn::NewImageFromFile(img_path);
}

bool getBuffer(crn::SImage img, int opt, char* img_path){
	bool error = false;

	std::string name = img_path;

	crn::SBlock block = getBlockFromImg(img);

	crn::SImage img_result = crn::SImage{};

	if (opt == 0){
		name.append("_gray");
		img_result = block->GetGray();

	} else if(opt == 1){
		name.append("_RGB");
		img_result = block->GetRGB();

	} else if(opt == 2){
		name.append("_BW");
		img_result = block->GetBW();

	}

	savePNG(img_result, name);

	return error;
}

void usage(char* prog_name){
  std::cout << std::endl << prog_name << " [option] [img]" << std::endl <<
      "Options:" << std::endl <<
      "-h    | --help                       Print this help" << std::endl <<
      "-v    | --version                    Print the script version" << std::endl <<
      "-i    | --information                Print information about image" << std::endl <<
      "-GRAY | --grayscale                  Get the gray scale image" << std::endl <<
      "-RGB  | --RGB                        Get the RGB image" << std::endl <<
      "-BW   | --blackwhite                 Get the black and white image" << std::endl <<
      "-s    | --scale  [width] [height]    Scale image" << std::endl;
}

void option (int argc, char *argv[]){

	bool error = false;
	char* prog_name = argv[0];

	if (argc >= 2){

		char* option = argv[1];

		if (!strcmp(option, "-h") || !strcmp(option,"--help") ) {
			usage(prog_name);
		} else if (!strcmp(option, "-v") || !strcmp(option,"--version") ) {
			std::cout << NAME << " " << VERSION << std::endl;
		} else if(argc >= 3){

			char* img_path = argv[2];

			crn::SImage img = crn::SImage{};

			try{
				img = getImgFromPath(img_path);
			} catch (std::exception &ex) {
				CRNError("Cannot open image");
				CRNVerbose(crn::String(U" ") + ex.what());
				error = true;
			} 	

			if (!error){
				if (!strcmp(option, "-GRAY") || !strcmp(option,"--grayscale") ) {
					error = getBuffer(img, 0, img_path);
				} else if (!strcmp(option, "-RGB") || !strcmp(option,"--RGB") ) {
					error = getBuffer(img, 1, img_path);
				} else if (!strcmp(option, "-BW") || !strcmp(option,"--blackwhite") ) {
					error = getBuffer(img, 2, img_path);
				} else if (!strcmp(option, "-i") || !strcmp(option,"--information") ) {
					auto img_gray = crn::NewImageGrayFromFile(argv[2]);
					
					std::cout << "width: " << img->GetWidth() << std::endl
					 << "height: " << img->GetHeight() << std::endl 
					 << "pixels: " << img->Size() << std::endl
					 << "strokes width: " << StrokesWidth(*img_gray) << std::endl
					 << "strokes height: " << StrokesHeight(*img_gray) << std::endl
					 << "lines height: " << EstimateLinesXHeight(*img_gray) << std::endl;

				} else if (argc == 5){
		
					if (!strcmp(option, "-s") || !strcmp(option,"--scale") ) {

						unsigned int width = atoi(argv[3]);
						unsigned int height = atoi(argv[4]);

						error = scale(img, img_path, width, height);
					} else {
						error = true;
					}

				} else{
					error = true;
				}
			}

		} else {
			error = true;
		}

	} else {
		error = true;
	}	

	if (error){
		std::perror("Error");
		usage(argv[0]);
		exit(-1);
	}

	exit(0);
}

int main(int argc, char *argv[]){
	option(argc, argv);
}