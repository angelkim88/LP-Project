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
	Ptr<ml::SVM> svm = ml::SVM::create();		// SVM ��ü ����
												// SVM �Ķ���� ����
	svm->setType(ml::SVM::C_SVC);				// C-Support Vector Classification				
	svm->setKernel(ml::SVM::LINEAR);			// ���� SVM 
	svm->setGamma(1);							// Ŀ���Լ��� ������
	svm->setC(1);								// ����ȭ�� ���� C �ĸ�����

	TermCriteria criteria(type, max_iter, epsilon);
	svm->setTermCriteria(criteria);				// �ݺ� �˰����� ����
	return svm;
}

int classify_plates(Ptr<ml::SVM> svm, vector<Mat> candi_img)
{
	for (int i = 0; i < (int)candi_img.size(); i++)
	{
		Mat onerow = candi_img[i].reshape(1, 1);  // 1�� ������ ��ȯ
		onerow.convertTo(onerow, CV_32F);

		Mat results;						// �з� ��� ���� ���
		svm->predict(onerow, results);		// SVM �з� ����

		if (results.at<float>(0) == 1) 		// �з������ ��ȣ���̸�
			return i;						// �����ȣ ��ȯ
	}
	return -1;
}

