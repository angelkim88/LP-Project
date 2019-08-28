void  refine_candidate(Mat image, RotatedRect& candi)
{
	Mat fill(image.size() + Size(2, 2), CV_8UC1, Scalar(0)); 
	Scalar  dif1(25, 25, 25), dif2(25, 25, 25);						
	int  flags = 4 + 0xff00;									
	flags += FLOODFILL_FIXED_RANGE + FLOODFILL_MASK_ONLY;
	vector<Point2f> rand_pt(15);						
	randn(rand_pt, 0, 7);
	Rect img_rect(Point(0, 0), image.size());			
	for (int i = 0; i < rand_pt.size(); i++) {
		Point2f seed = candi.center + rand_pt[i];	
		if (img_rect.contains(seed)) {				
			floodFill(image, fill, seed, Scalar(), &Rect(), dif1, dif2, flags);
		}
	}
	vector<Point> fill_pts;
	for (int i = 0; i < fill.rows; i++) {		
		for (int j = 0; j < fill.cols; j++) {
			if (fill.at<uchar>(i, j) == 255) 	
				fill_pts.push_back(Point(j, i));		
		}
	}
	candi = minAreaRect(fill_pts);			
}
void  rotate_plate(Mat image, Mat& corp_img, RotatedRect candi)
{
	float aspect = (float)candi.size.width / candi.size.height;	
	float angle = candi.angle;									

	if (aspect < 1) {											
		swap(candi.size.width, candi.size.height);				
		angle += 90;											
	}
	Mat rotmat = getRotationMatrix2D(candi.center, angle, 1);			
	warpAffine(image, corp_img, rotmat, image.size(), INTER_CUBIC);	
	getRectSubPix(corp_img, candi.size, candi.center, corp_img);
}
vector<Mat> make_candidates(Mat image, vector<RotatedRect>& candidates)
{
	vector<Mat> candidates_img;
	for (int i = 0; i < (int)candidates.size();)
	{
		refine_candidate(image, candidates[i]);		
		if (vertify_plate(candidates[i]))			
		{
			Mat corp_img;
			rotate_plate(image, corp_img, candidates[i]);	
			cvtColor(corp_img, corp_img, CV_BGR2GRAY); 			
			resize(corp_img, corp_img, Size(144, 28), 0, 0, INTER_CUBIC); 
			candidates_img.push_back(corp_img);						
			i++;
		}
		else 										
			candidates.erase(candidates.begin() + i);	
	}
	return candidates_img;
}
