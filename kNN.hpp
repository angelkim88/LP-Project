void find_histoPos(Mat img, int & start, int &end, int direct)
{
	reduce(img, img, direct, REDUCE_AVG);

	int  minFound = 0;
	for (int i = 0; i< (int)img.total(); i++) {
		if (img.at<uchar>(i) < 250) 	
		{
			end = i;						
			if (!minFound) {
				start = i;				
				minFound = 1;
			}
		}
	}
}
Mat find_number(Mat part)
{
	Point start, end;
	find_histoPos(part, start.x, end.x, 0);	
	find_histoPos(part, start.y, end.y, 1);   
	return part(Rect(start, end));		
}
Mat  place_middle(Mat number, Size new_size)
{
	int  big = max(number.cols, number.rows);
	Mat  square(big, big, number.type(), Scalar(255));	
	Point start = (square.size() - number.size()) / 2;
	Rect middle_rect(start, number.size());			
	Mat middle = square(middle_rect);
	number.copyTo(middle);
	resize(square, square, new_size);			
	square.convertTo(square, CV_32F);
	return square.reshape(0, 1);
}
Ptr<ml::KNearest>   kNN_train(string train_img, int K, int Nclass, int Nsample)
{
	Size  size(40, 40);							
	Mat  trainData, classLable;
	Mat  train_image = imread(train_img, 0);		
	CV_Assert(train_image.data);
	threshold(train_image, train_image, 32, 255, THRESH_BINARY);
	for (int i = 0, k = 0; i<Nclass; i++) {
		for (int j = 0; j< Nsample; j++, k++)
		{
			Point pt(j * size.width, i * size.height);	
			Rect  roi(pt, size);
			Mat   part = train_image(roi);			
			Mat  num = find_number(part);		
			Mat  data = place_middle(num, size);		
			trainData.push_back(data);						
			classLable.push_back(i);				
		}
	}
	Ptr<ml::KNearest>  knn = ml::KNearest::create();	
	knn->train(trainData, ml::ROW_SAMPLE, classLable);	
	return knn;
}