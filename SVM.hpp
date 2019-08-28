void  read_trainData(string fn, Mat & trainingData, Mat & lables = Mat())
{
	FileStorage fs(fn, FileStorage::READ);
	CV_Assert(fs.isOpened());

	fs["TrainingData"] >> trainingData;
	fs["classes"] >> lables;
	fs.release();

	trainingData.convertTo(trainingData, CV_32FC1);
}

Ptr<ml::SVM>  SVM_create(int type, int max_iter, double epsilon)
{
	Ptr<ml::SVM> svm = ml::SVM::create();		// SVM 객체 선언
												// SVM 파라미터 지정
	svm->setType(ml::SVM::C_SVC);				// C-Support Vector Classification				
	svm->setKernel(ml::SVM::LINEAR);			// 선형 SVM 
	svm->setGamma(1);							// 커널함수의 감마값
	svm->setC(1);								// 최적화를 위한 C 파리미터

	TermCriteria criteria(type, max_iter, epsilon);
	svm->setTermCriteria(criteria);				// 반복 알고리즘의 조건
	return svm;
}

int classify_plates(Ptr<ml::SVM> svm, vector<Mat> candi_img)
{
	for (int i = 0; i < (int)candi_img.size(); i++)
	{
		Mat onerow = candi_img[i].reshape(1, 1);  // 1행 데이터 변환
		onerow.convertTo(onerow, CV_32F);

		Mat results;						// 분류 결과 저장 행렬
		svm->predict(onerow, results);		// SVM 분류 수행

		if (results.at<float>(0) == 1) 		// 분류결과가 번호판이면
			return i;						// 영상번호 반환
	}
	return -1;
}

