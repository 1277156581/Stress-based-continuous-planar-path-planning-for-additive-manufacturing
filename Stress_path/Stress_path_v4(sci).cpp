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
#include<queue>
//#include"GA.h"
#define use_int32
#define pi 3.1415926
using namespace std;

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
	int id = -1;
		double x, y, z, direction;
};



vector<element>point_buffer1;//��һ�㻺��
vector<vector<element>>point_buffer2;//�ڶ��㻺��
vector<vector<vector<element>>>point;//ÿ����Ԫ����Ϣ
vector<vector<points>>model;//��Ӧ����С������Ч��Ԫ�������򣬴�->С,һά�������
vector<points>model_buffer;
vector<vector<vector<path>>>paths;//�洢·����һά�������
vector<path>paths_buffer1;
vector<vector<path>>paths_buffer2;
vector<vector<vector<path>>>path_w;//������
//int num=0;//����

queue<points> que;
int x_count, y_count, z_count;//�ֱ��ʾx��y,z�ĵ�Ԫ����
double Bmin_x, Bmin_y, Bmax_x, Bmax_y;

void ReadFile() {
	element e;
	ifstream file1, file2,file3;
	file1.open("VoxelizationData.txt");//��ȡ�����ļ�
	file2.open("abaqus.txt");//��ȡӦ���ļ�
	file3.open("path_w.txt");
	double a1, a2;
	int q1 = 4149;//����
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
		if ((a1 != 8888 && a2 != 8888)&&(a1!=9999 && a2!=9999)) {
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
		//for (int j = 0; j < paths[i].size(); j++) {
		//	Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
		//	Left_up = 0;
		//	Right_down = 0;
		//	Left_down = 0;
		//	for (int m = 0; m < paths[i].size(); m++) {
		//		for (int n = 1; n < paths[i][m].size(); n++) {
		//			if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
		//				(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
		//				Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
		//			}
		//			if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
		//				(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
		//				Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
		//			}
		//			if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
		//				(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
		//				Right_down = 1;//��������µĶ˵����ӣ���������е�·������
		//			}
		//			if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
		//				(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
		//				Left_down = 1;//��������µĶ˵����ӣ���������е�·������
		//			}
		//		}
		//	}
		//	//��
		//	if (paths[i][j][paths[i][j].size() - 1].direction >= 0 && paths[i][j][paths[i][j].size() - 1].direction < pi / (double)8) {
		//		for (int m = 0; m < paths[i].size(); m++) {
		//			if (m == j) m++;
		//			if (m == paths[i].size()) break;
		//			if ((paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)|| (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
		//				//��m�в��뵽��j�к󷽲�ɾ����m��
		//				paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
		//				paths[i].erase(paths[i].begin() + m);
		//				m = 0; j = 0;
		//				break;
		//			}
		//		}
		//	}
		//	//����
		//	else if (paths[i][j][paths[i][j].size() - 1].direction >= pi / (double)8 && paths[i][j][paths[i][j].size() - 1].direction < pi * ((double)3 / (double)8)) {
		//		for (int m = 0; m < paths[i].size(); m++) {
		//			if (m == j) m++;
		//			if (m == paths[i].size()) break;
		//			if ((paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && paths[i][j][paths[i][j].size() - 1].id % x_count != 0 && Right_up == 0)|| (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && paths[i][j][paths[i][j].size() - 1].id % x_count != 1 && Left_down == 0)) {
		//				//��m�в��뵽��j�к󷽲�ɾ����m��
		//				paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
		//				paths[i].erase(paths[i].begin() + m);
		//				m = 0; j = 0;
		//				break;
		//			}
		//		}
		//	}
		//	//��
		//	else if (paths[i][j][paths[i][j].size() - 1].direction >= pi * ((double)3 / (double)8) && paths[i][j][paths[i][j].size() - 1].direction < pi * ((double)5 / (double)8)) {
		//		for (int m = 0; m < paths[i].size(); m++) {
		//			if (m == j) m++;
		//			if (m == paths[i].size()) break;
		//			if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + x_count|| paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - x_count) {
		//				//��m�в��뵽��j�к󷽲�ɾ����m��
		//				paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
		//				paths[i].erase(paths[i].begin() + m);
		//				m = 0; j = 0;
		//				break;
		//			}
		//		}
		//	}
		//	//����
		//	else if (paths[i][j][paths[i][j].size() - 1].direction >= pi * ((double)5 / (double)8) && paths[i][j][paths[i][j].size() - 1].direction < pi * ((double)7 / (double)8)) {
		//		for (int m = 0; m < paths[i].size(); m++) {
		//			if (m == j) m++;
		//			if (m == paths[i].size()) break;
		//			if ((paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + x_count - 1 && paths[i][j][paths[i][j].size() - 1].id % x_count != 1 && Left_up == 0)|| (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - x_count + 1 && paths[i][j][paths[i][j].size() - 1].id % x_count != 0 && Right_down == 0)) {
		//				//��m�в��뵽��j�к󷽲�ɾ����m��
		//				paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
		//				paths[i].erase(paths[i].begin() + m);
		//				m = 0; j = 0;
		//				break;
		//			}
		//		}
		//	}
		//	//��
		//	else if (paths[i][j][paths[i][j].size() - 1].direction >= pi * ((double)7 / (double)8) && paths[i][j][paths[i][j].size() - 1].direction < pi) {
		//		for (int m = 0; m < paths[i].size(); m++) {
		//			if (m == j) m++;
		//			if (m == paths[i].size()) break;
		//			if ((paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)|| (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
		//				//��m�в��뵽��j�к󷽲�ɾ����m��
		//				paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
		//				paths[i].erase(paths[i].begin() + m);
		//				m = 0; j = 0;
		//				break;
		//			}
		//		}
		//	}
		//}
	
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
				if (abs(paths[i][m][1].id - paths[i][m][0].id) != abs(paths[i][j][0].id - paths[i][j][1].id) && paths[i][m][0].id == 2 * paths[i][j][0].id - paths[i][j][1].id) {
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
				if (abs(paths[i][m][paths[i][m].size() - 2].id - paths[i][m][paths[i][m].size() - 1].id) != abs(paths[i][j][0].id - paths[i][j][1].id) && paths[i][m][paths[i][m].size() - 1].id == 2 * paths[i][j][0].id - paths[i][j][1].id) {
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
				if (abs(paths[i][m][paths[i][m].size() - 2].id - paths[i][m][paths[i][m].size() - 1].id) != abs(paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id) && paths[i][m][paths[i][m].size() - 1].id == 2 * paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id) {
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
				if (abs(paths[i][m][1].id - paths[i][m][0].id) != abs(paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id) && paths[i][m][0].id == 2 * paths[i][j][paths[i][j].size() - 1].id - paths[i][j][paths[i][j].size() - 2].id) {
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
	double v = 0.06;//���˰ٷֱ�
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

	//����һ��������ʹ�ù�����ȱ����㷨��
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (abs(model[i][j].s_max) <= a) {
				model[i][j].direction = -1;
				que.push(model[i][j]);//�������
			}
		}
		while (!que.empty()) {
			points front = que.front();//��һ������
			que.pop();//ɾ����һ��Ԫ��
			double count = 0, sum = 0;
			//������ǰ�յ�Ԫ�İ˸��ڽӵ�Ԫ
			for (int m = 0; m < model[i].size(); m++) {
				if (model[i][m].direction != -1) {
					if (model[i][m].id == front.id + x_count|| model[i][m].id == front.id - x_count||
					(model[i][m].id == front.id + x_count + 1 && front.id % x_count != 0)|| (model[i][m].id == front.id + 1 && front.id % x_count != 0)||
						(model[i][m].id == front.id - x_count + 1 && front.id % x_count != 0)|| (model[i][m].id == front.id - x_count - 1 && front.id % x_count != 1)||
						(model[i][m].id == front.id - 1 && front.id % x_count != 1)|| (model[i][m].id == front.id - 1 + x_count && front.id % x_count != 1)) {
						sum += model[i][m].direction;
						count++;
					}
				}
			}
			if (count == 0) {
				que.push(front);
			}
			else {
				for (int m = 0; m < model[i].size(); m++) {
					if (model[i][m].id == front.id)
						model[i][m].direction = sum / count;
				}
			}
		}
		printf("%.2lf%%\r", i * 100.0 / model.size());
	}
	
	//������
	//for (int i = 0; i < model.size(); i++) {
	//	for (int j = 0; j < model[i].size(); j++) {
	//		if (abs(model[i][j].s_max) <= a) {
	//			model[i][j].direction = -1;
	//		}
	//	}
	//	for (int j = 0; j < model[i].size(); j++) {
	//		if (model[i][j].direction != -1) {
	//			for (int m = 0; m < model[i].size(); m++) {
	//				if (model[i][m].direction == -1) {
	//					if (model[i][m].id == model[i][j].id + 1 || model[i][m].id == model[i][j].id - 1) {
	//						model[i][m].direction = model[i][j].direction;
	//				
	//					}
	//					else if ((model[i][m].id == model[i][j].id + 1 - x_count || model[i][m].id == model[i][j].id + 1 || model[i][m].id == model[i][j].id + 1 + x_count) && model[i][j].id % x_count != 0) {
	//						model[i][m].direction = model[i][j].direction;
	//						
	//					}
	//					else if ((model[i][m].id == model[i][j].id - 1 - x_count || model[i][m].id == model[i][j].id - 1 || model[i][m].id == model[i][j].id - 1 + x_count) && model[i][j].id % x_count != 1) {
	//						model[i][m].direction = model[i][j].direction;
	//						
	//					}

	//				}
	//			}
	//		}
	//		if (j == model[i].size()-1) {
	//			for (int n = 0; n < model[i].size(); n++) {
	//				if (model[i][n].direction == -1) {
	//					j = 0;
	//					break;
	//				}
	//			}
	//		}
	//		
	//	}
	//	printf("%.2lf%%\r", i * 100.0 / model.size());
	//}


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

	StressOptimize();//��Ӧ��С����������Ż�


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
	//OptimizePoints();
	//����·���Ż�
	printf("����·���Ż�......\n");

	//PathConnect();
	//�Ż�G-code·��
	printf("����G-code·��......\n");
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

void DJ() {
	double arcs, s_begin, s_end;
	paths_buffer1.clear();
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size() - 1; j++) {
			 arcs = distance(paths[i][j][paths[i][j].size() - 1], paths[i][j + 1][0]);
			for (int m = j + 1; m < paths[i].size(); m++) {
				s_begin = distance(paths[i][j][paths[i][j].size() - 1], paths[i][m][0]);
				s_end = distance(paths[i][j][paths[i][j].size() - 1], paths[i][m][paths[i][m].size() - 1]);
				if (s_begin < arcs && s_begin < s_end) {
					arcs = s_begin;
					paths_buffer1 = paths[i][m];
					paths[i][m] = paths[i][j + 1];
					paths[i][j + 1] = paths_buffer1;
					paths_buffer1.clear();
				}
				else if (s_end < arcs && s_end < s_begin) {
					arcs = s_end;
					reverse(paths[i][m].begin(), paths[i][m].end());
					paths_buffer1 = paths[i][m];
					paths[i][m] = paths[i][j + 1];
					paths[i][j + 1] = paths_buffer1;
					paths_buffer1.clear();
				}
			}
		}
	}
}


void GcodePrint() {
	FILE* fp;
	errno_t err;     //�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&fp, "ls2.gcode", "a"); //��return 1 , ��ָ������ļ����ļ�����
	double t1 = 0.03326*2;//���0.2��˿��0.4,�����ֲ���
	double t2 = 0.04756*2;//���0.2��˿��0.56
	double t0 = t1 + (t2 - t1) / 2;
	double E = 0;
	double r;//�س�
	int L= 100;//ƫ����
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
	

	//fprintf(fp, ";LAYER:0\n");

	//fprintf(fp, "G0 F2000 Z%.1f\n", 0.1);
	//for (int i = 0; Bmin_x + 2 * i * 0.42 + 0.42 < Bmax_x; i++)  //��1��2���ʵ����һ��߶���0.27
	//{
	//	fprintf(fp, "G0 F2000 X%f Y%f \n", Bmin_x + 2 * i * 0.42, Bmin_y);
	//	fprintf(fp, "G1 F1000 X%f Y%f E%f\n", Bmin_x + 2 * i * 0.42, Bmax_y, E += (Bmax_y - Bmin_y) * t1);
	//	fprintf(fp, "G0 F2000 X%f Y%f \n", Bmin_x + 2 * i * 0.42 + 0.42, Bmax_y);
	//	fprintf(fp, "G1 F1000 X%f Y%f E%f\n", Bmin_x + 2 * i * 0.42 + 0.42, Bmin_y, E += (Bmax_y - Bmin_y) * t1);
	//}
	//fprintf(fp, ";LAYER:1\n");

	//fprintf(fp, "G0 F2000 Z%.1f\n", 0.3);
	//for (int i = 0; Bmin_y + 2 * i * 0.42 + 0.42 < Bmax_y; i++)  //���ǵڶ��㣬�����Ժ���0.2
	//{
	//	fprintf(fp, "G0 F2000 X%f Y%f \n", Bmin_x, Bmin_y + 2 * i * 0.42);
	//	fprintf(fp, "G1 F1000 X%f Y%f E%f\n", Bmax_x, Bmin_y + 2 * i * 0.42, E += (Bmax_y - Bmin_y) * t1);
	//	fprintf(fp, "G0 F2000 X%f Y%f \n", Bmax_x, Bmin_y + 2 * i * 0.42 + 0.42);
	//	fprintf(fp, "G1 F1000 X%f Y%f E%f\n", Bmin_x, Bmin_y + 2 * i * 0.42 + 0.42, E += (Bmax_y - Bmin_y) * t1);
	//}
	
	
	
	//ģ�͵�һ��
	
	fprintf(fp, "G0 F300 X%f Y%f Z%f\n", paths[0][0][0].x+L, paths[0][0][0].y + L, paths[0][0][0].z + 0.1);//ģ�͵�һ����������0.22
	fprintf(fp, "G1 F300 E0.00000\n");
	
	fprintf(fp, ";TYPE:WALL-OUTPE\n");


	//for (int j = 0; j < path_w.size(); j++) {
	//	r = E - 2;
	//	fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
	//	fprintf(fp, "G0 X%f Y%f\n", path_w[j][0].x + L, path_w[j][0].y + L);
	//	fprintf(fp, "G1 F4800 E%f\n", E);
	//	for (int m = 1; m < path_w[j].size(); m++) {
	//		fprintf(fp, "G1 F1200 X%f Y%f E%f\n", path_w[j][m].x + L, path_w[j][m].y + L, E += distance(path_w[j][m - 1], path_w[j][m]) * t2);
	//	}
	//}


	 fprintf(fp, ";TYPE:FILL\n");
	for (int j = 0; j < paths[0].size(); j++) {
		r = E - 0.5;
		fprintf(fp, "G1 F300 E%f\n",r);//�س�һ�����룬������˿
		fprintf(fp, "G0 F4800 X%f Y%f\n", paths[0][j][0].x + L, paths[0][j][0].y + L);
		fprintf(fp, "G1 F300 E%f\n", E);
		for (int m = 1; m < paths[0][j].size(); m++) {
			if (paths[0][j][m - 1].x != paths[0][j][m].x && paths[0][j][m - 1].y != paths[0][j][m].y) {
				fprintf(fp, "G1 F1200 X%f Y%f E%f\n", paths[0][j][m].x + L, paths[0][j][m].y + L, E += distance(paths[0][j][m - 1], paths[0][j][m]) * t1);
			}
			else {
				fprintf(fp, "G1 F1200 X%f Y%f E%f\n", paths[0][j][m].x + L, paths[0][j][m].y + L, E += distance(paths[0][j][m - 1], paths[0][j][m]) * t2);
			}
		}
	}

	for (int i = 1; i < path_w.size(); i++) {//�ӵ�i�㿪ʼ
		fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", path_w[i][0][0].x + L, path_w[i][0][0].y + L, paths[i][0][0].z + 0.1);
		fprintf(fp, ";TYPE:WALL-OUTPE\n");
		for (int j = 0; j < path_w[i].size(); j++) {

			r = E - 0.5;
			fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
			fprintf(fp, "G0 X%f Y%f\n", path_w[i][j][0].x + L, path_w[i][j][0].y + L);
			fprintf(fp, "G1 F4800 E%f\n", E);
			for (int m = 1; m < path_w[i][j].size(); m++) {
				fprintf(fp, "G1 F1200 X%f Y%f E%f\n", path_w[i][j][m].x + L, path_w[i][j][m].y + L, E += distance(path_w[i][j][m - 1], path_w[i][j][m]) * t2);
			}

			
		}


		fprintf(fp, "G0 F300 X%f Y%f Z%f\n", paths[i][0][0].x + L, paths[i][0][0].y + L, paths[i][0][0].z + 0.1);
		fprintf(fp, ";TYPE:FILL\n");
		for (int j = 0; j < paths[i].size(); j++) {

			r = E - 0.5;
			fprintf(fp, "G1 F300 E%f\n", r);//�س�һ�����룬������˿
			fprintf(fp, "G0 F4800 X%f Y%f\n", paths[i][j][0].x + L, paths[i][j][0].y + L);
			fprintf(fp, "G1 F300 E%f\n", E);

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

////////////////////�Ŵ��㷨///////////////////////
void calculation(int i) {
	int k;
	double ss, s;
	s = 0.0;
	ss = 0.0;
	for (int j = 0; j < groups.size(); j++) {
		for (int m = 1; m < groups[j].id.size(); m++) {
			s += dis[groups[j].id[m - 1]][groups[j].id[m]];
		}
		groups[j].fit = 1.0 / s;//������Ӧ�ȡ����ȣ�����ԽС��Ӧ��Խ��
		ss += groups[j].fit;//��ÿ�����ݵ���Ӧ����ӣ��õ�һ����Ӧ���ܺ�
		s = 0.0;
	}
	s = 0.0;
	for (int j = 0; j < groups.size(); j++) {
		groups[j].p = groups[j].fit / ss;//���������Ӧ��ռ����Ӧ�ȵı���
		s += groups[i].p;//�ۼ�
		groups[j].sum_p = s;//�ӵ�һ�����忪ʼ������Ӧ�ȱ��������ۼ�
	}
}

void savebest(int i) {//�������Ž�
	double fit = groupbestfit;
	int b = 0, flag = 0;//��Ǹ��ý��λ��
	for (int j = 0; j < groups.size(); j++) {
		if (groups[j].fit > fit) {
			b = j;
			fit = groups[j].fit;
			flag = 1;//����Ѿ��и��õ�Ⱦɫ��
		}
	}
	if (flag) {
		//groupbest.clear();
		generation = die;//����ǰ�Ĵ�����ֵ����
		groupbest=groups[b].id;
		groupbestp = groups[b].p;
		changebest = 0;
		groupbestfit = groups[b].fit;

	}
	else {
		changebest = 1;//˵����û��ԭ���ĺã�Ҫ�����滻
	}
}

void init_group(int i) {//��ʼ����Ⱥ
	vector<double>dis1;
	group G;
	vector<int>g;
	double dis3;
	int flag, random, flag1,n;
	groupbestp = 0.0;
	groupbestfit = 0.0;
	changebest = 0;
	generation = 0;
	groupbest.clear();
	groups.clear();
	dis.clear();
	for (int j = 0; j < paths[i].size(); j++) {
		for (int m = 0; m < paths[i].size(); m++) {
			dis3 = distance(paths[i][j][paths[i][j].size() - 1], paths[i][m][0]);
			dis1.push_back(dis3);
		}
		dis.push_back(dis1);
		dis1.clear();
	}


	///������Ⱥ
	srand((unsigned)time(NULL));//�������
	for (int j = 0; j < M; j++) {
		random = rand() % paths[i].size();
		g.push_back(random);
		for (int m = 1; m < paths[i].size(); m++) {
			flag = 1;
			while (flag) {
				random = rand() % paths[i].size();
				for (n = 0; n < g.size(); n++) {
					if (g[n] == random) {
						break;
					}
				}
				if (n == g.size()) {
					g.push_back(random);
					flag = 0;
				}
			}
		}
		G.id = g;
		G.fit = 0.0; G.p = 0.0; G.sum_p = 0.0;
		groups.push_back(G);
		g.clear();
		
	}
	//���Ϸֱ������M����Ⱥ���ֱ��в��ظ���Ⱦɫ��
	calculation(i);
	savebest(i);
}

void change_bestgroup(int i) {//�����Ž��������Ⱥ�յ�����Ⱦɫ��
	int b;
	double fit = groups[0].fit;
	b = 0;//���Ⱦɫ���λ��
	if (changebest) {
		for (int j = 1; j < groups.size(); j++) {
			if (groups[j].fit < fit) {
				fit = groups[j].fit;
				b = j;
			}
		}
		groups[b].id = groupbest;
		calculation(i);
	}
}

void select(int i) {//ѡ��-����
	double t;
	vector<vector<int>>temp;
	vector<int>temp1;
	srand((unsigned)time(NULL));
	for (int j = 0; j < groups.size(); j++) {
		t = rand() % 10000 * 1.0 / 10000;
		for (int m = 0; m < groups.size(); m++) {
			if (t < groups[m].sum_p) {
				for (int n = 0; n < groups[m].id.size(); n++) {
					temp1.push_back(groups[m].id[n]);
				}
				
				break;
			}
		}
		temp.push_back(temp1);
		temp1.clear();
	}
	for (int j = 0; j < temp.size(); j++) {
		for (int m = 0; m < temp[j].size(); m++) {
			groups[j].id[m] = temp[j][m];
		}
	}
	calculation(i);
	savebest(i);
	change_bestgroup(i);
}

void crossover(int i) {
	/*vector<int>temp1,temp2;
	int n,m1,m2,num;
	for (int j = 1; j < groups.size(); j = j + 2) {
		n = 0;
		temp1.push_back(groups[j - 1].id[0]);
		m1 = 1; m2 = 0;
		while (1) {
			if (m1 == groups[j - 1].id.size()) {
				n = 0;
			}
			if (n == 1&& m1< groups[j - 1].id.size()) {
				m1++;
				for (num = 0; num < temp1.size();num++) {
					if (temp1[num] == groups[j - 1].id[m1-1]) {
						temp2.push_back(groups[j - 1].id[m1-1]);
						break;
					}
				}
				if (num == temp1.size()) {
					temp1.push_back(groups[j - 1].id[m1-1]);
				}
				n = 0;
			}
			if (n == 0 && m2 < groups[j].id.size()) {
				m2++;
				for (num = 0; num < temp1.size(); num++) {
					if (temp1[num] == groups[j].id[m2-1]) {
						temp2.push_back(groups[j].id[m2-1]);
						break;
					}	
				}
				if (num == temp1.size()) {
					temp1.push_back(groups[j].id[m2-1]);
				}
				n = 1;
			}
			if (m1 == groups[j - 1].id.size() && m2 == groups[j].id.size())
				break;
		}
		groups[j - 1].id = temp1;
		groups[j].id = temp2;
		temp1.clear();
		temp2.clear();
		calculation(i);
		savebest(i);
		change_bestgroup(i);

	}*/



	int point1, point2;//�����1�������2
	int temp, k, num, write;
	int temp2[2][10000], temp3[2][10000];
	
	srand((unsigned)time(NULL));
	point1 = rand() % paths[i].size();
	point2 = rand() % paths[i].size();

	if (point1 > point2) {
		temp = point1;
		point1 = point2;
		point2 = temp;
	}
	if (point1 != point2) {
		for (int j = 1; j < groups.size(); j = j + 2) {
			memset(temp3, -1, sizeof(temp3));
			memset(temp2, -1, sizeof(temp2));

			k = 0;
			for (int m = point1; m <= point2; m++) {
				temp2[0][k] = groups[j].id[m];//temp2[0]��ŵ�1��Ⱦɫ��Ľ���Ƭ��B
				temp2[1][k] = groups[j - 1].id[m];//temp2[0]��ŵ�1��Ⱦɫ��Ľ���Ƭ��B
				//��������Ѿ�����
				temp3[0][temp2[0][k]] = 1;
				temp3[1][temp2[1][k]] = 1;
				k++;

				groups[j].id[m] = -1;
				groups[j - 1].id[m] = -1;
			}
			num = point2 - point1 + 1;//�����˶��ٻ���

			//����
			//��0��Ⱦɫ��
			for (k = 0; k < point1; k++) {
				if (temp3[0][groups[j - 1].id[k]] == 1)
					groups[j - 1].id[k] = -1;
				else
					temp3[0][groups[j - 1].id[k]] = 1;
			}
			for (k = point2 + 1; k < groups[j - 1].id.size(); k++) {
				if (temp3[0][groups[j - 1].id[k]] == 1)
					groups[j - 1].id[k] = -1;
				else
					temp3[0][groups[j - 1].id[k]] = 1;
			}
			//��1��Ⱦɫ��
			for (k = 0; k < point1; k++) {
				if (temp3[1][groups[j].id[k]] == 1)
					groups[j].id[k] = -1;
				else
					temp3[1][groups[j].id[k]] = 1;
			}
			for (k = point2 + 1; k < groups[j - 1].id.size(); k++) {
				if (temp3[1][groups[j].id[k]] == 1)
					groups[j].id[k] = -1;
				else
					temp3[1][groups[j].id[k]] = 1;
			}
			//��0��Ⱦɫ��
			write = 0;
			for (int m = 0; m < groups[j - 1].id.size(); m++) {
				while (write < groups[j - 1].id.size() && groups[j - 1].id[write] == -1) {
					write++;//����-1��λ�þ�+1
				}
				if (write < groups[j - 1].id.size()) {
					temp = groups[j - 1].id[m];
					groups[j - 1].id[m] = groups[j - 1].id[write];
					groups[j - 1].id[write] = temp;
					write++;
				}
				else {
					write = 0;
					for (k = m; k < groups[j - 1].id.size(); k++) {
						groups[j - 1].id[k] = temp2[0][write++];
						if (write == num)
							break;
					}
					break;
				}
			}
			//��1��Ⱦɫ��
			write = 0;
			for (int m = 0; m < groups[j].id.size(); m++) {
				while (write < groups[j].id.size() && groups[j].id[write] == -1) {
					write++;//����-1��λ�þ�+1
				}
				if (write < groups[j].id.size()) {
					temp = groups[j].id[m];
					groups[j].id[m] = groups[j].id[write];
					groups[j].id[write] = temp;
					write++;
				}
				else {
					write = 0;
					for (k = m; k < groups[j].id.size(); k++) {
						groups[j].id[k] = temp2[1][write++];
						if (write == num)
							break;
					}
					break;
				}
			}
			//��0��Ⱦɫ�岹ȫ
			k = 0;
			for (int m = 0; m < groups[j - 1].id.size(); m++) {
				if (groups[j - 1].id[m] == -1) {
					while (temp3[0][k] == 1 && k < groups[j - 1].id.size()) {
						k++;
					}
					groups[j - 1].id[m] = k++;
				}
			}
			//��1��Ⱦɫ�岹ȫ
			k = 0;
			for (int m = 0; m < groups[j].id.size(); m++) {
				if (groups[j].id[m] == -1) {
					while (temp3[1][k] == 1 && k < groups[j].id.size()) {
						k++;
					}
					groups[j].id[m] = k++;
				}
			}



		}
		calculation(i);
		savebest(i);
		change_bestgroup(i);
	}

}

void mutation(int i) {
	int t1, t2, temp, t, s = 3;
	srand((unsigned)time(NULL));
	//t = rand() % groups.size();
	//if (t > (groups.size() - (int)groups.size() * 0.7)) {
	//	t1 = rand() % groups.size();//��Ⱥ����һ��Ⱦɫ��
	//	t2 = rand() % paths[i].size();//��һ��Ⱦɫ���λ��
	//	temp = groups[t1].id[t2];
	//	groups[t1].id[t2] = groups[t1].id[groups[t1].id.size() - 1 - t2];
	//	groups[t1].id[groups[t1].id.size() - 1 - t2] = temp;
	//}
	for (t1 = 0; t1 < groups.size(); t1++) {
		t = rand() % groups.size();
		if (t > (groups.size() - (int)groups.size() * 0.9)) {
			t2 = rand() % paths[i].size();//��һ��Ⱦɫ���λ��
			temp = groups[t1].id[t2];
			groups[t1].id[t2] = groups[t1].id[groups[t1].id.size() - 1 - t2];
			groups[t1].id[groups[t1].id.size() - 1 - t2] = temp;
		}
	}
	calculation(i);
	savebest(i);
	change_bestgroup(i);
}

void GA(int i) {
	FILE* a1;
	errno_t err;//�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&a1, "GAdata.txt", "a"); //��return 1 , ��ָ������ļ����ļ�����
	distancenum = 0.0;
	init_group(i);
	die = Die;
	for (int j = 0; j < die; j++) {
		select(i);//ѡ��
		crossover(i);//����
		mutation(i);//����

		distancenum = 0.0;

		for (int j = 1; j < groupbest.size(); j++) {
			distancenum += dis[groupbest[j - 1]][groupbest[j]];
		}
		fprintf(a1,"%f\n", 1.0/groupbestfit);
	}
}

void GeneticAlgorithm() {

	printf("�Ŵ��㷨........\n");
	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			paths[i][j][0].id = j;
		}
		GA(i);
		for (int j = 0; j < paths[i].size()-1; j++) {
			for(int m = j+1; m < paths[i].size(); m++) {
				if (groupbest[j] == paths[i][m][0].id) {
					paths_buffer1 = paths[i][m];
					paths[i][m] = paths[i][j];
					paths[i][j] = paths_buffer1;
				}
			}
		}
		printf("%.2lf%%\r", i * 100.0 / paths.size());
	}
}
//////////////////////////////////////////////////
void main(int argc, char** argv) {
	printf("���ڶ�ȡ�ļ�........\n");
	ReadFile();
	printf("��ȡ�ɹ���\n");
	printf("�������������Ӧ��.......\n");
	FE_Analysis();
	printf("����·����........\n");
	
	StressPath();
	//��̿��г��Ż�
	
	printf("·�����ɳɹ���\n");
	//GeneticAlgorithm();
	DJ();
	GcodePrint();
	AbaqusData();
	//MatlabData();
	//fangxiangData();


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
	/*for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				s++;
			}
		}
	}*/

	for (int i = 0; i < point.size(); i++) {
		for (int j = 0; j < point[i].size(); j++) {
			for(int m = 0; m < point[i][j].size(); m++) {
				if (point[i][j][m].b == 1) {
					s++;
				}
			}
		}
	}
	printf("�ܹ���%d��·��,��Ԫ����Ϊ%d\n", paths[0].size(),s);
	/*for (int i = 0; i < model[0].size(); i++) {
		printf("%d %f\n",model[0][i].id, model[0][i].s_max);
	}*/
	//for (int i = 0; i < paths.size(); i++) {
		/*for (int j = 0; j < paths[0].size(); j++) {
			for (int m = 0; m < paths[0][j].size(); m++) {
				printf("%d\n",paths[0][j][m].id);
			}
			printf("*************\n");
		}
	printf("%d\n", paths[0].size());*/
	//}
	//�Ż��յ����
	/*int n=0;
	for (int j = 0; j < model[0].size(); j++) {
		if (model[0][j].b == 0) {
			n++;
		}
	}
	printf("%d\n", n);*/
	double dis12=0;
	for (int i = 1; i < paths[0].size(); i++) {
		dis12+=distance(paths[0][i - 1][paths[0][i-1].size() - 1], paths[0][i][0]);
	}
	printf("%d %f\n", paths[0].size()-1,dis12);
}
