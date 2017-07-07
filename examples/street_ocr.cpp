/* Copyright 2017
 * 
 * based on: ocr4dummies.cpp
 * 
 * A quick OCR engine to recognize letters from street names.
 * 
 * Compile with:
g++ street_ocr.cpp -o street_ocr -lcrn
*
 * \author 
 * Javier Izquierdo Vera - javierizquierdovera@gmail.com
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

using namespace crn::literals;

int main(int argc, char *argv[]){

	std::cout << std::endl;

	// Check argument.
	if (argc < 2){
		printf("Usage: %s <image_name>\n", argv[0]);
		return -1;
	}

	// Verbose
	crn::IO::IsVerbose() = true;
	crn::IO::IsQuiet() = false;

	// Starts the quick stopwatch.
	crn::Timer::Start(U"StreetOCR");



	/**************************************************************************/
	/* 1. Database                                                            */
	/**************************************************************************/
	// Create a feature extraction engine.
	auto featureExtractor = crn::FeatureSet{};

	// It will extract the four profiles, reduced each to 10 values in [0..100].
	featureExtractor.PushBack(std::make_shared<crn::FeatureExtractorProfile>(
				crn::Direction::LEFT | crn::Direction::RIGHT | 
				crn::Direction::TOP | crn::Direction::BOTTOM, 10, 1000));
	
	// It will also extract the two projections under the same conditions.
	featureExtractor.PushBack(std::make_shared<crn::FeatureExtractorProjection>(
				crn::Orientation::HORIZONTAL | crn::Orientation::VERTICAL, 10, 100));
	
	// Create the database.
	auto database = std::vector<crn::SObject>();


	// For each character...
	std::cout << "[*] Load characters: ";
	
	for (auto c = 'A'; c <= 'Z'; ++c){
	
		// Open a prototype image stored as "street/font/*.png"
		auto charFileName = "street/font/"_p / c + ".png"_p;

		auto charimage = crn::SImage{};
		
		try{
			// Open image
			charimage = crn::NewImageFromFile(charFileName);
		} catch (std::exception &ex) {
			CRNError(U"Cannot open database: ");
			CRNVerbose(crn::String(U" ") + ex.what());
			return -2;
		} 
		
		// Embed the image in a block structure
		auto charblock = crn::Block::New(charimage);
		
		// Extract the features and store it in the database (vector)
		database.push_back(featureExtractor.Extract(*charblock));
	}

	std::cout << std::endl << std::endl;
	crn::Timer::Split(U"StreetOCR", U"Database");

	/**************************************************************************/
	/* 2. Document                                                            */
	/**************************************************************************/

	// Open the document image file
	auto imageFileName = crn::Path(argv[1]);
	auto pageimage = crn::SImage{};
	
	try {
		pageimage = crn::NewImageFromFile(imageFileName);
	} catch (...) {
		CRNError(U"Cannot open document image");
		return -3;
	}


	/**************************************************************************/
	/* 2.1 Segmentation                                                       */
	/**************************************************************************/
	
	// Embed the image in a block structure.
	auto pageblock = crn::Block::New(pageimage);
	
	// Extract text lines
	crn::BlockTreeExtractorTextLinesFromProjection{U"Lines"}.Extract(*pageblock);
	
	/**************************************************************************/
	/* 2.2 Recognition                                                        */
	/**************************************************************************/
	auto s = crn::String{};
	
	// For each line...
	for (auto nline = size_t{0}; nline < pageblock->GetNbChildren(U"Lines"); ++nline){
		
		// Obtiene del árbol Lines la línea nline
		auto line = pageblock->GetChild(U"Lines", nline);
		
		// Extract connected components in the line (characters).
		// To do that, a new black and white image (that is not smeared) 
		// is automatically computed.
		line->ExtractCC(U"Characters");

		/**/line->FilterMinOr(U"Characters", 2, 2);
		
		// Sort characters from left to right.
		line->SortTree(U"Characters", crn::Direction::LEFT);
		
		// For each character...
		for (auto nchar = size_t{0}; nchar < line->GetNbChildren(U"Characters"); ++nchar){
			auto character = line->GetChild(U"Characters", nchar);

			// Extract the features.
			auto features = featureExtractor.Extract(*character);

			// Classify the character using the database.
			auto res = crn::BasicClassify::NearestNeighbor(features, database.begin(), database.end());

			// Print the characters label.
			s += char32_t(U'A' + res.class_id);

			char character_fn = char32_t(U'A' + res.class_id);

			std::cout << "ID char read: "
				 << res.class_id <<
				  " (id + val_char('A') ) --> " <<
				   char32_t(U'A' + res.class_id) <<
				   " = " << character_fn << std::endl;
		}
		
		// End of line
		s += U'\n';
	}


	// Verbose
	CRNVerbose(s);

	// Records time in a stopwatch. Stores a time with a name.
	// Separa el tiempo de Recognition del total
	crn::Timer::Split(U"StreetOCR", U"Recognition");
	CRNVerbose(crn::Timer::Stats(U"StreetOCR"));
}


