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

	double imu_raw[6];
	double imu_iir[6];
	//Mequalm(ilcd.acc, 3, 1, &imu_raw[0]);
	//Mequalm(ilcd.gyo, 3, 1, &imu_raw[3]);
	//iir_filter.IIR_Lowpass(imu_iir, imu_raw);
	//Mequalm(&imu_iir[0], 3, 1, ilcd.acc_iir);
	//Mequalm(&imu_iir[3], 3, 1, ilcd.gyo_iir);
	//Mequalm(&imu_iir[0],3,1,ilcd.acc);
	//Mequalm(&imu_iir[3],3,1,ilcd.gyo);


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
			GI_pro.ins.Init(GI_align.Att, GI_align.VnL, GI_align.PosL, eb, db, lever, dGnss2OutPointLever, GI_align.PRY_Install);
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


			Mat_equal(GI_pd.gyo, 3, 1, GI_pro.wmpre);
			Mat_equal(GI_pd.acc, 3, 1, GI_pro.vmpre);
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