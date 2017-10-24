#include "S57Reader.h"
#include <vector>
#include <fstream>
#include <list>
#include "tinyxml.h"  
#include "tinystr.h" 
#include <iostream>
#include <string>
#include "ogrsf_frmts.h"
#include <time.h>
#include <windows.h>

using namespace std;
typedef list<int> LISTINT;

// function: save geoInfo into xml format
static void SavetoXML(const char* pFilename, EleChartInfo Geolayer)
{
	cout << "  �ļ�������..." << endl;
	//����һ��XML���ĵ����󣬶�Ӧ��XML�������ĵ�
	TiXmlDocument doc;
	//��Ӧ��XML��Ԫ��
	TiXmlElement* msg;
	//comment;
	string s;
	//XML�е���������
	//TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");

	doc.LinkEndChild(decl);
	//����һ����Ԫ�ز�����
	TiXmlElement * root = new TiXmlElement("ENCInfo");
	doc.LinkEndChild(root);
	//ע��
	TiXmlComment * comment = new TiXmlComment();
	s = " GeographInfo for " + Geolayer.Chart_Name + " ";
	comment->SetValue(s.c_str());
	root->LinkEndChild(comment);

	// block: GeoInfo
	{
		TiXmlElement *chartNameElement = new TiXmlElement("Chart_Name");
		root->LinkEndChild(chartNameElement);
		TiXmlText *chartNameContent = new TiXmlText(Geolayer.Chart_Name.c_str());
		chartNameElement->LinkEndChild(chartNameContent);
		list<EncLayer> Geogralayerlist = Geolayer.Geolayerlist;
		list<EncLayer>::iterator iter;
		for (iter = Geogralayerlist.begin(); iter != Geogralayerlist.end(); iter++)
		{
			TiXmlElement * layersNode = new TiXmlElement("LayerInfo");
			root->LinkEndChild(layersNode);
			const EncLayer& w = *iter;  //wΪһ��layerʵ��
			TiXmlElement * window = new TiXmlElement("Layer");
			layersNode->LinkEndChild(window);
			//����Ԫ�ص�����
			window->SetAttribute("id", w.layer_ID);
			//����Ԫ�ز�����
			TiXmlElement *layerNameElement = new TiXmlElement("layerName");
			window->LinkEndChild(layerNameElement);
			TiXmlText *layerNameContent = new TiXmlText(w.layer_Name.c_str());
			layerNameElement->LinkEndChild(layerNameContent);
			list<GeoFeature> ww = w.GeoFeaturelist;  //Ҫ�ص�list
			{
				TiXmlElement * FeaturesNode = new TiXmlElement("Features");
				window->LinkEndChild(FeaturesNode);
				list<GeoFeature>::iterator featureiter;
				for (featureiter = ww.begin(); featureiter != ww.end(); featureiter++)
				{
					const GeoFeature& feature = *featureiter;

					TiXmlElement * FeatureEle = new TiXmlElement("Feature");
					FeaturesNode->LinkEndChild(FeatureEle);
					//����Ԫ�ص�����
					FeatureEle->SetAttribute("id", feature.Feature_ID);
					//����Ԫ�ز�����
					//Feature_Type
					TiXmlElement *TypeElement = new TiXmlElement("Type");
					FeatureEle->LinkEndChild(TypeElement);
					TiXmlText *TypeContent = new TiXmlText(feature.Feature_Type.c_str());
					TypeElement->LinkEndChild(TypeContent);
					//belong_layer
					TiXmlElement *belonglayerElement = new TiXmlElement("layer");
					FeatureEle->LinkEndChild(belonglayerElement);
					TiXmlText *belonglayerContent = new TiXmlText(to_string(feature.Feature_layer).c_str());
					belonglayerElement->LinkEndChild(belonglayerContent);
					//valdco
					TiXmlElement *valdcoElement = new TiXmlElement("valdco");
					FeatureEle->LinkEndChild(valdcoElement);
					TiXmlText *valdcoContent = new TiXmlText(to_string(feature.Feature_valdco).c_str());
					valdcoElement->LinkEndChild(valdcoContent);

					list<EleChartWaypoint> ww = feature.EleChartWaypointlist;  //���list
					{
						TiXmlElement * PointsNode = new TiXmlElement("wayPoints");
						FeatureEle->LinkEndChild(PointsNode);
						list<EleChartWaypoint>::iterator waypointiter;
						for (waypointiter = ww.begin(); waypointiter != ww.end(); waypointiter++)
						{
							const EleChartWaypoint& point = *waypointiter;

							TiXmlElement * waypointEle = new TiXmlElement("waypoint");
							PointsNode->LinkEndChild(waypointEle);
							//����Ԫ�ز�����
							TiXmlElement *IDElement = new TiXmlElement("id");
							waypointEle->LinkEndChild(IDElement);
							TiXmlText *IDContent = new TiXmlText(to_string(point.waypoint_ID).c_str());
							IDElement->LinkEndChild(IDContent);

							TiXmlElement *LngElement = new TiXmlElement("lon");
							waypointEle->LinkEndChild(LngElement);
							TiXmlText *LngContent = new TiXmlText(to_string(point.longitude).c_str());
							LngElement->LinkEndChild(LngContent);

							TiXmlElement *LatElement = new TiXmlElement("lat");
							waypointEle->LinkEndChild(LatElement);
							TiXmlText *LatContent = new TiXmlText(to_string(point.latitude).c_str());
							LatElement->LinkEndChild(LatContent);
						}
					}
				}
			}
			
		}
	}
	doc.SaveFile(pFilename);
}


// read ENC from S57File
void OpenS57File(string lpFileName, list<int> ObjlList, double filter_lllat, double filter_urlat, double filter_lllon, double filter_urlon)
{
	EleChartInfo eleChartInfo;
	list<GeoFeature> GeoFeature_list;
	list<EncLayer> GeoLayer_list;
	EncLayer encLayerInfo;
	OGRRegisterAll();  //ע��GDAL/OGR֧�ֵ����и�ʽ
	OGRDataSource  *poDS;
	CPLSetConfigOption("OGR_S57_OPTIONS", "SPLIT_MULTIPOINT=ON,ADD_SOUNDG_DEPTH=ON");
	poDS = OGRSFDriverRegistrar::Open(lpFileName.c_str(), FALSE);
	eleChartInfo.Chart_Name = "US1BS01M.000";
	if (poDS == NULL)
	{
		cout << "open FAILED!" << endl;
	}
	cout << "  �ļ���ȡ��..." << endl;
	const char *pStr = poDS->GetName(); //��ȡ.000�ļ�����·��
	//����ʱ�䴴��xml�ļ���
	SYSTEMTIME sTime;
	GetSystemTime(&sTime);
	string xmlfileName = to_string(sTime.wYear) + to_string(sTime.wMonth) + to_string(sTime.wDay) + "_" + to_string(sTime.wHour) + to_string(sTime.wMinute) + to_string(sTime.wSecond)+ "_" + "ENCResolution";
	string xmlFilePath = "E:\\" + xmlfileName + ".xml";

	OGRLayer  *poLayer;
	OGREnvelope *pEnvelope = new OGREnvelope;
	//һ��OGRDataSource���ܰ����ܶ�Ĳ㣬���� OGRDataSource::GetLayerCount()�õ�
	poLayer = poDS->GetLayer(0);
	//->�ṹ�����ָ��ĳ�Ա�������߳�Ա�����������ṹ��ָ��������Ա���������ķ�ʽ
	poLayer->GetExtent(pEnvelope);
	//�õ�ͼ���������
	double MaxX = pEnvelope->MaxX;
	double MaxY = pEnvelope->MaxY;
	double MinX = pEnvelope->MinX;
	double MinY = pEnvelope->MinY;

	delete pEnvelope;
	int objectName = 0;  //OBJL���
	int layerIndex = 0;
	double VALDCO;  //ˮ��
	int layerobjl = 0;
	//��ͼ��ֲ��ȡS57�е�����
	for (layerIndex = 0; layerIndex < poDS->GetLayerCount(); layerIndex++)
	{
		poLayer = poDS->GetLayer(layerIndex);   //�õ�S57����ͼ����Ϣ
		//��ȡ�������features
		OGRFeature *poFeature;		
		poLayer->ResetReading();   //�Ӳ�Ŀ�ͷ��ʼ��ȡfeature
		OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
		encLayerInfo.layer_Name = poFDefn->GetName();
		//����GetNextFeature�������е�feature
		while ((poFeature = poLayer->GetNextFeature()) != NULL)
		{
			int nOBJL=0;
			int pID = poFeature->GetFID();
			int iField;
			objectName = 0;
			//���һ��Ҫ��feature�������ֶ�field
			for (iField = 0; iField < poFDefn->GetFieldCount(); iField++)
			{
				//�õ���������
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
				//�ֶ�����
				string fieldtemp = poFieldDefn->GetNameRef();
				//�ֶ�����
				OGRFieldType fieldType = poFieldDefn->GetType();
				const char *pFieldTypeName = poFieldDefn->GetFieldTypeName(fieldType);
				string Typetemp = pFieldTypeName;  
				//�ֶ�ֵ
				string fieldTypeValue = "";
				if (poFieldDefn->GetType() == OFTInteger)  //���Ե�����������
					fieldTypeValue = to_string(poFeature->GetFieldAsInteger(iField));
				else if (poFieldDefn->GetType() == OFTReal) //���Ե������Ǹ�����
					fieldTypeValue = to_string(poFeature->GetFieldAsDouble(iField));
				else if (poFieldDefn->GetType() == OFTString) //���Ե��������ַ���
					fieldTypeValue = (poFeature->GetFieldAsString(iField));
				else
					fieldTypeValue = (poFeature->GetFieldAsString(iField));
			}

			int iOBJL = poFDefn->GetFieldIndex("OBJL");   //�ֶα��
			//��ö�Ӧ��OBJL��ͼ���ţ���½�ض�Ӧobjl=71
			layerobjl = poFeature->GetFieldAsInteger(iOBJL);
			encLayerInfo.layer_ID = layerobjl;  //��ͼ���ำ������
			//����list���Ƿ����iOBJL
			list<int>::iterator index = find(ObjlList.begin(), ObjlList.end(), layerobjl); //����
			if (index != ObjlList.end())
			{
				if (43 == poFeature->GetFieldAsInteger(iOBJL))
				{
					//Depth contour ��ˮ����
					VALDCO = poFeature->GetFieldAsDouble("VALDCO");
				}
				else
					VALDCO = -1;
			}
			else
				break;   //��������ͼ����м�����״����
			//��ȡ��������geometry,����x��y��z�����
			OGRGeometry *poGeometry;
			poGeometry = poFeature->GetGeometryRef();  //��ȡ��Ҫ�صļ�����״,����һ��ָ������OGRFeature���ڲ��������ݵ�ָ��
			if (poGeometry != NULL)
			{
				list<EleChartWaypoint> eleChartWaypoint_list;  //�����꼯��list
				GeoFeature geoFeature;
				geoFeature.Feature_ID = poFeature->GetFID();
				geoFeature.Feature_layer = layerIndex;  //ͼ����
				geoFeature.Feature_valdco = VALDCO;
				//ֻ����½����Ϣ 71:Land area
				pStr = poGeometry->getGeometryName();
				//InputToTxtFile(TxtFilePath, " Ҫ�ؼ�������: " +(string)(pStr));
				//int k = poGeometry->getGeometryType();
				if (poGeometry != NULL
					&& wkbFlatten(poGeometry->getGeometryType()) == wkbPoint)
				{
					geoFeature.Feature_Type = "Point";
					//������
					//wkbPlatten() ��һ��wkbPoint25D(����Z)ת��Ϊ����2D������(wkbPoint)
					OGRPoint *poPoint = (OGRPoint *)poGeometry;
					if (poPoint == NULL)
					{
						//InputToTxtFile(TxtFilePath, "  OGRLinePointΪnull��");
					}
					if (poPoint->getY()>filter_lllat &&poPoint->getY()<filter_urlat && poPoint->getX()>filter_lllon && poPoint->getX() < filter_urlon)
					{
						EleChartWaypoint eleChartWaypoint;
						eleChartWaypoint.waypoint_ID = 0;
						eleChartWaypoint.longitude = poPoint->getX();
						eleChartWaypoint.latitude = poPoint->getY();
						eleChartWaypoint_list.push_back(eleChartWaypoint);
					}
				}
				else if (poGeometry != NULL
					&& wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
				{
					geoFeature.Feature_Type = "Line";
					//��
					OGRLineString *poLine = (OGRLineString *)poGeometry;
					OGRPoint OgrPoint;
					for (int i = 0; i<poLine->getNumPoints(); i++)
					{
						poLine->getPoint(i, &OgrPoint);
						if (OgrPoint.getY()>filter_lllat && OgrPoint.getY()<filter_urlat && OgrPoint.getX()>filter_lllon && OgrPoint.getX() < filter_urlon)
						{
							EleChartWaypoint eleChartWaypoint;
							eleChartWaypoint.waypoint_ID = i;
							eleChartWaypoint.longitude = OgrPoint.getX();
							eleChartWaypoint.latitude = OgrPoint.getY();
							eleChartWaypoint_list.push_back(eleChartWaypoint);
						}
					}
				}
				else if (poGeometry != NULL
					&& wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
				{
					geoFeature.Feature_Type = "Polygon";
					//�����
					OGRPolygon *poPolygon = (OGRPolygon *)poGeometry;
					OGRLineString *poLine = poPolygon->getExteriorRing();  //��ȡ�ö���ε��⻷
					OGRPoint OgrPoint;
					for (int i = 0; i<poLine->getNumPoints(); i++)
					{
						poLine->getPoint(i, &OgrPoint);
						if (OgrPoint.getY()>filter_lllat && OgrPoint.getY()<filter_urlat && OgrPoint.getX()>filter_lllon && OgrPoint.getX() < filter_urlon)
						{
							EleChartWaypoint eleChartWaypoint;
							eleChartWaypoint.waypoint_ID = i;
							eleChartWaypoint.longitude = OgrPoint.getX();
							eleChartWaypoint.latitude= OgrPoint.getY();		
							eleChartWaypoint_list.push_back(eleChartWaypoint);
						}
					}
				}
				else
				{}
				geoFeature.EleChartWaypointlist = eleChartWaypoint_list;
				GeoFeature_list.push_back(geoFeature);  //Ҫ���е��list
			}//geo
			//�ͷ�feature����������GDAL����ɾ��������win32 heap����
			OGRFeature::DestroyFeature(poFeature);
			encLayerInfo.GeoFeaturelist=(GeoFeature_list);   //Ҫ�صļ���
		} //Feature
		eleChartInfo.Geolayerlist.push_back(encLayerInfo);   //ͼ��ļ���
	} //ͼ��
	//ɾ�������ļ�һ��ͼ��Ӷ��ر�������ļ�
	OGRDataSource::DestroyDataSource(poDS);
	SavetoXML(xmlFilePath.c_str(), eleChartInfo);  //���浽XML
}