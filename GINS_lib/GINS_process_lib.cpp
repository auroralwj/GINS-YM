#include "GINS_process_lib.h"
#include "GINS_process.h"
#include "GINS_test.h"
GINS_YM *s_pstGilc = NULL;
int GINS_init(GINS_cfg_t* cfgdata)
{
	//static GINS_Process gilc;
	//s_pstGilc = &gilc;
	return s_pstGilc->GINS_Init(cfgdata);
}
int GINS_PROCESS_Lib(GINS_raw_t* pstRaw, GINS_result_t* pstOut)
{
	//if (s_pstGilc)
	return s_pstGilc->GINS_PROCESS_Lib(pstRaw, pstOut);
	//else
	//return GILC_RET__RST_NOTHING;
	//return 0;
}


int GINS_YM::GINS_PROCESS_Lib(GINS_raw_t* pstRaw, GINS_result_t* pstOut)
{
	GINS_data_correct(pstRaw);//��������׼����gnss״̬��⡢imu��ϵ����
	int ret = GINS_Rawdata_Quality(pstRaw);//���������������
	gtime_t gt = gpst2time(GI_pd.week, GI_pd.imutimetarge);//imuʱ���ʱ��ת��
	time2epoch(gpst2utc(gt), GI_pd.ep);
	//Mat_equal(GI_pd.ep, 6, 1, glv.ep);
	IMU_Filter(GI_pd);
	
	if (!GetGPS)
	{
		if (GI_pd.bGPSavail)
		{
			GetGPS = true;
		}
	}
	if (GetGPS)
	{
		//kinalign.bStatic = gipro.bStatic = gipro.detect.DetectStatic_car_dsf(ilcd.ep, ilcd.acc, ilcd.gyo, ilcd.vn, gipro.ins.vm_car, ilcd.bGPSavail, gipro.bGnssLost, ilcd.stat);
		if (GI_pd.gnss_speed< 0.4) //���ũ���汾�����ض���̬�ж���ֵ����ʹ��
		{
			GI_align.bStatic = GI_pro.bStatic = 1;
		}
		else
		{
			GI_align.bStatic = GI_pro.bStatic = 0;
		}


	if(!bAlign)
	{ 
		bool align= GI_align.KinmateAlign(GI_pd, GI_pro);//λ�á��ٶȡ���̬��ʼ��
		if (align)
		{
			bAlign = true;
			printf("%02d:%02d:%06.3f ������%f\r\n",
				int(GI_pd.ep[3]), int(GI_pd.ep[4]), GI_pd.ep[5], GI_pd.imutimetarge);

			double lever[3] = { GI_cfg.fIns2GnssVector[0],GI_cfg.fIns2GnssVector[1],GI_cfg.fIns2GnssVector[2] };
			/*INS�����ʼֵ*/
			/*��ǰ������ƫ�ͼӼ���ƫ����װ���ǳ�ֵΪ0*/
			GI_pro.ins.Init(GI_align.Att, GI_align.Vn, GI_align.Pos, eb, db, lever, dGnss2OutPointLever, GI_align.PRY_Install);
			printf("init_data:%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", GI_align.Att[0], GI_align.Att[1], GI_align.Att[2], 
				GI_align.Pos[0], GI_align.Pos[1], GI_align.Pos[2],
				GI_align.Vn[0], GI_align.Vn[1], GI_align.Vn[2],pstRaw->imutimetarget);


			if (GI_align.Att[2])
			{
				/*�����ѳ�ʼ����ͨ���˱ۡ���̬�����Ƽ���IMU��ʼλ��*/
				//double dLeverAnt2Ins[3] = { 0 };
				//Mat_mul(s_cfgdata.stEkfX_Init.Lever, 3, 1, -1, dLeverAnt2Ins);
				//gipro.ins.Lever(kinalign.PosL, kinalign.VnL, gipro.ins.pos, gipro.ins.vn, dLeverAnt2Ins);
				Mat_equal(GI_align.Vn, 3, 1, GI_pro.ins.vn);
				Mat_equal(GI_align.Pos, 3, 1, GI_pro.ins.pos);
			}
			else
			{
				/*����δ��ʼ����������λ����ΪIMU��ʼλ��*/
				Mat_equal(GI_align.Vn, 3, 1, GI_pro.ins.vn);
				Mat_equal(GI_align.Pos, 3, 1, GI_pro.ins.pos);
			}

			double cbn[9], vb[3];
			a2mat(GI_pro.ins.PRY_Install, GI_pro.ins.Cmb);//��װ����ת��Ϊ��̬����
			a2mat(GI_pro.ins.att, GI_pro.ins.Cnb);//imu��̬ת��Ϊ��̬����

			Mat_tran(GI_pro.ins.Cnb, 3, 3, cbn);
			Mat_tran(GI_pro.ins.Cmb, 3, 3, GI_pro.ins.Cbm);


			Mat_mul(cbn, GI_pro.ins.vn, 3, 3, 1, vb);
			Mat_mul(GI_pro.ins.Cmb, vb, 3, 3, 1, GI_pro.ins.vm_car);//�˱�ת����ĳ����ٶ�

			Mat_mul(GI_pro.ins.Cnb, GI_pro.ins.Cbm, 3, 3, 3, GI_pro.ins.Cnm);//imu��̬�˱�ת��
			m2att(GI_pro.ins.Cnm, GI_pro.ins.att_car);//������̬

			GI_pro.preheading = GI_pro.ins.att[2];


			Mat_equal(GI_pd.gyo, 3, 1, GI_pro.ins.wmpre);
			Mat_equal(GI_pd.acc, 3, 1, GI_pro.ins.vmpre);
			GI_pro.tpre = GI_pd.imutimetarge - 0.01;
			GINS_ekf_data_ready(&GI_cfg);//�˲���������ʼ
		}
	}
	int gilc_flag = 0;
	if (bAlign)
	{
		GI_pro.ins.imutimetarge = GI_pd.imutimetarge;
		SaveRst(&GI_pro, &GI_pd);
		gilc_flag= GI_pro.GINS_P2(GI_pd);

	}
	if (GI_pd.bGPSavail)
	{
		pstOut->bPPSSync = GI_pro.bPPSSync;
	}
	stRaw_tmp.bGPSavail = false;
	stRaw_tmp.bMEMSavail = false;
	stRaw_tmp.bPPSavail = false;
	return gilc_flag;
	}
}

int GINS_Process::ZUpdate(Process_Data ilcd)
{
	GINS_INS ppsins;
	int iUpdateZ_flag = 0;
	int i = 0, j = 0, iRet = 0;
	double dpos[3] = { 0.0 }, dvn[3] = { 0.0 };
	double enu[3];
	double dAtt[3] = { 0 }, dvm[3] = { 0.0 };
	double dVel = 0.0, dYawRate = 0.0, dWh = 0.0;
	double dVd = 0.0, dDualYaw = 0.0;
	c++; //imu���¼���

	double ppsinspos[3] = { 0.0 }, ppsinsvn[3] = { 0.0 };
	double gnssHeading, insHeading, insYaw, gnssYaw, dheading;

	insHeading = atan2(ins.vnL[0], ins.vnL[1])*R2D;
	DEG_0_360(insHeading);
	insYaw = -ins.att_car[2] * R2D;
	DEG_0_360(insYaw);

	/*----�ٶ���̬����ƫ��----*/
	double dDiffHeadingYaw_ins = insHeading - insYaw;
	DEG_NEG180_180(dDiffHeadingYaw_ins);

	double wib_car[3] = { 0.0 };
	double speed_car = 0.0;
	Mat_mul(ins.Cmb, ins.wib, 3, 3, 1, wib_car);
	speed_car = sqrt(SQ(ins.vm_car[0]) + SQ(ins.vm_car[1]));
	/*----��̬����仯��-----*/
	double dGyroZ = ins.wib[2] * R2D;
	if (dGyroZ > 2)
		bTurn = 1;
	else if (dGyroZ < -2)
		bTurn = -1;
	else
		bTurn = 0;

	loadPPSSyncInsData(ilcd, ppsins);
	if (ilcd.bGPSavail)  //GNSS
	{
		//PPSʱ�̵�INSλ��
		Mat_equal(ppsins.posL, 3, 1, ppsinspos);
		Mat_equal(ppsins.vnL, 3, 1, ppsinsvn);

		Mat_min(ppsinspos, ilcd.pos, dpos, 3, 1);
		Mat_min(ppsinsvn, ilcd.vn, dvn, 3, 1);

		//λ��+�ٶ�
		diffpos(ilcd.pos, ppsinspos, enu);
		difpos_b(ilcd.pos, ppsinspos, ppsins.att_car, dposb_sync);

		Mat_equal(enu, 3, 1, dpos_sync);//ppsʱ��λ���ٶȲ�ֵ
		Mat_equal(dvn, 3, 1, dvn_sync);

		/*��ͷ��̬��*/
		//gnssYaw = ilcd.heading2*R2D;
		gnssYaw = ilcd.heading2;

		if (gnssYaw<0) gnssYaw += 360;

		/*�ٶȺ����*/
		gnssHeading = atan2(ilcd.vn[0], ilcd.vn[1])*R2D;
		if (gnssHeading<0) gnssHeading += 360;
	}

	dheading = insHeading - gnssHeading;
	DEG_NEG180_180(dheading);

	//GnssIntegrity(ilcd, dheading, dvn);

	//OdoIntegrity(ilcd);

	/*----��̬�ж�----*/
	if (bStatic != bStaticpre)
	{
		bStaticpre = bStatic;
		if (bStatic && ins.imutimetarge)
		{
			inspre_forStatic = ins;
		}
	}

#if 0
	/*dsf90,2018.9.20˫���ߺ�������*/
	if (ilcd.bGPSavail && (ilcd.stat == 4) && ilcd.GA_RK[2]>SQ(0.001) && ilcd.GA_RK[2] <= SQ(0.2))	//GNSS
	{
		/*��ͷ��̬��*/
		double dYawBias = insYaw - gnssYaw;
		DEG_NEG180_180(dYawBias);

		if (!bDualAntAvail)
		{
			if (ilcd.heading2 && ilcd.GA_RK[2])
				bDualAntAvail = 1;
		}

		if (!bDualAntCalibrateOk)
		{
			if (!bDualAntInitOk)
			{
				if (!bTurn)
					getDualAntBias(ilcd, dYawBias); /*��ʼ��װ���*/
			}
			else
			{
				if (!bGnssLost && ilcd.stat == 4 && ilcd.heading2  && ilcd.GA_RK[2] && !bTurn)
				{
					ins.dual_yaw = insYaw;
					ins.dual_yaw_h = gnssYaw + dDualAntYawBias;

					dDualYaw = (ins.dual_yaw - ins.Byaw) - ins.dual_yaw_h;
					DEG_NEG180_180(dDualYaw);

					kf_cal.Rk[2] = ilcd.GA_RK[2] * SQ(R2D);
					iCalUpdateZ_flag |= CAL_UPDATE_Z_DUAL_YAW;

					if (kf_cal.xk_std[5] < 0.005)
						bDualAntCalibrateOk = 1; //��ɰ�װ���궨
				}
				bDualAntCalibrateOk = 1; //��ɰ�װ���궨
			}
		}

		if (bDualAntCalibrateOk)
		{
			dDiffYaw_Ins2DualAnt = insYaw - (gnssYaw + dDualAntYawBias);
			DEG_NEG180_180(dDiffYaw_Ins2DualAnt);
			if (fabs(dDiffYaw_Ins2DualAnt)<2)	//dsf90:�������䣬��ΪGNSS�쳣
			{
				dAtt[2] = dDiffYaw_Ins2DualAnt * D2R;
				kf.Rk[2] = ilcd.GA_RK[2];
				iUpdateZ_flag |= UPDATE_Z_ATT_Z;
			}
		}
	}
#endif
		kf.setRk_constraint();

		/*ʹ��Լ��*/
		dvm[0] = ins.vm_car[0];
		dvm[1] = 0;
		dvm[2] = ins.vm_car[2];
		//iUpdateZ_flag |= UPDATE_Z_CONS_VER_XZ;

	//���ٸ���
	if (bStatic)
	{
		//if (!bGnssLost || ilcd.bGPSavail || ilcd.stat == 4)
		{
			double dPos[3] = { 0 };
			Mat_min(ins.pos, inspre_forStatic.pos, dPos, 3, 1);

			//if(!bDualAntAvail || bGnssLost)/*�����߻����ǣ���̬����Լ��*/
			{
				Mat_min(ins.att_car, inspre_forStatic.att_car, dAtt, 3, 1);
				//iUpdateZ_flag |=  UPDATE_Z_ATT_Z;//���˴��Ϻ���Լ��������
			}

			double zk[NUMV] = { -dAtt[0],-dAtt[1],-dAtt[2],ins.vn[0],ins.vn[1],ins.vn[2],dPos[0],dPos[1],dPos[2],dvm[0],dvm[1],dvm[2],dYawRate };

			//iUpdateZ_flag = UPDATE_Z_ATT_Z| UPDATE_Z_VER_XYZ|UPDATE_Z_POS_XYZ;
			//iUpdateZ_flag |= UPDATE_Z_VER_XYZ;
			if (iUpdateZ_flag)
			{
				//if(bInstallOk)
				//	iUpdateZ_flag &= (~UPDATE_Z_VER_XYZ);
				//kf.MUpadte(ins, zk, iUpdateZ_flag);
				//kf.Feedback(ins, dt);
			}
			if (!ilcd.bGPSavail)
				return 1;
			else
				return 5;
		}
	}

	if (!ilcd.bGPSavail)  //��GNSS��INS��е���Ž׶�
	{
		/*dsf90:�໬Լ��*/
		double zk[NUMV] = { 0,0,0,0,0,0,0,0,0,dvm[0],dvm[1],dvm[2],dYawRate };

		if (bTurn)//���˴�ת���ʱ���������ٶ�Լ��
		{
			double dVx = fabs(ins.wib[2])*1.2;
			//dVx /= 10;
			if (dVx < 0.1)	dVx = 0.1;
			//if (dVx < 0.1)	dVx = 0.1;
			kf.Rk[9] = SQR(dVx);
			/*
			double dVz = fabs(ins.wib[0])*para.dWheelBase;
			dVz /= 10;
			if (dVz < 0.01)
			dVz = 0.01;
			else if (dVz > 0.1)
			dVz = 0.1;
			kf.Rk[11] = SQR(dVz);
			*/
			//iUpdateZ_flag &= (~UPDATE_Z_CONS_VER_X);/*��ʹ��Լ��Ч���ϲ�*/
		}

		//iUpdateType_flag = UPDATE_TYPE_CONSTRAINT;
		if (iUpdateZ_flag)
		{
			//if(bInstallOk)
			//	iUpdateZ_flag &= (~UPDATE_Z_VER_XYZ);
			//kf.MUpadte(ins, zk, iUpdateZ_flag);
			//kf.Feedback(ins, dt);
		}
#if 1
		/*dsf90:λ�ý��Լ������λ��*/
		//correctSideslip();
#endif
		return 1;
	}

	double zk[NUMV] = { dAtt[0],dAtt[1],dAtt[2],dvn[0],dvn[1],dvn[2],dpos[0],dpos[1],dpos[2],dvm[0],dvm[1],dvm[2],dYawRate };
//#if 1  //GNSS����-�����»��ٶȸ���
//	if ((fabs(enu[0])>2.0 || fabs(enu[1])>2.0 || fabs(enu[2])>2.0) && c<1000)
//	{
//		//gilc_log("   ���㣺enu>0.5\n");
//		//kf.resetRk(ilcd);
//		num_GPSskip++;
//		if ((ilcd.stat == 4 || ilcd.stat ==5) && busegnssvn)
//		{
//			//GNSS��ʱ���жϣ��ָ�ʱ�䳤
//			double PosRk[3] = { 0.0 };
//			kf.downgrade_car(ilcd, enu, PosRk);
//			/*pos*/
//			kf.Rk[6] = PosRk[0];
//			kf.Rk[7] = PosRk[1];
//			kf.Rk[8] = PosRk[2];
//			iUpdateZ_flag |= UPDATE_Z_VER_XYZ | UPDATE_Z_POS_XYZ;
//			iUpdateType_flag |= UPDATE_TYPE_GNSS;
//			c = 0;
//		}
//		else if (busegnssvn)  //λ�����䣬���ٶȿ���
//		{
//			iUpdateZ_flag |= UPDATE_Z_VER_XYZ;
//			iUpdateType_flag |= UPDATE_TYPE_GNSS;
//			c = 0;
//		}
//		if (iUpdateZ_flag)
//		{
//			//if(bInstallOk)
//			//	iUpdateZ_flag &= (~UPDATE_Z_VER_XYZ);
//			kf.MUpadte(ins, zk, iUpdateZ_flag, iUpdateType_flag);
//			kf.Feedback(ins, 0.2);
//		}
//		//λ���ٶȾ����䣬������GNSS�쳣��Ҳ������INS�ۻ�����
//		//��ֹ��INS��ɢ��������10�����䣬������GNSS
//		if (num_GPSskip<25)
//		{
//			return 2;
//		}  //�����ں�
//	}
//#endif 
//
//#if 1   //��ʱ���ж�20s��λ������������ֵ--,GPS�ָ��׶� - �ٶȺ�λ����ΪGNSS�Ľ��
//	//��Ҫ�Ļ��Ϳ�����
//	if (c>999 || num_GPSskip>24)
//	{
//		kf.resetRk(ilcd);
//		num_GNSSrestore++;
//		//if((ilcd.nsused>=10 && ilcd.hdop<=1.0) || num_GNSSrestore>25)
//		//if((ilcd.nsused>=7 && ilcd.hdop<=2.0) || num_GNSSrestore>20)
//		if ((ilcd.nsused >= 8 || ilcd.hdop <= 2.0) || num_GNSSrestore>20)
//		{
//			//GNSS�����������ڽ��㣩��PDOP����������GNSS��λ�����Ϻ�
//			iUpdateZ_flag |= UPDATE_Z_VER_XYZ | UPDATE_Z_POS_XYZ;
//			iUpdateType_flag |= UPDATE_TYPE_GNSS;
//			num_GNSSmupdate++;
//		}
//		//else
//		//{//GNSS��λ�����λ�ý�Ȩ�����ٶȸ���
//		//	//double PosRk[3]={0.0};
//		//	//kf.downgrade_car(ilcd,enu,PosRk);
//		//	//kf.resetRK2(ilcd,PosRk);
//
//		//	kf.MUpdate_veloicty(ilcd,dvn);
//		//	kf.Feedback(ins);
//
//		//	//num_GNSSmupdate=0;
//		//}
//		//�˳��ָ����� ���� GNSS�����������ڽ��㣩��PDOP����������GNSS��λ�����Ϻ� 4��
//		if (num_GNSSmupdate>4)
//		{
//			num_GPSskip = 0;
//			num_GNSSmupdate = 0;
//			num_GNSSrestore = 0;
//			c = 0;
//			num_ContinueFix = 5;
//			num_ContinueFloat = 50;
//		}
//		if (iUpdateZ_flag)
//		{
//			//if(bInstallOk)
//			//	iUpdateZ_flag &= (~UPDATE_Z_VER_XYZ);
//			kf.MUpadte(ins, zk, iUpdateZ_flag, iUpdateType_flag);
//			kf.Feedback(ins, 0.1);
//		}
//		return 2;
//	}
//#endif

	/*dsf90:GNSS���ݿ��ã�����*/
	//kf.resetRk(ilcd);
	c = 0; //INS���¼�������
		   //���쳣�ٶȽ���̽��
	//if (!busegnssvn) //gnss�ٶȲ�����
	//{
	//	if (ilcd.stat == GNSSFIX)          //�̶�
	//	{
	//		num_ContinueFix++;
	//		if (num_ContinueFix>5)       //�����̶���Ŀ>6,ֱ��ʹ��GNSS�����ķ�����Ϣ����ʼ��Ϊ6
	//		{

	//		}
	//		else                        //�����̶���Ŀ<6���������̶�������ʹ��GNSS��Ȩ��ƽ������
	//		{
	//			double PosRk[3] = { 0.0 };
	//			kf.downgrade(ilcd, (6 - num_ContinueFix), PosRk);
	//			/*pos*/
	//			kf.Rk[6] = PosRk[0];
	//			kf.Rk[7] = PosRk[1];
	//			kf.Rk[8] = PosRk[2];
	//		}
	//		num_ContinueFloat = 0;        //����������Ŀ����
	//	}
	//	else                            //�ǹ̶�
	//	{
	//		num_ContinueFloat++;
	//		if (num_ContinueFloat<10)   //�̶������㣬2���ȶ��ڣ�ʹ��denu��GNSS�ķ�����Ϣȷ���۲�ֵȨ����ֹGNSS����
	//		{
	//			double PosRk[3] = { 0.0 };
	//			kf.downgrade(ilcd, (11 - num_ContinueFloat), PosRk);
	//			//kf.downgrade_car(ilcd,enu,PosRk);
	//			/*pos*/
	//			kf.Rk[6] = PosRk[0];
	//			kf.Rk[7] = PosRk[1];
	//			kf.Rk[8] = PosRk[2];
	//		}
	//		else                        //GNSS���������ȶ������ڶ�GNSS��Ȩ����ֹINS��ɢ
	//		{

	//		}
	//		num_ContinueFix = 0;          //�����̶���Ŀ����
	//	}
	//	iUpdateZ_flag |= UPDATE_Z_POS_XYZ;
	//	iUpdateType_flag |= UPDATE_TYPE_GNSS;
	//	upmodel = 3;
	//}
	//else
	//{
	//	if (ilcd.stat == GNSSFIX)             //�̶�
	//	{
	//		num_ContinueFix++;
	//		if (num_ContinueFix>5)          //�����̶���Ŀ>6,ֱ��ʹ��GNSS�����ķ�����Ϣ����ʼ��Ϊ6
	//		{

	//		}
	//		else
	//		{
	//			double PosRk[3] = { 0.0 };
	//			kf.downgrade(ilcd, (6 - num_ContinueFix), PosRk);
	//			/*pos*/
	//			kf.Rk[6] = PosRk[0];
	//			kf.Rk[7] = PosRk[1];
	//			kf.Rk[8] = PosRk[2];
	//		}
	//		num_ContinueFloat = 0;
	//	}
	//	else                               //�ǹ̶�
	//	{
	//		num_ContinueFloat++;
	//		if (num_ContinueFloat<10) //100
	//		{
	//			double PosRk[3] = { 0.0 };
	//			kf.downgrade(ilcd, (11 - num_ContinueFloat), PosRk);
	//			//kf.downgrade_car(ilcd,enu,PosRk);
	//			/*pos*/
	//			kf.Rk[6] = PosRk[0];
	//			kf.Rk[7] = PosRk[1];
	//			kf.Rk[8] = PosRk[2];
	//		}
	//		else
	//		{

	//		}
	//		num_ContinueFix = 0;          //�����̶���Ŀ����
	//	}
	//	iUpdateZ_flag |= UPDATE_Z_VER_XYZ | UPDATE_Z_POS_XYZ;
	//	iUpdateType_flag |= UPDATE_TYPE_GNSS;
	//	upmodel = 23;
	//}
	iUpdateZ_flag |= UPDATE_Z_VER_XYZ | UPDATE_Z_POS_XYZ;
	if (iUpdateZ_flag)
	{
		//if(bInstallOk)
		//	iUpdateZ_flag &= (~UPDATE_Z_VER_XYZ);
		kf.MUpadte(ins, zk, iUpdateZ_flag);
		kf.Feedback(ins, 0.2);
	}

	Mat_equal(kf.xk, kf.ROW, 1, kf.xkpre);
	Mat_equal(dpos, 3, 1, kf.dpos);
	Mat_equal(enu, 3, 1, kf.denu);

	num_GPSskip = 0;
	return 2;
}

void MUpdate_Variable(int ROW, int COL, double Hk[], double Rk[], double ZK[], double xk[], double Pxk[], int zflag)
{
	int i = 0, j = 0, k = 0, usecol = 0;
	for (i = 0;i<COL;i++)
	{
		if ((zflag >> i) % 2)
			usecol++;
	}
	double* usehk = (double*)__ml_zero(sizeof(double)*usecol*ROW);
	double* userk = (double*)__ml_zero(sizeof(double)*usecol*usecol);
	double* usezk = (double*)__ml_zero(sizeof(double)*usecol * 1);
	for (i = 0;i<COL;i++)
	{
		if ((zflag >> i) % 2)
		{
			for (j = 0;j<ROW;j++)
				usehk[k*ROW + j] = Hk[i*ROW + j];
			userk[k*usecol + k] = Rk[i];
			usezk[k] = ZK[i];
			k++;
		}
	}

	MUpdate_True(ROW, usecol, usehk, userk, usezk, xk, Pxk);

	free(usehk);
	free(userk);
	free(usezk);
}

void MUpdate_True(int ROW, int COL, double Hk[], double Rk[], double ZK[], double xk[], double Pxk[])
{
	double* xkk_1 = (double*)__ml_zero(sizeof(double)*ROW * 1);
	double* Pxkk_1 = (double*)__ml_zero(sizeof(double)*ROW*ROW);
	double* Pxykk_1 = (double*)__ml_zero(sizeof(double)*ROW*COL);
	double* Pykk_1 = (double*)__ml_zero(sizeof(double)*COL*COL);
	double* Kk = (double*)__ml_zero(sizeof(double)*ROW*COL);
	double* ykk_1 = (double*)__ml_zero(sizeof(double)*COL * 1);
	double* dxk = (double*)__ml_zero(sizeof(double)*ROW * 1);
	double* Hkt = (double*)__ml_zero(sizeof(double)*ROW*COL);
	double* invPy = (double*)__ml_zero(sizeof(double)*COL*COL);
	double* KPyKt = (double*)__ml_zero(sizeof(double)*ROW*ROW);


	Mat_equal(xk, ROW, 1, xkk_1);
	Mat_equal(Pxk, ROW, ROW, Pxkk_1);
	Mat_tran(Hk, COL, ROW, Hkt);
	Mat_mul(Pxkk_1, Hkt, ROW, ROW, COL, Pxykk_1);
	Mat_mul(Hk, Pxykk_1, COL, ROW, COL, Pykk_1);
	Mat_add(Pykk_1, Rk, Pykk_1, COL, COL);
	Mat_inv(Pykk_1, COL, invPy);
	Mat_mul(Pxykk_1, invPy, ROW, COL, COL, Kk);
	Mat_mul(Hk, xkk_1, COL, ROW, 1, ykk_1);
	Mat_min(ZK, ykk_1, ykk_1, COL, 1);
	Mat_mul(Kk, ykk_1, ROW, COL, 1, dxk);
	Mat_add(xkk_1, dxk, xk, ROW, 1);

	/*add by dsf90,2018,3,1;����Ч��һ�£�û�в���*/
	double* KkHk = (double*)__ml_zero(sizeof(double)*ROW*ROW);
	double* KkHkPxkk_1 = (double*)__ml_zero(sizeof(double)*ROW*ROW);

	/*dsf90:Pxk  	= Pxk - Kk X Hk X Pxk*/
	Mat_mul(Kk, Hk, ROW, COL, ROW, KkHk);
	Mat_mul(KkHk, Pxkk_1, ROW, ROW, ROW, KkHkPxkk_1);
	Mat_min(Pxkk_1, KkHkPxkk_1, Pxk, ROW, ROW);
	//Mmul(Pxk,ROW,ROW,0.995);

	free(KkHk);       KkHk = NULL;
	free(KkHkPxkk_1); KkHkPxkk_1 = NULL;
	/*end add*/

	free(xkk_1);
	free(Pxkk_1);
	free(Pxykk_1);
	free(Pykk_1);
	free(Kk);
	free(ykk_1);
	free(dxk);
	free(Hkt);
	free(invPy);
	free(KPyKt);
	xkk_1 = NULL;
	Pxkk_1 = NULL;
	Pxykk_1 = NULL;
	Pykk_1 = NULL;
	Kk = NULL;
	ykk_1 = NULL;
	dxk = NULL;
	Hkt = NULL;
	invPy = NULL;
	KPyKt = NULL;
}