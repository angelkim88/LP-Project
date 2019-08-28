void preprocessing_plate(Mat plate_img , Mat& ret_img)
{
	resize(plate_img, plate_img, Size(180, 35));
	threshold(plate_img, plate_img, 32, 255, THRESH_BINARY | THRESH_OTSU);
	imwrite("plate_img.png", plate_img);
	Point pt1 = Point(6, 3) ;
	Point pt2 = plate_img.size() - Size(pt1);
	ret_img = plate_img(Rect(pt1, pt2)).clone();
	imwrite("ret_img.png", ret_img);
}

void find_objects(Mat sub_mat, vector <Rect>& object_rects)
{
	vector<vector<Point>> contours;
	findContours(sub_mat, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	
	vector <Rect> text_rects;
	for (int i = 0; i< (int)contours.size(); i++)
	{
		Rect r = boundingRect(contours[i]);				
		if (r.width / (float)r.height > 2.5 ) continue;
		if (r.x > 45 && r.x < 80 && r.area() > 60 ) 
			text_rects.push_back(r); 					
		else if (r.area() > 150)							
		{
			object_rects.push_back(r); 
		}
	}
	if (text_rects.size() > 0) {
		for (size_t i = 1; i < (int)text_rects.size(); i++) {		
			text_rects[0] |= text_rects[i];
		}
		object_rects.push_back(text_rects[0]);				
	}
}

void sort_rects(vector <Rect> object_rects, vector<Rect>& sorted_rects)
{
	Mat pos_x;
	for (size_t i = 0; i < object_rects.size(); i++) { 
		pos_x.push_back(object_rects[i].x);
	}

	sortIdx(pos_x, pos_x, SORT_EVERY_COLUMN + SORT_ASCENDING);
	for (int i = 0; i < pos_x.rows; i++)	{
		int idx = pos_x.at<int>(i, 0);
		sorted_rects.push_back(object_rects[idx]);
	}
}

void classify_numbers(vector<Mat> numbers, Ptr<ml::KNearest> knn[2], int K1, int K2)
{
	string text_value[] = {							
		"가", "나", "다", "라", "마", "거", "너", "더", "러", "머",
		"고", "노", "도", "로", "모", "구", "누", "두", "루", "무",
		"바", "사", "아", "자", "허",
	};

	cout << "분류결과 : ";
	for (int i = 0; i < (int)numbers.size(); i++)
	{
		Mat  num = find_number(numbers[i]);			
		Mat  data = place_middle(num, Size(40, 40));	

		Mat results;
		if (i == 2) {
			knn[1]->findNearest(data, K1, results);				
			cout << text_value[(int)results.at<float>(0)] << " ";
		}
		else {
			knn[0]->findNearest(data, K2, results);				
			cout << results.at<float>(0) << " ";			
		}
	}
}