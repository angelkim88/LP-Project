#include "final_preprocess.hpp"					// 전처리 및 후보영역 검출 함수
#include "candiate.hpp"						// 후보영역 개선 및 후보영상 생성 함수
#include "SVM.hpp"						// 학습데이터 로드 및 SVM 수행
#include "kNN.hpp"						// kNN 학습 함수 – 10장 3절
#include "classify_objects.hpp"			// kNN 분류 함수

int main()
{
	
	int K1 = 15  , K2 = 15;
	Ptr<ml::KNearest> knn[2];
	knn[0] = kNN_train("../image_data/trainimage/train_numbers2001.png", K1, 10, 20);
	knn[1] = kNN_train("../image_data/trainimage/train_texts.png", K2, 25, 20);
	Ptr<ml::SVM> svm = ml::StatModel::load<ml::SVM>("../SVMtrain.xml");
	int car_no;
	cout << "차량 영상 번호 (0-20) : ";
	cin >> car_no;
	// string fn = format("../image/final_car/%003d.jpg", car_no);
	 string fn = format("../image_data/test_carrrrr/%02d.jpg", car_no);
	Mat image = imread(fn, 1);
	CV_Assert(image.data);
	double time = (double)cv::getTickCount();
	Mat morph = preprocessing(image);						
	vector<RotatedRect> candidates;
	find_candidates(morph, candidates);							
	vector<Mat> candidate_img = make_candidates(image, candidates);
	int plate_no = classify_plates(svm , candidate_img);		
	if (plate_no >= 0) 
	{
		vector <Rect> obejct_rects, sorted_rects;
		vector <Mat> numbers;						
		Mat  plate_img, color_plate;					
		preprocessing_plate(candidate_img[plate_no], plate_img);	
		cvtColor(plate_img, color_plate, CV_GRAY2BGR);
		find_objects(~plate_img, obejct_rects);	 
		sort_rects(obejct_rects, sorted_rects);		
		for (size_t i = 0; i < sorted_rects.size(); i++) 
		{
			numbers.push_back(plate_img(sorted_rects[i]));	
			rectangle(color_plate, sorted_rects[i], Scalar(0, 0, 255), 1); 
		}
		if (numbers.size() == 7) {
			classify_numbers(numbers, knn, K1, K2);	
		}
		else  cout << "숫자(문자) 객체를 정확히 검출하지 못했습니다." << endl;	
		draw_rotatedRect(image, candidates[plate_no], Scalar(0, 0, 255), 2);
	}
	else  cout << "번호판을 검출하지 못하였습니다. " << endl;
	time = (double)cv::getTickCount() - time; //////
	printf("%5.4fms\n", time*1000. / cv::getTickFrequency());
	imshow("image", image);
	waitKey();
	return 0;
}

