#include "S57Reader.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>

typedef list<int> LISTINT;

void S57Reader(string encpath)
{
	LISTINT objllist;

	/*
	Underwater/awash rock ƽˮ��     153
	Land area             ½��       71
	Depth contour         ��ˮ����   43
	*/

	//choose filter shapelayer
	objllist.push_front(71);   // Land area shapelayer
	//objllist.push_front(43); // Depth contour shapelayer
	//objllist.push_back(153); // Underwater/awash rock shapelayer

	//region
	double filter_lllat = 52.4;
	double filter_urlat = 53.2;
	double filter_lllon = -170.9;
	double filter_urlon = -169.4;

	OpenS57File(encpath, objllist, filter_lllat, filter_urlat, filter_lllon, filter_urlon);
}

int main()
{
	cout << "******��ӭʹ�õ��Ӻ�ͼ�������******" << endl;
	cout << "  ��������������Ӻ�ͼ·������Enter��ȷ��" << endl;

	//US1BS01M USA
	string ENCpath = "E://US1BS01M.000";

	S57Reader(ENCpath);
	cout << "  ���Ӻ�ͼ�ļ��������,��������˳���" << endl;
	getchar();
	return 0;
}