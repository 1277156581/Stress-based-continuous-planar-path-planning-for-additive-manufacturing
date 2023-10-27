#include<iostream>
#include<OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>  
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh\Core\Utils\Property.hh> 
#include <string>
#include <vector>
#include <algorithm>
#include<stdio.h>
#include "clipper.hpp"  
#include"clipper.cpp"
#include<GL/glut.h>
//#include"GA.h"
#define use_int32
#define pi 3.1415926
using namespace std;
using namespace ClipperLib;

#define M 700
#define Die 5000
int  die = Die;

using namespace std;
struct group {
	vector<int>id;
	double p;
	double fit;
	double sum_p;
};
vector<vector<double>>dis;//�������
vector<int> groupbest;//ÿ�������Ⱦɫ��
vector<group>groups;//Ⱦɫ������
double d1=0.56,d2=0.4;//·�����

int generation;
double groupbestp;//���Ž��p
double groupbestfit;//���Ž��fit
int changebest;//�Ƿ������Ž��������Ⱥ
double distancenum;



struct element {//��Ԫ�ṹ�壬����洢��ÿ����Ԫ��ȫ����Ϣ
	int id;
	double x, y, z, s11, s22, s12, s_max, direction;
	int b;
};
struct points {//�洢boolΪ1 �ĵ�Ԫ��Ϣ
	int id;
	double x, y, z, s_max, direction;
	int b = 0;//�õ�Ԫ�Ķ���
};
struct path {//·���ṹ�壬ÿ��·������һ��˫��
	int id;
	double x, y, z, direction;
	int b = 0;//�жϸ�·�����Ƿ�Ϊ����㣬���г���Ϊ-1��һ��ĵ�Ϊ0,���Ϊ1
};



vector<element>point_buffer1;//��һ�㻺��
vector<vector<element>>point_buffer2;//�ڶ��㻺��
vector<vector<vector<element>>>point;//ÿ����Ԫ����Ϣ
vector<vector<points>>model;//��Ӧ����С������Ч��Ԫ�������򣬴�->С,һά�������
vector<points>model_buffer;
vector<vector<vector<path>>>lines;
vector<vector<vector<path>>>paths;//�洢·����һά�������
vector<path>paths_buffer1;
vector<vector<path>>paths_buffer2;
vector<vector<vector<path>>>paths_buffer3;
vector<vector<vector<path>>>path_w;//������
queue<points> que;

vector<vector<vector<int>>>G;//�ڽӾ���һάΪ��
vector<int>G_buffer1;
vector<vector<int>>G_buffer2;
vector<vector<int>>visited;//��������
vector<int>arr_buffer1;
vector<vector<int>>arr;//�洢��ÿ��·���ı���ѭ��
//int num=0;//����
int x_count, y_count, z_count;//�ֱ��ʾx��y,z�ĵ�Ԫ����
double Bmin_x, Bmin_y, Bmax_x, Bmax_y;

void ReadFile() {
	element e;
	ifstream file1, file2,file3;
	file1.open("VoxelizationData.txt");//��ȡ�����ļ�
	file2.open("abaqus.txt");//��ȡӦ���ļ�
	file3.open("path_w.txt");
	double a1, a2;
	int q1 = 25095;//����
	path s;
	/*for (int i = 0; i < q1; i++) {
		file3 >> a1;
		file3 >> a2;
		if (a1 != 9999 && a2 != 9999) {
			s.x = a1;
			s.y = a2;
			paths_buffer1.push_back(s);
		}
		else {
			path_w.push_back(paths_buffer1);
			paths_buffer1.clear();
		}
	}
	path_w.push_back(paths_buffer1);
	paths_buffer1.clear();*/
	for (int i = 0; i < q1; i++) {
		file3 >> a1;
		file3 >> a2;
		if ((a1 != 8888 && a2 != 8888) && (a1 != 9999 && a2 != 9999)) {
			s.x = a1;
			s.y = a2;
			paths_buffer1.push_back(s);
		}
		else if (a1 == 9999 && a2 == 9999) {
			paths_buffer2.push_back(paths_buffer1);
			paths_buffer1.clear();
		}
		else {
			paths_buffer2.push_back(paths_buffer1);
			paths_buffer1.clear();
			path_w.push_back(paths_buffer2);
			paths_buffer2.clear();
		}
	}
	paths_buffer1.clear();
	paths_buffer2.clear();

	file1 >> x_count;
	file1 >> y_count;
	file1 >> z_count;
	printf("%d %d %d \n", x_count, y_count, z_count);
	for (int i = 0; i < z_count; i++) {//��ÿ����Ԫ��������Ϣ��������
		for (int j = 0; j < y_count; j++) {
			for (int m = 0; m < x_count; m++) {
				for (int s = 0; s < 5; s++) {
					switch (s) {
					case 0:file1 >> e.id; break;
					case 1:file1 >> e.x; break;
					case 2:file1 >> e.y; break;
					case 3:file1 >> e.z; break;
					case 4:file1 >> e.b; break;
					}
				}
				point_buffer1.push_back(e);
			}
			point_buffer2.push_back(point_buffer1);
			point_buffer1.clear();
		}
		point.push_back(point_buffer2);
		point_buffer2.clear();
	}
	for (int i = 0; i < point.size(); i++) {//��ȡ��Ԫ��x,yӦ������
		for (int j = 0; j < point[0].size(); j++) {
			for (int m = 0; m < point[0][0].size(); m++) {
				if (point[i][j][m].b == 1) {
					for (int s = 0; s < 4; s++) {
						switch (s) {
						case 0: {file2 >> e.id; if (e.id != point[i][j][m].id) printf("�ļ���ȡ����\n"); break; }
						case 1:file2 >> point[i][j][m].s11; break;
						case 2:file2 >> point[i][j][m].s22; break;
						case 3:file2 >> point[i][j][m].s12; break;
						}
					}
				}
			}
		}
	}
}

double distance(path a, path b)	//��������
{
	return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

void FE_Analysis() {//���������Ӧ���Լ������Ӧ������
	double s1, s2, d1, d2;
	for (int i = 0; i < point.size(); i++) {
		for (int j = 0; j < point[i].size(); j++) {
			for (int m = 0; m < point[i][j].size(); m++) {
				if (point[i][j][m].b == 1) {
					//�����Ӧ����ֵ��abs��
					s1 = ((point[i][j][m].s11 + point[i][j][m].s22) / (double)2) - sqrt(pow(((point[i][j][m].s11 - point[i][j][m].s22) / (double)2), 2) + pow(point[i][j][m].s12, 2));
					s2 = ((point[i][j][m].s11 + point[i][j][m].s22) / (double)2) + sqrt(pow(((point[i][j][m].s11 - point[i][j][m].s22) / (double)2), 2) + pow(point[i][j][m].s12, 2));
					if (fabs(s1) >= fabs(s2)) {
						point[i][j][m].s_max = s1;
					}
					else {
						point[i][j][m].s_max = s2;
					}
					//�����Ӧ������
					d1 = atan(-(2 * point[i][j][m].s12) / (point[i][j][m].s11 - point[i][j][m].s22));
					if (point[i][j][m].s11 >= point[i][j][m].s22) {
						if (d1 <= 0) d1 = d1 + 2 * pi;
						
					}
					else if (point[i][j][m].s11 <= point[i][j][m].s22) {
						d1 = d1 + pi;
					}
					d1 = d1 / (double)2;
					//��С��Ӧ������
					d2= atan(-(2 * point[i][j][m].s12) / (point[i][j][m].s11 - point[i][j][m].s22));
					if (point[i][j][m].s11 >= point[i][j][m].s22) {
						d2 = d2 + pi;
					}
					else if (point[i][j][m].s11 <= point[i][j][m].s11) {
						if (d2 <= 0) d2 = d2 + 2 * pi;
					}
					d2 = d2 / (double)2;
					//�����Ӧ����abs������
					if ((s1 > s2 && fabs(s1) > fabs(s2)) || (s2 > s1 && fabs(s2) > fabs(s1))) point[i][j][m].direction = pi-d1;
					else if ((s1 > s2 && fabs(s1) < fabs(s2)) || (s2 > s1 && fabs(s2) < fabs(s1))) point[i][j][m].direction =pi- d2;
				}
			}
		}
	}
}

void VectorInsert(int i,int m) {//���ɵ�·������
	bool flag = true;
	path s;
	int Right_up, Left_up, Right_down, Left_down;
	int q=0;
	while (flag) {
		q++;
		flag = false;
		Right_up = 0;
		Left_up = 0;
		Right_down = 0;
		Left_down = 0;
			for (int c = 0; c < paths_buffer2.size(); c++) {
					for (int n = 1; n < paths_buffer2[c].size(); n++) {
						if ((model[i][m].id + 1 == paths_buffer2[c][n - 1].id && model[i][m].id + x_count == paths_buffer2[c][n].id && model[i][m].id % x_count != 0) ||
							(model[i][m].id + 1 == paths_buffer2[c][n].id && model[i][m].id + x_count == paths_buffer2[c][n - 1].id && model[i][m].id % x_count != 0)) {
							Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
						}
						if ((model[i][m].id - 1 == paths_buffer2[c][n - 1].id && model[i][m].id + x_count == paths_buffer2[c][n].id && model[i][m].id % x_count != 1) ||
							(model[i][m].id - 1 == paths_buffer2[c][n].id && model[i][m].id + x_count == paths_buffer2[c][n - 1].id && model[i][m].id % x_count != 1)) {
							Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
						}
						if ((model[i][m].id + 1 == paths_buffer2[c][n - 1].id && model[i][m].id - x_count == paths_buffer2[c][n].id && model[i][m].id % x_count != 0) ||
							(model[i][m].id + 1 == paths_buffer2[c][n].id && model[i][m].id - x_count == paths_buffer2[c][n - 1].id && model[i][m].id % x_count != 0)) {
							Right_down = 1;//��������µĶ˵����ӣ���������е�·������
						}
						if ((model[i][m].id - 1 == paths_buffer2[c][n - 1].id && model[i][m].id - x_count == paths_buffer2[c][n].id && model[i][m].id % x_count != 1) ||
							(model[i][m].id - 1 == paths_buffer2[c][n].id && model[i][m].id - x_count == paths_buffer2[c][n - 1].id && model[i][m].id % x_count != 1)) {
							Left_down = 1;//��������µĶ˵����ӣ���������е�·������
						}
					}
			}
		if (model[i][m].direction >= 0 && model[i][m].direction < pi / (double)8) {
			
			for (int j = 0; j < model[i].size(); j++) {
				if (((model[i][j].id == model[i][m].id + 1 && model[i][m].id % x_count != 0) || (model[i][j].id == model[i][m].id - 1 && model[i][m].id % x_count != 1)) && model[i][j].b == 0) {//�ұߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					//flag = true;
					if (model[i][m].direction >= 0 && model[i][m].direction < pi / (double)8)
					{
						flag = true;
					}
					break;
				}
			}
		}
		else if (model[i][m].direction >= pi / (double)8 && model[i][m].direction < pi * ((double)3 / (double)8)) {
			for (int j = 0; j < model[i].size(); j++) {
				if (((model[i][j].id == model[i][m].id + x_count + 1 && model[i][m].id % x_count != 0 && Right_up == 0) || (model[i][j].id == model[i][m].id - x_count - 1 && model[i][m].id % x_count != 1 && Left_down == 0))  && model[i][j].b == 0) {//���ϱߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					//flag = true;
					if (model[i][m].direction >= pi / (double)8 && model[i][m].direction < pi * ((double)3 / (double)8))
					{
						flag = true;
					}
					
					break;
				}
			}
		}
		else if (model[i][m].direction >= pi * ((double)3 / (double)8) && model[i][m].direction < pi * ((double)5 / (double)8)) {
			for (int j = 0; j < model[i].size(); j++) {
				if ((model[i][j].id == model[i][m].id + x_count || model[i][j].id == model[i][m].id - x_count) && model[i][j].b == 0) {//�ϱߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					//flag = true;
					if (model[i][m].direction >= pi * ((double)3 / (double)8) && model[i][m].direction < pi * ((double)5 / (double)8))
					{
						flag = true;
					}
					
					break;
				}
			}
		}
		else if (model[i][m].direction >= pi * ((double)5 / (double)8) && model[i][m].direction < pi * ((double)7 / (double)8)) {
			for (int j = 0; j < model[i].size(); j++) {
				if (((model[i][j].id == model[i][m].id + x_count - 1 && model[i][m].id % x_count != 1 && Left_up == 0) || (model[i][j].id == model[i][m].id - x_count + 1 && model[i][m].id % x_count != 0 && Right_down == 0)) && model[i][j].b == 0) {//���ϱߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					//flag = true;
					if (model[i][m].direction >= pi * ((double)5 / (double)8) && model[i][m].direction < pi * ((double)7 / (double)8))
					{
						flag = true;
					}
					
					break;
				}
			}
		}
		else if (model[i][m].direction >= pi * ((double)7 / (double)8) && model[i][m].direction < pi) {
			for (int j = 0; j < model[i].size(); j++) {
				if (((model[i][j].id == model[i][m].id - 1 && model[i][m].id % x_count != 1) || (model[i][j].id == model[i][m].id + 1 && model[i][m].id % x_count != 0)) && model[i][j].b == 0) {//��ߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					//flag = true;
					if (model[i][m].direction >= pi * ((double)7 / (double)8) && model[i][m].direction < pi)
					{
						flag = true;
					}
					
					break;
				}
			}
		}
	}
}

void OptimizePoints() {//�Ż��յ㺯��
	path s;
	int Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
	int Left_up = 0;
	int Right_down = 0;
	int Left_down = 0;
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
				Left_up = 0;
				Right_down = 0;
				Left_down = 0;
				for (int m = 0; m < paths[i].size(); m++) {
					for (int n = 1; n < paths[i][m].size(); n++) {
						if ((model[i][j].id + 1 == paths[i][m][n - 1].id && model[i][j].id + x_count == paths[i][m][n].id && model[i][j].id % x_count != 0) ||
							(model[i][j].id + 1 == paths[i][m][n].id && model[i][j].id + x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 0)) {
							Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
						}
						if ((model[i][j].id - 1 == paths[i][m][n - 1].id && model[i][j].id + x_count == paths[i][m][n].id && model[i][j].id % x_count != 1) ||
							(model[i][j].id - 1 == paths[i][m][n].id && model[i][j].id + x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 1)) {
							Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
						}
						if ((model[i][j].id + 1 == paths[i][m][n - 1].id && model[i][j].id - x_count == paths[i][m][n].id && model[i][j].id % x_count != 0) ||
							(model[i][j].id + 1 == paths[i][m][n].id && model[i][j].id - x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 0)) {
							Right_down = 1;//��������µĶ˵����ӣ���������е�·������
						}
						if ((model[i][j].id - 1 == paths[i][m][n - 1].id && model[i][j].id - x_count == paths[i][m][n].id && model[i][j].id % x_count != 1) ||
							(model[i][j].id - 1 == paths[i][m][n].id && model[i][j].id - x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 1)) {
							Left_down = 1;//��������µĶ˵����ӣ���������е�·������
						}
					}
				}

				for (int m = 0; m < paths[i].size(); m++) {
					//��-�յ�
					if (paths[i][m][paths[i][m].size() - 2].id == paths[i][m][paths[i][m].size() - 1].id + 1 && paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 && model[i][j].id % x_count != 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;//Ѱ�ҵ����ӵ�·�����˳��ÿյ��ѭ��
					}//��-�յ�
					else if (paths[i][m][paths[i][m].size() - 2].id == paths[i][m][paths[i][m].size() - 1].id + x_count && paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + x_count) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//��-�յ�
					else if (paths[i][m][paths[i][m].size() - 2].id == paths[i][m][paths[i][m].size() - 1].id - 1 && paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 && model[i][j].id % x_count != 1) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//��-�յ�
					else if (paths[i][m][paths[i][m].size() - 2].id == paths[i][m][paths[i][m].size() - 1].id - x_count && paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - x_count) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//����-�յ�
					else if (paths[i][m][paths[i][m].size() - 2].id == paths[i][m][paths[i][m].size() - 1].id + 1 + x_count && paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && Right_up == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//����-�յ�
					else if (paths[i][m][paths[i][m].size() - 2].id == paths[i][m][paths[i][m].size() - 1].id - 1 + x_count && paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && Left_up == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//����-�յ�
					else if (paths[i][m][paths[i][m].size() - 2].id == paths[i][m][paths[i][m].size() - 1].id - 1 - x_count && paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && Left_down == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//����-�յ�
					else if (paths[i][m][paths[i][m].size() - 2].id == paths[i][m][paths[i][m].size() - 1].id + 1 - x_count && paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && Right_down == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//��-���
					else if (paths[i][m][1].id == paths[i][m][0].id + 1 && paths[i][m][0].id == model[i][j].id + 1 && model[i][j].id % x_count != 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//��-���
					else if (paths[i][m][1].id == paths[i][m][0].id + x_count && paths[i][m][0].id == model[i][j].id + x_count) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//��-���
					else if (paths[i][m][1].id == paths[i][m][0].id - 1 && paths[i][m][0].id == model[i][j].id - 1 && model[i][j].id % x_count != 1) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//��-���
					else if (paths[i][m][1].id == paths[i][m][0].id - x_count && paths[i][m][0].id == model[i][j].id - x_count) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//����-���
					else if (paths[i][m][1].id == paths[i][m][0].id + 1 + x_count && paths[i][m][0].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && Right_up == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//����-���
					else if (paths[i][m][1].id == paths[i][m][0].id - 1 + x_count && paths[i][m][0].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && Left_up == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//����-���
					else if (paths[i][m][1].id == paths[i][m][0].id - 1 - x_count && paths[i][m][0].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && Left_down == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//����-���
					else if (paths[i][m][1].id == paths[i][m][0].id + 1 - x_count && paths[i][m][0].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && Right_down == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}


					////��-�յ�
					//if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 && model[i][j].id % x_count != 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	j = 0;
					//	break;//Ѱ�ҵ����ӵ�·�����˳��ÿյ��ѭ��
					//}//��-�յ�
					//else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + x_count) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	j = 0;
					//	break;
					//}//��-�յ�
					//else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 && model[i][j].id % x_count != 1) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	j = 0;
					//	break;
					//}//��-�յ�
					//else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - x_count) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	j = 0;
					//	break;
					//}//����-�յ�
					//else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && Right_up == 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	j = 0;
					//	break;
					//}//����-�յ�
					//else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && Left_up == 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	j = 0;
					//	break;
					//}//����-�յ�
					//else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && Left_down == 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	j = 0;
					//	break;
					//}//����-�յ�
					//else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && Right_down == 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	j = 0;
					//	break;
					//}//��-���
					//else if (paths[i][m][0].id == model[i][j].id + 1 && model[i][j].id % x_count != 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	for (int f = paths[i][m].size() - 1; f > 0; f--) {
					//		s = paths[i][m][f];
					//		paths[i][m][f] = paths[i][m][f - 1];
					//		paths[i][m][f - 1] = s;
					//	}
					//	j = 0;
					//	break;
					//}//��-���
					//else if (paths[i][m][0].id == model[i][j].id + x_count) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	for (int f = paths[i][m].size() - 1; f > 0; f--) {
					//		s = paths[i][m][f];
					//		paths[i][m][f] = paths[i][m][f - 1];
					//		paths[i][m][f - 1] = s;
					//	}
					//	j = 0;
					//	break;
					//}//��-���
					//else if (paths[i][m][0].id == model[i][j].id - 1 && model[i][j].id % x_count != 1) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	for (int f = paths[i][m].size() - 1; f > 0; f--) {
					//		s = paths[i][m][f];
					//		paths[i][m][f] = paths[i][m][f - 1];
					//		paths[i][m][f - 1] = s;
					//	}
					//	j = 0;
					//	break;
					//}//��-���
					//else if (paths[i][m][0].id == model[i][j].id - x_count) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	for (int f = paths[i][m].size() - 1; f > 0; f--) {
					//		s = paths[i][m][f];
					//		paths[i][m][f] = paths[i][m][f - 1];
					//		paths[i][m][f - 1] = s;
					//	}
					//	j = 0;
					//	break;
					//}//����-���
					//else if (paths[i][m][0].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && Right_up == 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	for (int f = paths[i][m].size() - 1; f > 0; f--) {
					//		s = paths[i][m][f];
					//		paths[i][m][f] = paths[i][m][f - 1];
					//		paths[i][m][f - 1] = s;
					//	}
					//	j = 0;
					//	break;
					//}//����-���
					//else if (paths[i][m][0].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && Left_up == 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	for (int f = paths[i][m].size() - 1; f > 0; f--) {
					//		s = paths[i][m][f];
					//		paths[i][m][f] = paths[i][m][f - 1];
					//		paths[i][m][f - 1] = s;
					//	}
					//	j = 0;
					//	break;
					//}//����-���
					//else if (paths[i][m][0].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && Left_down == 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	for (int f = paths[i][m].size() - 1; f > 0; f--) {
					//		s = paths[i][m][f];
					//		paths[i][m][f] = paths[i][m][f - 1];
					//		paths[i][m][f - 1] = s;
					//	}
					//	j = 0;
					//	break;
					//}//����-���
					//else if (paths[i][m][0].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && Right_down == 0) {
					//	model[i][j].b++;
					//	s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					//	paths[i][m].push_back(s);
					//	for (int f = paths[i][m].size() - 1; f > 0; f--) {
					//		s = paths[i][m][f];
					//		paths[i][m][f] = paths[i][m][f - 1];
					//		paths[i][m][f - 1] = s;
					//	}
					//	j = 0;
					//	break;
					//}


				}
			}
		}
	}
}

void PathConnect() {
	paths_buffer1.clear();
	path s;
	int Right_up, Left_up, Right_down, Left_down;
	///////////////////////���ݷ����Ƚ��кϲ����յ�-��㣩
	for (int i = 0; i < paths.size(); i++) {

	
	///////////ͬб�ʵĺϲ�
	

		for (int j = 0; j < paths[i].size(); j++) {
			Right_up = 0;
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int c = 0; c < paths[i].size(); c++) {
				for (int n = 1; n < paths[i][c].size(); n++) {
					if ((paths[i][j][0].id + 1 == paths[i][c][n - 1].id && paths[i][j][0].id + x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 0) ||
						(paths[i][j][0].id + 1 == paths[i][c][n].id && paths[i][j][0].id + x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id - 1 == paths[i][c][n - 1].id && paths[i][j][0].id + x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 1) ||
						(paths[i][j][0].id - 1 == paths[i][c][n].id && paths[i][j][0].id + x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id + 1 == paths[i][c][n - 1].id && paths[i][j][0].id - x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 0) ||
						(paths[i][j][0].id + 1 == paths[i][c][n].id && paths[i][j][0].id - x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id - 1 == paths[i][c][n - 1].id && paths[i][j][0].id - x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 1) ||
						(paths[i][j][0].id - 1 == paths[i][c][n].id && paths[i][j][0].id - x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {

				if (m == j) m++;
				if (m == paths[i].size()) break;
				//���-���
				if (paths[i][m][1].id  == 2*paths[i][m][0].id - paths[i][j][0].id && paths[i][m][0].id ==  2*paths[i][j][0].id - paths[i][j][1].id) {
					if ((paths[i][m][0].id == paths[i][j][0].id + 1 + x_count && Right_up == 1) || (paths[i][m][0].id == paths[i][j][0].id - 1 + x_count && Left_up == 1) || (paths[i][m][0].id == paths[i][j][0].id - 1 - x_count && Left_down == 1) || (paths[i][m][0].id == paths[i][j][0].id + 1 - x_count && Right_down == 1))
						break;
					if ((paths[i][j][0].id % x_count == 0 && (paths[i][m][0].id == paths[i][j][0].id + 1 || paths[i][m][0].id == paths[i][j][0].id + 1 + x_count || paths[i][m][0].id == paths[i][j][0].id + 1 - x_count)) || (paths[i][j][0].id % x_count == 1 && (paths[i][m][0].id == paths[i][j][0].id - 1 || paths[i][m][0].id == paths[i][j][0].id - 1 + x_count || paths[i][m][0].id == paths[i][j][0].id - 1 - x_count)))
						break;
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//���-�յ�
				if (paths[i][m][paths[i][m].size() - 2].id  == 2*paths[i][m][paths[i][m].size() - 1].id - paths[i][j][0].id && paths[i][m][paths[i][m].size() - 1].id == 2*paths[i][j][0].id - paths[i][j][1].id ) {
					if ((paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 + x_count && Right_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 + x_count && Left_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 - x_count && Left_down == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 - x_count && Right_down == 1))
						break;
					if ((paths[i][j][0].id % x_count == 0 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 - x_count)) || (paths[i][j][0].id % x_count == 1 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 - x_count)))
						break;
					paths[i][m].insert(paths[i][m].end(), paths[i][j].begin(), paths[i][j].end());
					paths[i].erase(paths[i].begin() + j);
					m = 0; j = 0;
					break;
				}
			}
		}

		

		for (int j = 0; j < paths[i].size(); j++) {
			Right_up = 0;//�жϸÿյ��б�����Ƿ������
			//��������Ϊ0����������Ϊ1
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int c = 0; c < paths[i].size(); c++) {
				for (int n = 1; n < paths[i][c].size(); n++) {
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {



				if (m == j) m++;
				if (m == paths[i].size()) break;
				////�յ�-�յ�
				if (paths[i][m][paths[i][m].size() - 2].id==2*paths[i][m][paths[i][m].size() - 1].id - paths[i][j][paths[i][j].size() - 1].id && paths[i][m][paths[i][m].size() - 1].id== 2*paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id ) {
					if ((paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && Right_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count && Left_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && Left_down == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count && Right_down == 1))
						break;
					if ((paths[i][j][paths[i][j].size() - 1].id % x_count == 0 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count)) || (paths[i][j][paths[i][j].size() - 1].id % x_count == 1 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count)))
						break;
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					j = 0; m = 0;
					break;
				}
				////�յ�-���
				if (paths[i][m][1].id ==2* paths[i][m][0].id - paths[i][j][paths[i][j].size() - 1].id && paths[i][m][0].id== 2*paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id ) {
					if ((paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && Right_up == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count && Left_up == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && Left_down == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count && Right_down == 1))
						break;
					if ((paths[i][j][paths[i][j].size() - 1].id % x_count == 0 && (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count)) || (paths[i][j][paths[i][j].size() - 1].id % x_count == 1 && (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count)))
						break;
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
			}
		}
		//�ϲ���ͬб�ʵ�·��
		for (int j = 0; j < paths[i].size(); j++) {
			int right_up = 0;
			int left_up = 0;
			int right_down = 0;
			int left_down = 0;
			for (int c = 0; c < paths[i].size(); c++) {
				for (int n = 1; n < paths[i][c].size(); n++) {
					if ((paths[i][j][0].id + 1 == paths[i][c][n - 1].id && paths[i][j][0].id + x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 0) ||
						(paths[i][j][0].id + 1 == paths[i][c][n].id && paths[i][j][0].id + x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 0)) {
						right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id - 1 == paths[i][c][n - 1].id && paths[i][j][0].id + x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 1) ||
						(paths[i][j][0].id - 1 == paths[i][c][n].id && paths[i][j][0].id + x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 1)) {
						left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id + 1 == paths[i][c][n - 1].id && paths[i][j][0].id - x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 0) ||
						(paths[i][j][0].id + 1 == paths[i][c][n].id && paths[i][j][0].id - x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 0)) {
						right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id - 1 == paths[i][c][n - 1].id && paths[i][j][0].id - x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 1) ||
						(paths[i][j][0].id - 1 == paths[i][c][n].id && paths[i][j][0].id - x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 1)) {
						left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {

				if (m == j) m++;
				if (m == paths[i].size()) break;
				//���-���
				if (abs(paths[i][m][1].id - paths[i][m][0].id) != abs(paths[i][j][0].id - paths[i][j][1].id) && paths[i][m][0].id == 2 * paths[i][j][0].id - paths[i][j][1].id) {
					if ((paths[i][m][0].id == paths[i][j][0].id + 1 + x_count && right_up == 1) || (paths[i][m][0].id == paths[i][j][0].id - 1 + x_count && left_up == 1) || (paths[i][m][0].id == paths[i][j][0].id - 1 - x_count && left_down == 1) || (paths[i][m][0].id == paths[i][j][0].id + 1 - x_count && right_down == 1))
						break;
					if ((paths[i][j][0].id % x_count == 0 && (paths[i][m][0].id == paths[i][j][0].id + 1 || paths[i][m][0].id == paths[i][j][0].id + 1 + x_count || paths[i][m][0].id == paths[i][j][0].id + 1 - x_count)) || (paths[i][j][0].id % x_count == 1 && (paths[i][m][0].id == paths[i][j][0].id - 1 || paths[i][m][0].id == paths[i][j][0].id - 1 + x_count || paths[i][m][0].id == paths[i][j][0].id - 1 - x_count)))
						break;
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//���-�յ�
				if (abs(paths[i][m][paths[i][m].size() - 2].id - paths[i][m][paths[i][m].size() - 1].id) != abs(paths[i][j][0].id - paths[i][j][1].id) && paths[i][m][paths[i][m].size() - 1].id == 2 * paths[i][j][0].id - paths[i][j][1].id) {
					if ((paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 + x_count && right_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 + x_count && left_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 - x_count && left_down == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 - x_count && right_down == 1))
						break;
					if ((paths[i][j][0].id % x_count == 0 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 - x_count)) || (paths[i][j][0].id % x_count == 1 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 - x_count)))
						break;
					paths[i][m].insert(paths[i][m].end(), paths[i][j].begin(), paths[i][j].end());
					paths[i].erase(paths[i].begin() + j);
					m = 0; j = 0;
					break;
				}
			}
		}



		for (int j = 0; j < paths[i].size(); j++) {
			int right_up = 0;//�жϸÿյ��б�����Ƿ������
			//��������Ϊ0����������Ϊ1
			int left_up = 0;
			int right_down = 0;
			int left_down = 0;
			for (int c = 0; c < paths[i].size(); c++) {
				for (int n = 1; n < paths[i][c].size(); n++) {
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {



				if (m == j) m++;
				if (m == paths[i].size()) break;
				////�յ�-�յ�
				if (abs(paths[i][m][paths[i][m].size() - 2].id - paths[i][m][paths[i][m].size() - 1].id) != abs(paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id) && paths[i][m][paths[i][m].size() - 1].id == 2 * paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id) {
					if ((paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && right_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count && left_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && left_down == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count && right_down == 1))
						break;
					if ((paths[i][j][paths[i][j].size() - 1].id % x_count == 0 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count)) || (paths[i][j][paths[i][j].size() - 1].id % x_count == 1 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count)))
						break;
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					j = 0; m = 0;
					break;
				}
				////�յ�-���
				if (abs(paths[i][m][1].id - paths[i][m][0].id) != abs(paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id) && paths[i][m][0].id == 2 * paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id) {
					if ((paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && right_up == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count && left_up == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && left_down == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count && right_down == 1))
						break;
					if ((paths[i][j][paths[i][j].size() - 1].id % x_count == 0 && (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count)) || (paths[i][j][paths[i][j].size() - 1].id % x_count == 1 && (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count)))
						break;
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
			}
		}
		

		//OptimizePoints();
		///////////////////////////////////////////////////
		for (int j = 0; j < model[i].size(); j++) {
		if (model[i][j].b == 0) {
			Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int m = 0; m < paths[i].size(); m++) {
				for (int n = 1; n < paths[i][m].size(); n++) {
					if ((model[i][j].id + 1 == paths[i][m][n - 1].id && model[i][j].id + x_count == paths[i][m][n].id && model[i][j].id % x_count != 0) ||
						(model[i][j].id + 1 == paths[i][m][n].id && model[i][j].id + x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((model[i][j].id - 1 == paths[i][m][n - 1].id && model[i][j].id + x_count == paths[i][m][n].id && model[i][j].id % x_count != 1) ||
						(model[i][j].id - 1 == paths[i][m][n].id && model[i][j].id + x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((model[i][j].id + 1 == paths[i][m][n - 1].id && model[i][j].id - x_count == paths[i][m][n].id && model[i][j].id % x_count != 0) ||
						(model[i][j].id + 1 == paths[i][m][n].id && model[i][j].id - x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((model[i][j].id - 1 == paths[i][m][n - 1].id && model[i][j].id - x_count == paths[i][m][n].id && model[i][j].id % x_count != 1) ||
						(model[i][j].id - 1 == paths[i][m][n].id && model[i][j].id - x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}

			for (int m = 0; m < paths[i].size(); m++) {

				//��-�յ�
				if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 && model[i][j].id % x_count != 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					j = 0;
					break;//Ѱ�ҵ����ӵ�·�����˳��ÿյ��ѭ��
				}//��-�յ�
				else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + x_count) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					j = 0;
					break;
				}//��-�յ�
				else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 && model[i][j].id % x_count != 1) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					j = 0;
					break;
				}//��-�յ�
				else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - x_count) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					j = 0;
					break;
				}//����-�յ�
				else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && Right_up == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					j = 0;
					break;
				}//����-�յ�
				else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && Left_up == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					j = 0;
					break;
				}//����-�յ�
				else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && Left_down == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					j = 0;
					break;
				}//����-�յ�
				else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && Right_down == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					j = 0;
					break;
				}//��-���
				else if (paths[i][m][0].id == model[i][j].id + 1 && model[i][j].id % x_count != 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
				}//��-���
				else if (paths[i][m][0].id == model[i][j].id + x_count) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
				}//��-���
				else if (paths[i][m][0].id == model[i][j].id - 1 && model[i][j].id % x_count != 1) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
				}//��-���
				else if (paths[i][m][0].id == model[i][j].id - x_count) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
				}//����-���
				else if (paths[i][m][0].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && Right_up == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
				}//����-���
				else if (paths[i][m][0].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && Left_up == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
				}//����-���
				else if (paths[i][m][0].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && Left_down == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
				}//����-���
				else if (paths[i][m][0].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && Right_down == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
				}


			}
		}
		}
		//���ϲ�
		for (int j = 0; j < paths[i].size(); j++) {
			Right_up = 0;
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int c = 0; c < paths[i].size(); c++) {
				for (int n = 1; n < paths[i][c].size(); n++) {
					if ((paths[i][j][0].id + 1 == paths[i][c][n - 1].id && paths[i][j][0].id + x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 0) ||
						(paths[i][j][0].id + 1 == paths[i][c][n].id && paths[i][j][0].id + x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id - 1 == paths[i][c][n - 1].id && paths[i][j][0].id + x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 1) ||
						(paths[i][j][0].id - 1 == paths[i][c][n].id && paths[i][j][0].id + x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id + 1 == paths[i][c][n - 1].id && paths[i][j][0].id - x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 0) ||
						(paths[i][j][0].id + 1 == paths[i][c][n].id && paths[i][j][0].id - x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id - 1 == paths[i][c][n - 1].id && paths[i][j][0].id - x_count == paths[i][c][n].id && paths[i][j][0].id % x_count != 1) ||
						(paths[i][j][0].id - 1 == paths[i][c][n].id && paths[i][j][0].id - x_count == paths[i][c][n - 1].id && paths[i][j][0].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {

				if (m == j) m++;
				if (m == paths[i].size()) break;
				//���-���
				if ((abs(paths[i][m][0].id - paths[i][j][0].id) == 1|| abs(paths[i][m][0].id - paths[i][j][0].id) == x_count|| abs(paths[i][m][0].id - paths[i][j][0].id) == x_count-1|| abs(paths[i][m][0].id - paths[i][j][0].id) == x_count+1)) {
					if ((paths[i][m][0].id == paths[i][j][0].id + 1 + x_count && Right_up == 1) || (paths[i][m][0].id == paths[i][j][0].id - 1 + x_count && Left_up == 1) || (paths[i][m][0].id == paths[i][j][0].id - 1 - x_count && Left_down == 1) || (paths[i][m][0].id == paths[i][j][0].id + 1 - x_count && Right_down == 1))
						break;
					if ((paths[i][j][0].id % x_count == 0&&(paths[i][m][0].id== paths[i][j][0].id+1|| paths[i][m][0].id == paths[i][j][0].id + 1+x_count|| paths[i][m][0].id == paths[i][j][0].id + 1-x_count) )||( paths[i][j][0].id % x_count == 1&&(paths[i][m][0].id == paths[i][j][0].id - 1|| paths[i][m][0].id == paths[i][j][0].id - 1+x_count|| paths[i][m][0].id == paths[i][j][0].id - 1-x_count)))
						break;
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//���-�յ�
				if ( (abs(paths[i][m][paths[i][m].size() - 1].id - paths[i][j][0].id) == 1 || abs(paths[i][m][paths[i][m].size() - 1].id - paths[i][j][0].id) == x_count || abs(paths[i][m][paths[i][m].size() - 1].id - paths[i][j][0].id) == x_count - 1 || abs(paths[i][m][paths[i][m].size() - 1].id - paths[i][j][0].id) == x_count + 1)) {
					if ((paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 + x_count && Right_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 + x_count && Left_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 - x_count && Left_down == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 - x_count && Right_down == 1))
						break;
					if ((paths[i][j][0].id % x_count == 0 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id + 1 - x_count)) || (paths[i][j][0].id % x_count == 1 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][0].id - 1 - x_count)))
						break;
					paths[i][m].insert(paths[i][m].end(), paths[i][j].begin(), paths[i][j].end());
					paths[i].erase(paths[i].begin() + j);
					m = 0; j = 0;
					break;
				}
			}
		}



		for (int j = 0; j < paths[i].size(); j++) {
			Right_up = 0;//�жϸÿյ��б�����Ƿ������
			//��������Ϊ0����������Ϊ1
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int c = 0; c < paths[i].size(); c++) {
				for (int n = 1; n < paths[i][c].size(); n++) {
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][c][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][c][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {



				if (m == j) m++;
				if (m == paths[i].size()) break;
				////�յ�-�յ�
				if ((abs(paths[i][m][paths[i][m].size() - 1].id - paths[i][j][paths[i][j].size() - 1].id) == 1 || abs(paths[i][m][paths[i][m].size() - 1].id - paths[i][j][paths[i][j].size() - 1].id) == x_count || abs(paths[i][m][paths[i][m].size() - 1].id - paths[i][j][paths[i][j].size() - 1].id) == x_count - 1 || abs(paths[i][m][paths[i][m].size() - 1].id - paths[i][j][paths[i][j].size() - 1].id) == x_count + 1)) {
					if ((paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && Right_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count && Left_up == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && Left_down == 1) || (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count && Right_down == 1))
						break;
					if ((paths[i][j][paths[i][j].size() - 1].id % x_count == 0 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count)) || (paths[i][j][paths[i][j].size() - 1].id % x_count == 1 && (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count || paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count)))
						break;
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					j = 0; m = 0;
					break;
				}
				////�յ�-���
				if ((abs(paths[i][m][0].id - paths[i][j][paths[i][j].size() - 1].id) == 1 || abs(paths[i][m][0].id - paths[i][j][paths[i][j].size() - 1].id) == x_count || abs(paths[i][m][0].id - paths[i][j][paths[i][j].size() - 1].id) == x_count - 1 || abs(paths[i][m][0].id - paths[i][j][paths[i][j].size() - 1].id) == x_count + 1)) {
					if ((paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && Right_up == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count && Left_up == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && Left_down == 1) || (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count && Right_down == 1))
						break;
					if ((paths[i][j][paths[i][j].size() - 1].id % x_count == 0 && (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count)) || (paths[i][j][paths[i][j].size() - 1].id % x_count == 1 && (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count || paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count)))
						break;
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
			}
		}

		printf("%.2lf%%\r", i * 100.0 / paths.size());
	}
	
}

void StressOptimize() {
	//����Ӧ����Ӧ��С��������й���
	double v = 0.3;//���˰ٷֱ�,gong = 0.05;ls = ? ; qiao = 0.1
	double a;
	bool flag;
	element s;
	vector<element>q;//��ÿ����ɢ��Ԫ����q�У����ڼ������ֵ�µ�Ӧ����С
	//for (int i = 0; i < point.size(); i++) {//��ÿ����Ԫ�����ݴ洢��q��
	for (int j = 0; j < point[0].size(); j++) {
		for (int m = 0; m < point[0][j].size(); m++) {
			if (point[0][j][m].b == 1)
				q.push_back(point[0][j][m]);
		}
	}
	//}
	printf("�Ի����������\n");
	for (int j = 0; j < q.size() - 1; j++) {//��q���д�С��������
		flag = false;
		for (int m = q.size() - 1; m > j; m--) {
			if (abs(q[m - 1].s_max) > abs(q[m].s_max)) {
				s = q[m - 1];
				q[m - 1] = q[m];
				q[m] = s;
				flag = true;
			}
		}
		if (flag == false) break;
		printf("%.2lf%%\r", j * 100.0 / (q.size() - 1));
	}
	a = abs(q[0].s_max) + (abs(q[q.size() - 1].s_max) - abs(q[0].s_max)) * v;
	printf("�Ż�Ӧ��С����\n");
	//����һ
	//����һ��������ʹ�ù�����ȱ����㷨��
	//for (int i = 0; i < model.size(); i++) {
	//	for (int j = 0; j < model[i].size(); j++) {
	//		if (abs(model[i][j].s_max) <= a) {
	//			model[i][j].direction = -1;
	//			que.push(model[i][j]);//�������
	//		}
	//	}
	//	while (!que.empty()) {
	//		points front = que.front();//��һ������
	//		que.pop();//ɾ����һ��Ԫ��
	//		double count = 0, sum = 0;
	//		//������ǰ�յ�Ԫ�İ˸��ڽӵ�Ԫ
	//		for (int m = 0; m < model[i].size(); m++) {
	//			if (model[i][m].direction != -1) {
	//				if (model[i][m].id == front.id + x_count || model[i][m].id == front.id - x_count ||
	//					(model[i][m].id == front.id + x_count + 1 && front.id % x_count != 0) || (model[i][m].id == front.id + 1 && front.id % x_count != 0) ||
	//					(model[i][m].id == front.id - x_count + 1 && front.id % x_count != 0) || (model[i][m].id == front.id - x_count - 1 && front.id % x_count != 1) ||
	//					(model[i][m].id == front.id - 1 && front.id % x_count != 1) || (model[i][m].id == front.id - 1 + x_count && front.id % x_count != 1)) {
	//					sum += model[i][m].direction;
	//					count++;
	//				}
	//			}
	//		}
	//		if (count == 0) {
	//			que.push(front);
	//		}
	//		else {
	//			for (int m = 0; m < model[i].size(); m++) {
	//				if (model[i][m].id == front.id)
	//					model[i][m].direction = sum / count;
	//			}
	//		}
	//	}
	//	printf("%.2lf%%\r", i * 100.0 / model.size());
	//}
	//������
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (abs(model[i][j].s_max) <= a) {
				model[i][j].direction = -1;
			}
		}
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].direction != -1) {
				for (int m = 0; m < model[i].size(); m++) {
					if (model[i][m].direction == -1) {
						if (model[i][m].id == model[i][j].id + 1 || model[i][m].id == model[i][j].id - 1) {
							model[i][m].direction = model[i][j].direction;
					
						}
						else if ((model[i][m].id == model[i][j].id + 1 - x_count || model[i][m].id == model[i][j].id + 1 || model[i][m].id == model[i][j].id + 1 + x_count) && model[i][j].id % x_count != 0) {
							model[i][m].direction = model[i][j].direction;
							
						}
						else if ((model[i][m].id == model[i][j].id - 1 - x_count || model[i][m].id == model[i][j].id - 1 || model[i][m].id == model[i][j].id - 1 + x_count) && model[i][j].id % x_count != 1) {
							model[i][m].direction = model[i][j].direction;
							
						}

					}
				}
			}
			if (j == model[i].size()-1) {
				for (int n = 0; n < model[i].size(); n++) {
					if (model[i][n].direction == -1) {
						j = 0;
						break;
					}
				}
			}
			
		}
		printf("%.2lf%%\r", i * 100.0 / model.size());
	}


}

void StressPath() {
	points s;//����
	path n;
	bool flag;
	
	//������boolΪ1�ĵ�Ԫ����model��
	for (int i = 0; i < point.size(); i++) {
		for (int j = 0; j < point[0].size(); j++) {
			for (int m = 0; m < point[0][0].size(); m++) {
				if (point[i][j][m].b == 1) {
					s.id = point[i][j][m].id;
					s.x = point[i][j][m].x;
					s.y = point[i][j][m].y;
					s.z = point[i][j][m].z;
					s.s_max = point[i][j][m].s_max;
					s.direction = point[i][j][m].direction;
					model_buffer.push_back(s);
				}

			}
		}
		model.push_back(model_buffer);
		model_buffer.clear();
	}
	//����Ӧ����abs���Ĵ�С�����������
	//����ð������;
	printf("����Ӧ����С��������\n");
	for (int i = 0; i < model.size(); i++) {
		
		for (int j = 0; j < model[i].size() - 1; j++) {
			flag = false;
			for (int m = model[i].size() - 1; m > j; m--) {
				if (abs(model[i][m - 1].s_max) < abs(model[i][m].s_max)) {
					s = model[i][m - 1];
					model[i][m - 1] = model[i][m];
					model[i][m] = s;
					flag = true;
				}
			}
			if (flag == false) break;
		}
		printf("%.2lf%%\r", i * 100.0 / model.size());
	}

	//StressOptimize();//��Ӧ��С����������Ż�


	//����Ӧ��·��
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				//model[i][j].b++;//���Ӹõ�Ԫ�Ķ���
				s = model[i][j];
				n.id = s.id;
				n.x = s.x;
				n.y = s.y;
				n.z = s.z;
				n.direction = s.direction;
				paths_buffer1.push_back(n);
				VectorInsert(i, j);
				paths_buffer2.push_back(paths_buffer1);
				paths_buffer1.clear();
			}
			
		}
		printf("%.2lf%%\r", i * 100.0 / model.size());
		paths.push_back(paths_buffer2);
		paths_buffer2.clear();
	}
	//ɾ��ֻ��һ����Ԫ��·��
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			if (paths[i][j].size() == 1) {
				paths[i].erase(paths[i].begin() + j);
				j--;//ÿɾ��һ��Ԫ�أ������ĵ�ַ����ǰ��һ����λ������j��Ҫ-1
			}
		}
	}
	//�Ż�·���յ�
	printf("�Ż��յ�......\n");
	OptimizePoints();
	//����·���Ż�
	printf("����·���Ż�......\n");

	PathConnect();
	//�Ż�G-code·��
	printf("����G-code·��......\n");
	/*for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			if (paths[i][j].size() > 2) {
				for (int m = 2; m < paths[i][j].size(); m++) {
					if (((paths[i][j][m].x==paths[i][j][m-1].x&&paths[i][j][m-1].x==paths[i][j][m-2].x)||(paths[i][j][m].y == paths[i][j][m - 1].y && paths[i][j][m - 1].y == paths[i][j][m - 2].y))
					) {
						paths[i][j].erase(paths[i][j].begin() + m - 1);
						m = m - 1;
					}
				}
			}

		}
	}*/

	

}

void GcodePrint() {
	FILE* fp;
	errno_t err;     //�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&fp, "ls2.gcode", "a"); //��return 1 , ��ָ������ļ����ļ�����
	double t1 = 0.03326;//���0.2��˿��0.4,�����ֲ���
	double t2 = 0.04756;//���0.2��˿��0.56
	double t0 = t1 + (t2 - t1) / 2;
	double E = 0;
	double r;//�س�
	int L=-50;//ƫ����
	fprintf(fp, ";FLAVOR:Marlin\n");
	fprintf(fp, ";Generated with Cura_SteamEngine 4.10.0\n");
	fprintf(fp, "M140 S50\n");
	fprintf(fp, "M105\n");
	fprintf(fp, "M190 S50\n");
	fprintf(fp, "M104 S210\n");
	fprintf(fp, "M105\n");
	fprintf(fp, "M109 S210\n");
	fprintf(fp, "M82 ;absolute extrusion mode\n");
	fprintf(fp, "M201 X500.00 Y500.00 Z100.00 E5000.00 ;Setup machine max acceleration\n");
	fprintf(fp, "M203 X500.00 Y500.00 Z10.00 E50.00 ;Setup machine max feedrate\n");
	fprintf(fp, "M204 P500.00 R1000.00 T500.00 ;Setup Print/Retract/Travel acceleration\n");
	fprintf(fp, "M205 X8.00 Y8.00 Z0.40 E5.00 ;Setup Jerk\n");
	fprintf(fp, "M220 S100 ;Reset Feedrate\n");
	fprintf(fp, "M221 S100 ;Reset Flowrate\n");

	fprintf(fp, "G28 ;Home\n");

	fprintf(fp, "G92 E0\n");
	fprintf(fp, "G92 E0\n");
	fprintf(fp, "G1 F2700 E-5\n");
	fprintf(fp, "M107\n");
	

	
	
	//ģ�͵�һ��
	
	//fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", paths[0][0][0].x+L, paths[0][0][0].y + L, paths[0][0][0].z + 0.1);//ģ�͵�һ����������0.22
	//fprintf(fp, "G1 F4800 E0.00000\n");
	//
	//fprintf(fp, ";TYPE:WALL-OUTPE\n");


	//for (int j = 0; j < path_w.size(); j++) {
	//	r = E - 2;
	//	fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
	//	fprintf(fp, "G0 X%f Y%f\n", path_w[j][0].x + L, path_w[j][0].y + L);
	//	fprintf(fp, "G1 F4800 E%f\n", E);
	//	for (int m = 1; m < path_w[j].size(); m++) {
	//		fprintf(fp, "G1 F1200 X%f Y%f E%f\n", path_w[j][m].x + L, path_w[j][m].y + L, E += distance(path_w[j][m - 1], path_w[j][m]) * t2);
	//	}
	//}


	//fprintf(fp, ";TYPE:FILL\n");
	//for (int j = 0; j < paths[0].size(); j++) {
	//	r = E - 2;
	//	fprintf(fp, "G1 F4800 E%f\n",r);//�س�һ�����룬������˿
	//	fprintf(fp, "G0 X%f Y%f\n", paths[0][j][0].x + L, paths[0][j][0].y + L);
	//	fprintf(fp, "G1 F4800 E%f\n", E);
	//	for (int m = 1; m < paths[0][j].size(); m++) {
	//		if (paths[0][j][m - 1].x != paths[0][j][m].x && paths[0][j][m - 1].y != paths[0][j][m].y) {
	//			fprintf(fp, "G1 F1200 X%f Y%f E%f\n", paths[0][j][m].x + L, paths[0][j][m].y + L, E += distance(paths[0][j][m - 1], paths[0][j][m]) * t1);
	//		}
	//		else {
	//			fprintf(fp, "G1 F1200 X%f Y%f E%f\n", paths[0][j][m].x + L, paths[0][j][m].y + L, E += distance(paths[0][j][m - 1], paths[0][j][m]) * t2);
	//		}
	//	}
	//}

	for (int i = 5; i < paths.size(); i++) {//�ӵڶ��㿪ʼ
		//fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", path_w[i][0][0].x + L, path_w[i][0][0].y + L, paths[i][0][0].z + 0.1);
		//fprintf(fp, ";TYPE:WALL-OUTPE\n");
		//for (int j = 0; j < path_w[i].size(); j++) {

		//	r = E - 2;
		//	fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
		//	fprintf(fp, "G0 X%f Y%f\n", path_w[i][j][0].x + L, path_w[i][j][0].y + L);
		//	fprintf(fp, "G1 F4800 E%f\n", E);
		//	for (int m = 1; m < path_w[i][j].size(); m++) {
		//		fprintf(fp, "G1 F1200 X%f Y%f E%f\n", path_w[i][j][m].x + L, path_w[i][j][m].y + L, E += distance(path_w[i][j][m - 1], path_w[i][j][m]) * t2);
		//	}
		//}

		fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", paths[i][0][0].x + L, paths[i][0][0].y + L, paths[i][0][0].z + 0.1);
		fprintf(fp, ";TYPE:FILL\n");
		for (int j = 0; j < paths[i].size(); j++) {

			r = E - 2;
			fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
			fprintf(fp, "G0 X%f Y%f\n", paths[i][j][0].x + L, paths[i][j][0].y + L);
			fprintf(fp, "G1 F4800 E%f\n", E);

			for (int m = 1; m < paths[i][j].size(); m++) {
				if (paths[i][j][m - 1].x != paths[i][j][m].x && paths[i][j][m - 1].y != paths[i][j][m].y) {
					
					fprintf(fp, "G1 F3000 X%f Y%f E%f\n", paths[i][j][m].x + L, paths[i][j][m].y + L, E += distance(paths[i][j][m - 1], paths[i][j][m]) * t1);
				}
				else {
					
					fprintf(fp, "G1 F3000 X%f Y%f E%f\n", paths[i][j][m].x + L, paths[i][j][m].y + L, E += distance(paths[i][j][m - 1], paths[i][j][m]) * t2);
				}
			}
		}
	}
	fprintf(fp, "M140 S0\n");
	fprintf(fp, "M107\n");
	fprintf(fp, "G91\n");
	fprintf(fp, "G1 E-2 F2700\n");
	fprintf(fp, "G1 E-2 Z0.2 F2400 ;Retract and raise Z\n");
	fprintf(fp, "G1 X5 Y5 F3000 ;Wipe out\n");
	fprintf(fp, "G1 Z10 ;Raise Z more\n");
	fprintf(fp, "G90 ;Absolute positioning\n");

	fprintf(fp, "G1 X0 Y300 ;Present print\n");
	fprintf(fp, "M106 S0 ;Turn-off fan\n");
	fprintf(fp, "M104 S0 ;Turn-off hotend\n");
	fprintf(fp, "M140 S0 ;Turn-off bed\n");

	fprintf(fp, "M84 X Y E ;Disable all steppers but Z\n");

	fprintf(fp, "M82 ;absolute extrusion mode\n");
	fprintf(fp, "M104 S0\n");
	fclose(fp);
}

void AbaqusData() {
	FILE* a; 
	errno_t err;//�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&a, "Abaqus_a.txt", "a"); //��return 1 , ��ָ������ļ����ļ�����
	FILE* b;    //�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&b, "Abaqus_b.txt", "a"); //��return 1 , ��ָ������ļ����ļ�����
	FILE* c;
	err = fopen_s(&c, "Abaqus_c.txt", "a");
	float b1, b2, c1;
	for (int i = 0; i < 2; i++) {

		for (int j = 0; j < point[i].size(); j++) {
			for (int m = 0; m < point[i][j].size(); m++) {
				fprintf(a, "%d\n", point[i][j][m].b);
				b1 = (double)180 * (pi - point[i][j][m].direction) / pi;
				if ((b1 >= 0 && b1 < 22.5) || (b1 >= 157.5 && b1 < 180)) b2 = 0;
				else if (b1 >= 22.5 && b1 < 67.5) b2 = 45;
				else if (b1 >= 67.5 && b1 < 112.5) b2 = 90;
				else if (b1 >= 112.5 && b1 < 157.5) b2 = 135;
				fprintf(b, "%.1f\n", b2);
				if (b1 >= 0 && b1 < 60) c1 = 30;
				else if (b1 >= 60 && b1 < 120) c1 = 90;
				else if (b1 >= 120 && b1 < 180) c1 = 150;
				fprintf(c, "%.1f\n", c1);
			}
		}
	}
	fclose(a);
	fclose(b);
	fclose(c);
}

void MatlabData() {
	int d = 0;
	int num;
	FILE* a1;
	errno_t err;//�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&a1, "Matlab_x.txt", "a"); //��return 1 , ��ָ������ļ����ļ�����
	FILE* b1;    //�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&b1, "Matlab_y.txt", "a"); //��return 1 , ��ָ������ļ����ļ�����
	num = paths[paths.size()-1][0].size();
	for (int i = 0; i < paths[paths.size() - 1].size(); i++) {
		if (paths[paths.size() - 1][i].size() > num)
			num = paths[paths.size() - 1][i].size();
	}

	for (int i = 0; i < paths[paths.size() - 1].size(); i++) {
		for (int j = 0; j < num; j++) {
			if (j < paths[paths.size() - 1][i].size()) {
				fprintf(a1, "%f\t", paths[paths.size() - 1][i][j].x);
				fprintf(b1, "%f\t", paths[paths.size() - 1][i][j].y);
			}
			else {
				fprintf(a1, "%f\t",0);
				fprintf(b1, "%f\t",0);
			}
			

		}
		fprintf(a1, "\n");
		fprintf(b1, "\n");
	}
	fclose(a1);
	fclose(b1);
}

void fangxiangData() {
	int d = 0;
	int num;
	FILE* a1;
	errno_t err;//�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&a1, "fx.txt", "a"); //��return 1 , ��ָ������ļ����ļ�����
	
	for (int j = 0; j < model[0].size(); j++) {
		fprintf(a1, "%f %f %f %f\n", model[0][j].x, model[0][j].y,model[0][j].s_max, model[0][j].direction);
	}
	fclose(a1);

}


void double_path() {
	path p;
	int t = 0;//�����ж������յ�
	paths_buffer1.clear();
	paths_buffer2.clear();
	paths_buffer3.clear();
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			for (int n = 0; n < 2; n++) {
				if (n == 1) {
					reverse(paths[i][j].begin(), paths[i][j].end());
				}
				for (int m = 0; m < paths[i][j].size(); m++) {
					if (m == 0) {//���
						if (paths[i][j][m + 1].id == paths[i][j][m].id - x_count) {//��->��
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id + x_count) {//��->��
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id + 1) {//��->��
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id - 1) {//��->��
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id - x_count + 1) {//����->����
							p.x = paths[i][j][m].x - sqrt(2) * d2 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id + x_count - 1) {//����->����
							p.x = paths[i][j][m].x + sqrt(2) * d2 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id - x_count - 1) {//����->����
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + sqrt(2) * d2 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id + x_count + 1) {//����->����
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - sqrt(2) * d2 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
					}
					else if (m == paths[i][j].size() - 1) {//�յ�
						if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count) {//��-��
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count) {//��-��
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - 1) {//��-��
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + 1) {//��-��
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1) {//����-����
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - sqrt(2) * d2 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1) {//����-����
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + sqrt(2) * d2 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1) {//����-����
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - sqrt(2) * d2 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count - 1) {//����-����
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + sqrt(2) * d2 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						
						}
					}
					else {
						if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count) || 
							(paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) { //case 1
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count - 1)) {//case 2
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 + x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1)) {//case 3
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1 - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1 + x_count)) {//case 4
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count - 1) {//case 5
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1) {//case 6
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count - 1) {//case 7
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1) {//case 8
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) {//case 9
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1)) {//case 10
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1)) {//case 11
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) {//case 12
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count - 1)||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) {//case 13
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y - d1;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count +1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1)) {//case 14
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y + d1;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) {//case 15
						p.x = paths[i][j][m].x - d1 * 0.5;
						p.y = paths[i][j][m].y + d1;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count - 1)) {//case 16
						p.x = paths[i][j][m].x + d1 * 0.5;
						p.y = paths[i][j][m].y - d1;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id  - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1)) {//case 17
						p.x = paths[i][j][m].x + d1;
						p.y = paths[i][j][m].y - d1 * 0.5;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id  + 1) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 + x_count)) {//case 18
						p.x = paths[i][j][m].x - d1;
						p.y = paths[i][j][m].y + d1 * 0.5;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1 + x_count)) {//case 19
						p.x = paths[i][j][m].x - d1;
						p.y = paths[i][j][m].y - d1 * 0.5;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 - x_count) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id - 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1)) {//case 20
						p.x = paths[i][j][m].x + d1;
						p.y = paths[i][j][m].y + d1 * 0.5;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 - x_count) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id - 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count - 1)) {//case 21
						p.x = paths[i][j][m].x;
						p.y = paths[i][j][m].y - d1 * sqrt(2) * 0.5;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1 - x_count) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id + 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1)) {//case 22
						p.x = paths[i][j][m].x;
						p.y = paths[i][j][m].y + d1 * sqrt(2) * 0.5;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 + x_count) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id - 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1)) {//case 23
						p.x = paths[i][j][m].x - d1 * sqrt(2) * 0.5;
						p.y = paths[i][j][m].y;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 - x_count) ||
						(paths[i][j][m - 1].id == paths[i][j][m].id - 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1)) {//case 24
						p.x = paths[i][j][m].x + d1 * sqrt(2) * 0.5;
						p.y = paths[i][j][m].y;
						p.z = paths[i][j][m].z;
						paths_buffer1.push_back(p);
						}
					}
				}
			}
			//paths_buffer1.push_back(paths_buffer1[0]);//�յ����������
			paths_buffer2.push_back(paths_buffer1);
			paths_buffer1.clear();
		}
		paths_buffer3.push_back(paths_buffer2);
		paths_buffer2.clear();
	}
	lines = paths;
	paths.clear();
	paths = paths_buffer3;
	paths_buffer3.clear();
	paths_buffer1.clear();
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			for (int m = 0; m < paths[i][j].size() - 1; m += 2) {
				p.x = (paths[i][j][m].x + paths[i][j][m + 1].x) * 0.5;
				p.y = (paths[i][j][m].y + paths[i][j][m + 1].y) * 0.5;
				p.z = (paths[i][j][m].z + paths[i][j][m + 1].z) * 0.5;
				paths_buffer1.push_back(p);
				paths[i][j].insert(paths[i][j].begin() + m + 1, paths_buffer1.begin(), paths_buffer1.end());
				paths_buffer1.clear();

			}
		}
	}
}
void underfill_optimize() {
	path p;
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				model[i][j].b++;
				for (int n = 0; n < 4; n++) {
					if (n == 0) {
						p.x = model[i][j].x - d1 * 0.5;
						p.y = model[i][j].y + d1 * 0.5;
						p.z = model[i][j].z;
						paths_buffer1.push_back(p);
					}
					else if (n == 1) {
						p.x = model[i][j].x + d1 * 0.5;
						p.y = model[i][j].y + d1 * 0.5;
						p.z = model[i][j].z;
						paths_buffer1.push_back(p);
					}
					else if (n == 2) {
						p.x = model[i][j].x + d1 * 0.5;
						p.y = model[i][j].y - d1 * 0.5;
						p.z = model[i][j].z;
						paths_buffer1.push_back(p);
					}
					else if (n == 3) {
						p.x = model[i][j].x - d1 * 0.5;
						p.y = model[i][j].y - d1 * 0.5;
						p.z = model[i][j].z;
						paths_buffer1.push_back(p);
					}
				}
				
				paths_buffer1.push_back(paths_buffer1[0]);
				paths[i].push_back(paths_buffer1);
				paths_buffer1.clear();
				p.x = model[i][j].x;
				p.y = model[i][j].y;
				p.z = model[i][j].z;
				paths_buffer1.push_back(p);
				lines[i].push_back(paths_buffer1);
				paths_buffer1.clear();
			}
		}
	}
}

void Continuous_path() {

	for (int i = 0; i < paths.size(); i++) {
		for (int j = 1; j < paths[i].size(); j++) {
			for (int n = 0; n < paths[i][j].size() - 1; n++) {
				for (int m = 0; m < paths[i][0].size() - 1; m++) {
					//����
					if (paths[i][j][n].x != paths[i][j][n + 1].x && paths[i][j][n].y == paths[i][j][n + 1].y&&paths[i][0][m].x != paths[i][0][m + 1].x && paths[i][0][m].y == paths[i][0][m + 1].y) {
						//˳��
						if (paths[i][j][n].x == paths[i][0][m].x && paths[i][j][n + 1].x == paths[i][0][m + 1].x && abs(paths[i][j][n].y - paths[i][0][m].y) > d1 - 0.01 && abs(paths[i][j][n].y - paths[i][0][m].y) < d1 + 0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > d1-0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < d1 + 0.01) {
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						////����
						else if (paths[i][j][n].x == paths[i][0][m + 1].x && paths[i][j][n+1].x == paths[i][0][m].x && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > d1 - 0.01 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) < d1 + 0.01 && abs(paths[i][j][n+1].y - paths[i][0][m].y) > d1 - 0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < d1 + 0.01) {
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
					//����
					else if (paths[i][j][n].y != paths[i][j][n + 1].y && paths[i][j][n].x == paths[i][j][n + 1].x&& paths[i][0][m].y != paths[i][0][m + 1].y && paths[i][0][m].x == paths[i][0][m + 1].x) {
						//˳��
						if (paths[i][j][n].y == paths[i][0][m].y && paths[i][j][n + 1].y == paths[i][0][m + 1].y && abs(paths[i][j][n].x - paths[i][0][m].x) > d1 -0.0l && abs(paths[i][j][n].x - paths[i][0][m].x) < d1 + 0.0l && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > d1 - 0.01 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < d1 + 0.01) {
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						//����
						else if (paths[i][j][n].y == paths[i][0][m + 1].y && paths[i][j][n+1].y == paths[i][0][m].y && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > d1-0.001&& abs(paths[i][j][n].x - paths[i][0][m + 1].x) < d1 + 0.001 && abs(paths[i][j][n+1].x - paths[i][0][m].x) > d1-0.001&& abs(paths[i][j][n + 1].x - paths[i][0][m].x) < d1 + 0.001) {
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
					//б
					else if (paths[i][j][n].y != paths[i][j][n + 1].y && paths[i][j][n].x != paths[i][j][n + 1].x && paths[i][0][m].y != paths[i][0][m + 1].y && paths[i][0][m].x != paths[i][0][m + 1].x) {
						//˳��
						if (abs(paths[i][j][n].x - paths[i][0][m].x) < (sqrt(2) * d2 * 0.5) + 0.01 && abs(paths[i][j][n].x - paths[i][0][m].x) > (sqrt(2) * d2 * 0.5) - 0.01 && abs(paths[i][j][n].y - paths[i][0][m].y) < (sqrt(2) * d2 * 0.5) + 0.01 && abs(paths[i][j][n].y - paths[i][0][m].y) > (sqrt(2) * d2 * 0.5) - 0.01 &&
							abs(paths[i][j][n+1].x - paths[i][0][m+1].x) < (sqrt(2) * d2 * 0.5) + 0.01 && abs(paths[i][j][n+1].x - paths[i][0][m+1].x) > (sqrt(2) * d2 * 0.5) - 0.01 && abs(paths[i][j][n+1].y - paths[i][0][m+1].y) < (sqrt(2) * d2 * 0.5) + 0.01 && abs(paths[i][j][n+1].y - paths[i][0][m+1].y) > (sqrt(2) * d2 * 0.5) - 0.01) {
							paths[i][0][m + 1].x = (paths[i][0][m].x + paths[i][0][m + 1].x) * 0.5;
							paths[i][0][m + 1].y = (paths[i][0][m].y + paths[i][0][m + 1].y) * 0.5;
							paths[i][j][n + 1].x = (paths[i][j][n].x + paths[i][j][n + 1].x) * 0.5;
							paths[i][j][n + 1].y = (paths[i][j][n].y + paths[i][j][n + 1].y) * 0.5;
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						//����
						else if (abs(paths[i][j][n].x - paths[i][0][m + 1].x) < (sqrt(2) * d2 * 0.5) + 0.1 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > (sqrt(2) * d2 * 0.5) - 0.1 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) < (sqrt(2) * d2 * 0.5) + 0.1 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > (sqrt(2) * d2 * 0.5) - 0.1&&
							abs(paths[i][j][n+1].x - paths[i][0][m].x) < (sqrt(2) * d2 * 0.5) + 0.1 && abs(paths[i][j][n+1].x - paths[i][0][m].x) > (sqrt(2) * d2 * 0.5) - 0.1 && abs(paths[i][j][n+1].y - paths[i][0][m].y) < (sqrt(2) * d2 * 0.5) + 0.1 && abs(paths[i][j][n+1].y - paths[i][0][m].y) > (sqrt(2) * d2 * 0.5) - 0.1) {
							paths[i][0][m + 1].x = (paths[i][0][m].x + paths[i][0][m + 1].x) * 0.5;
							paths[i][0][m + 1].y = (paths[i][0][m].y + paths[i][0][m + 1].y) * 0.5;
							paths[i][j][n].x = (paths[i][j][n].x + paths[i][j][n + 1].x) * 0.5;
							paths[i][j][n].y = (paths[i][j][n].y + paths[i][j][n + 1].y) * 0.5;
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);//���ұ�
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
				}
				if (j == 0) break;
			}
		}
	}


	//////����������
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 1; j < paths[i].size(); j++) {
			for (int n = 0; n < paths[i][j].size() - 1; n++) {
				for (int m = 0; m < paths[i][0].size() - 1; m++) {
					//����
					if (paths[i][j][n].x != paths[i][j][n + 1].x && paths[i][j][n].y == paths[i][j][n + 1].y && paths[i][0][m].x != paths[i][0][m + 1].x && paths[i][0][m].y == paths[i][0][m + 1].y) {
						//˳��
						if (abs(paths[i][j][n].x - paths[i][0][m].x) < (d1 * 0.5) + 0.4&& abs(paths[i][j][n].x - paths[i][0][m].x) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < (d1*0.5) + 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n].y - paths[i][0][m].y) > d1 - 0.1 && abs(paths[i][j][n].y - paths[i][0][m].y) < d1 + 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > d1 - 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < d1 + 0.1) {
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						////����
						else if (abs(paths[i][j][n].x - paths[i][0][m+1].x) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n].x - paths[i][0][m+1].x) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > d1 - 0.1 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) < d1 + 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) > d1 - 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < d1 + 0.1) {
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
					//����
					else if (paths[i][j][n].y != paths[i][j][n + 1].y && paths[i][j][n].x == paths[i][j][n + 1].x && paths[i][0][m].y != paths[i][0][m + 1].y && paths[i][0][m].x == paths[i][0][m + 1].x) {
						//˳��
						if (abs(paths[i][j][n].y - paths[i][0][m].y) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n].y - paths[i][0][m].y) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n].x - paths[i][0][m].x) > d1 - 0.l && abs(paths[i][j][n].x - paths[i][0][m].x) < d1 + 0.l && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > d1 - 0.1 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < d1 + 0.1) {
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						//����
						else if (abs(paths[i][j][n].y - paths[i][0][m + 1].y) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > d1 - 0.1 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) < d1 + 0.1 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) > d1 - 0.1 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) < d1 + 0.1) {
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
					//б
					else if (paths[i][j][n].y != paths[i][j][n + 1].y && paths[i][j][n].x != paths[i][j][n + 1].x && paths[i][0][m].y != paths[i][0][m + 1].y && paths[i][0][m].x != paths[i][0][m + 1].x) {
						//˳��
						if (abs(paths[i][j][n].x - paths[i][0][m].x) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n].x - paths[i][0][m].x) > (sqrt(2) * d2 * 0.5) - 0.4 && abs(paths[i][j][n].y - paths[i][0][m].y) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n].y - paths[i][0][m].y) > (sqrt(2) * d2 * 0.5) - 0.4 &&
							abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > (sqrt(2) * d2 * 0.5) - 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > (sqrt(2) * d2 * 0.5) - 0.4) {
							paths[i][0][m + 1].x = (paths[i][0][m].x + paths[i][0][m + 1].x) * 0.5;
							paths[i][0][m + 1].y = (paths[i][0][m].y + paths[i][0][m + 1].y) * 0.5;
							paths[i][j][n + 1].x = (paths[i][j][n].x + paths[i][j][n + 1].x) * 0.5;
							paths[i][j][n + 1].y = (paths[i][j][n].y + paths[i][j][n + 1].y) * 0.5;
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						//����
						else if (abs(paths[i][j][n].x - paths[i][0][m + 1].x) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > (sqrt(2) * d2 * 0.5) - 0.4 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > (sqrt(2) * d2 * 0.5) - 0.4 &&
							abs(paths[i][j][n + 1].x - paths[i][0][m].x) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) > (sqrt(2) * d2 * 0.5) - 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) > (sqrt(2) * d2 * 0.5) - 0.4) {
							paths[i][0][m + 1].x = (paths[i][0][m].x + paths[i][0][m + 1].x) * 0.5;
							paths[i][0][m + 1].y = (paths[i][0][m].y + paths[i][0][m + 1].y) * 0.5;
							paths[i][j][n].x = (paths[i][j][n].x + paths[i][j][n + 1].x) * 0.5;
							paths[i][j][n].y = (paths[i][j][n].y + paths[i][j][n + 1].y) * 0.5;
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);//���ұ�
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
				}
				if (j == 0) break;
			}
		}
	}
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			paths[i][j].push_back(paths[i][j][0]);
		}
	}
	//ɾ��ֱ�ߵ��м������
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			if (paths[i][j].size() > 2) {
				for (int m = 2; m < paths[i][j].size(); m++) {
					if (((paths[i][j][m].x==paths[i][j][m-1].x&&paths[i][j][m-1].x==paths[i][j][m-2].x)||(paths[i][j][m].y == paths[i][j][m - 1].y && paths[i][j][m - 1].y == paths[i][j][m - 2].y))
					) {
						paths[i][j].erase(paths[i][j].begin() + m - 1);
						m = m - 1;
					}
				}
			}

		}
	}
}

//void DFS(int i,int j) {
//	int m;
//	visited[i][j] = 1;//��i��ĵ�j��·���ѱ�����
//	arr_buffer1.push_back(j);
//	for (m = 0; m < G[i].size(); m++) {
//		if (G[i][j][m] == 1 && visited[i][m] != 1) {
//			DFS(i, m);
//		}
//	}
//}
//
//void DFSTraverse() {//����������ȱ����㷨��ͨ·��ʵ��һ�ʻ�
//	///////////////////�����ڽӾ���
//	int b;
//	for (int i = 0; i < lines.size(); i++) {
//		for (int j = 0; j < lines[i].size(); j++) {
//			for (int m = 0; m < lines[i].size(); m++) {
//				b = 0;//bΪ�Ƿ������
//				if (j == m) {
//					G_buffer1.push_back(b);
//					continue;
//				}
//				for (int k = 0; k < lines[i][j].size()-1; k++) {
//					for (int n = 0; n < lines[i][m].size(); n++) {
//						if (abs(lines[i][j][k].id - lines[i][j][k + 1].id) == (1 || x_count) && abs(lines[i][j][k].id - lines[i][j][n].id) == (1 || x_count)) {
//							b = 1;
//							break;
//						}else if (abs(lines[i][j][k].id - lines[i][j][k + 1].id) == x_count - 1 && abs(lines[i][j][k].id - lines[i][j][n].id) == x_count - 1) {
//							b = 1;
//							break;
//						}
//					}
//					if (b == 1)
//						break;
//				}
//				//for (int n = 0; n < lines[i][m].size(); n++) {
//				//	if (abs(lines[i][j][0].id - lines[i][j][1].id) == 1 || abs(lines[i][j][0].id - lines[i][j][1].id) == x_count || abs(lines[i][j][lines[i][j].size() - 1].id - lines[i][j][lines[i][j].size() - 2].id) == 1 || abs(lines[i][j][lines[i][j].size() - 1].id - lines[i][j][lines[i][j].size() - 2].id) == x_count) {
//				//		if (lines[i][j][0].id % x_count == 1 || lines[i][j][lines[i][j].size() - 1].id % x_count == 1) {//��߽�
//				//			if (lines[i][j][0].id + 1 == lines[i][m][n].id || lines[i][j][0].id + x_count == lines[i][m][n].id|| lines[i][j][0].id -x_count == lines[i][m][n].id||
//				//				 lines[i][j][lines[i][j].size() - 1].id + 1 == lines[i][m][n].id || lines[i][j][lines[i][j].size() - 1].id + x_count == lines[i][m][n].id || lines[i][j][lines[i][j].size() - 1].id - x_count == lines[i][m][n].id) {
//				//				b = 1;
//				//				break;
//				//			}
//				//		}
//				//		else if (lines[i][j][0].id % x_count == 0 || lines[i][j][lines[i][j].size() - 1].id % x_count == 0) {//�ұ߽�
//				//			if ( lines[i][j][0].id - 1 == lines[i][m][n].id ||  lines[i][j][0].id + x_count == lines[i][m][n].id || lines[i][j][0].id - x_count == lines[i][m][n].id ||
//				//				 lines[i][j][lines[i][j].size() - 1].id - 1 == lines[i][m][n].id ||  lines[i][j][lines[i][j].size() - 1].id + 1 == lines[i][m][n].id || lines[i][j][lines[i][j].size() - 1].id - 1 == lines[i][m][n].id) {
//				//				b = 1;
//				//				break;
//				//			}
//				//		}
//				//		else {
//				//			if (lines[i][j][0].id + 1 == lines[i][m][n].id || lines[i][j][0].id + x_count == lines[i][m][n].id || lines[i][j][0].id - 1 == lines[i][m][n].id ||lines[i][j][0].id - x_count == lines[i][m][n].id ||
//				//				lines[i][j][lines[i][j].size() - 1].id + 1 == lines[i][m][n].id || lines[i][j][lines[i][j].size() - 1].id + x_count == lines[i][m][n].id || lines[i][j][lines[i][j].size() - 1].id - 1 == lines[i][m][n].id || lines[i][j][lines[i][j].size() - 1].id - x_count == lines[i][m][n].id) {
//				//				b = 1;
//				//				break;
//				//			}
//				//		}
//				//	}
//				//}
//				G_buffer1.push_back(b);
//			}
//			G_buffer2.push_back(G_buffer1);
//			G_buffer1.clear();
//		}
//		G.push_back(G_buffer2);
//		G_buffer2.clear();
//	}
//	//��ʼ�����н��Ϊδ���ʣ�����δ����Ϊ0�����ʹ�Ϊ1��
//	G_buffer1.clear();
//	for (int i = 0; i < G.size(); i++) {
//		for (int j = 0; j < G[i].size(); j++) {
//			G_buffer1.push_back(0);
//		}
//		visited.push_back(G_buffer1);
//		G_buffer1.clear();
//	}
//	///������ȱ����㷨
//	for (int i = 0; i < G.size(); i++) {
//		for (int j = 0; j < G[i].size(); j++) {
//			if (visited[i][j] != 1) {
//				DFS(i, j);
//			}
//		}
//		arr.push_back(arr_buffer1);
//		arr_buffer1.clear();
//	}
//	for (int i = 0; i < arr[0].size(); i++) {
//		printf("%d,", arr[0][i]);
//	}
//}
//
//
//void Continuous_path() {
//	DFSTraverse();//���ɿ�����·������
//	�ϲ�·��
//	paths_buffer1.clear();
//	paths_buffer2.clear();
//	paths_buffer3.clear();//��ʼ������·��
//	for (int i = 0; i < arr.size(); i++) {//���ݿ����������������·��˳��
//		for (int j = 0; j < arr[i].size(); j++) {
//			paths_buffer2.push_back(paths[i][arr[i][j]]);
//		}
//		paths_buffer3.push_back(paths_buffer2);
//		paths_buffer2.clear();
//	}
//	paths.clear();
//	paths = paths_buffer3;
//	paths_buffer3.clear();
//	
//	for (int i = 0; i < paths.size(); i++) {
//		for (int j = 0; j < paths[i].size(); j++) {
//			for (int c = 0; c < paths[i].size(); c++) {
//				for (int m = 0; m < paths[i][j].size() - 1; m++) {
//					for (int n = 0; n < paths[i][c].size() - 1; n++) {
//						if (paths[i][j][m].x != paths[i][j][m + 1].x && paths[i][j][m].y == paths[i][j][m + 1].y) {//ˮƽ
//							if (paths[i][c][n].x == paths[i][j][m].x && paths[i][c][n].y == paths[i][j][m].y + d1 &&
//								paths[i][c][n + 1].x == paths[i][j][m + 1].x && paths[i][c][n + 1].y == paths[i][j][m + 1].x + d1) {
//								reverse(paths[i][c].begin(), paths[i][c].begin() + n - 1);
//								reverse(paths[i][c].begin() + n, paths[i][c].end());
//								paths[i][j].insert(paths[i][j].begin() + m - 1, paths[i][c].begin(), paths[i][c].end());
//								paths[i].erase(paths[i].begin() + j);
//								j--;
//								c = 0;
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//}

void main(int argc, char** argv) {
	clock_t start, end;
	start = clock();//��ʼʱ��
	printf("���ڶ�ȡ�ļ�........\n");
	ReadFile();
	printf("��ȡ�ɹ���\n");
	printf("�������������Ӧ��.......\n");
	FE_Analysis();
	printf("����·����........\n");
	StressPath();
	//clipper();
	double_path();
	underfill_optimize();
	Continuous_path();
	printf("·�����ɳɹ���\n");
	GcodePrint();
	//AbaqusData();
	//MatlabData();
	//fangxiangData();
	end = clock();//����ʱ��

	int s1, s2, s3, s4, s5;
	s1 = 0; s2 = 0; s3 = 0; s4 = 0; s5 = 0;
	for (int i = 0; i < point.size(); i++) {
		for (int j = 0; j < point[0].size(); j++) {
			for (int m = 0; m < point[0][0].size(); m++) {
				if (point[i][j][m].direction > 0 && point[i][j][m].direction < pi / 8) s1++;
				else if (point[i][j][m].direction >= (double)pi / (double)8 && point[i][j][m].direction < (double)pi * (double)((double)3 / (double)8)) s2++;
				else if (point[i][j][m].direction >= (double)pi * (double)((double)3 / (double)8) && point[i][j][m].direction < (double)pi * (double)((double)5 / (double)8)) s3++;
				else if (point[i][j][m].direction >= (double)pi * (double)((double)5 / (double)8) && point[i][j][m].direction < (double)pi * (double)((double)7 / (double)8)) s4++;
				else if (point[i][j][m].direction >= (double)pi * (double)((double)7 / (double)8) && point[i][j][m].direction < (double)pi) s5++;
				//printf("%d %f %f %f %d %f %f %f %f %f\n", point[i][j][m].id, point[i][j][m].x, point[i][j][m].y, point[i][j][m].z, point[i][j][m].b, point[i][j][m].s11, point[i][j][m].s22, point[i][j][m].s12, point[i][j][m].s_max, point[i][j][m].direction);
			}
		}
	}
	printf("%d %d %d %d %d\n", s1, s2, s3, s4,s5);
	int s=0;
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 1) {
				s++;
			}
		}
	}
	cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;  //���ʱ�䣨��λ����
	printf("��1���ܹ���%d��·��,��Ԫ��������Ϊ%d\n", paths[0].size(),s);

}
