#ifndef _ATT_TRAN_H
#define _ATT_TRAN_H
#include <math.h>
#include "GINS_math.h"

void m2qua_ned(double m[], double q[]);//�������Ҿ���ת��Ԫ��ned
void q2mat_ned(double qua[], double m[]);//��Ԫ��ת�������Ҿ���ned
void m2att_ned(double m[], double a[]);//�������Ҿ���תŷ����ned
void a2mat_ned(double att[], double m[]);//ŷ����ת�������Ҿ���ned
void a2mat(double att[], double m[]);//ŷ����2�������Ҿ���enu
void a2qua(double att[], double qua[]);//ŷ����2��Ԫ��enu
void m2att(double mat[], double att[]);//�������Ҿ���2ŷ����enu
void m2qua(double mat[], double qua[]);//�������Ҿ���2��Ԫ��enu
void q2att(double qua[], double att[]);//��Ԫ��2ŷ����
void q2mat(double qua[], double mat[]);//��Ԫ��2�������Ҿ���
void askew(double v[], double m[]);
void qdelphi(double qpb[], double phi[]);
void rv2q(double rv[], double q[]);
void qmuln(double q1[], double q2[], double q[]);
//��̬���£���������
void qupdt2(double qnb0[], double rv_ib[], double rv_in[], double qnb1[]);
#endif
